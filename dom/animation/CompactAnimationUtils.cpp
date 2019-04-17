/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "CompactAnimationUtils.h"

#include "mozilla/dom/Animation.h"
#include "mozilla/dom/AnimationBinding.h"  // for AnimationPlayState
#include "mozilla/dom/Nullable.h"
#include "mozilla/AnimationTarget.h"
#include "mozilla/CompactFillEffect.h"
#include "mozilla/EffectSet.h"
#include "mozilla/KeyframeEffectComparator.h"
#include "mozilla/TimeStamp.h"  // For TimeDuration

using namespace mozilla::dom;

namespace mozilla {

template <class T>
bool IsMarkupAnimation(T* aAnimation) {
  return aAnimation && aAnimation->IsTiedToMarkup();
}

/* static */ bool CompactAnimationUtils::ShouldCompact(
    const Animation& aAnimation) {
  // We never compact CSS animations or CSS transitions since they are managed
  // by CSS.
  if (IsMarkupAnimation(aAnimation.AsCSSAnimation()) ||
      IsMarkupAnimation(aAnimation.AsCSSTransition())) {
    return false;
  }

  // Only finished animations compact.
  if (aAnimation.PlayState() != AnimationPlayState::Finished) {
    return false;
  }

  // We can only compact an animation if we know that, uninterfered, it would
  // never start playing again. That excludes any animations on timelines that
  // aren't monotonically increasing.
  if (aAnimation.GetTimeline() &&
      !aAnimation.GetTimeline()->TracksWallclockTime()) {
    return false;
  }

  // We only compact animations that have an effect we know how to compact.
  if (!aAnimation.GetEffect() || !aAnimation.GetEffect()->AsKeyframeEffect()) {
    return false;
  }

  // We only compact animations that are filling.
  if (aAnimation.GetEffect()->GetComputedTiming().mProgress.IsNull()) {
    return false;
  }

  // We shouldn't compact animations that are themselves compacted animations.
  if (aAnimation.GetEffect()->AsCompactFillEffect()) {
    return false;
  }

  // We shouldn't compact animations without a target element.
  // After all, where would we store the CompactFillEffect to ensure it stays
  // alive?
  if (!aAnimation.GetEffect()->AsKeyframeEffect()->GetTarget()) {
    return false;
  }

  return true;
}

/* static */ void CompactAnimationUtils::CompactAnimation(
    Animation& aAnimation) {
  MOZ_ASSERT(
      ShouldCompact(aAnimation),
      "Animation to compact should meet the requirements for compacting");
  MOZ_ASSERT(!aAnimation.GetEffect()->AsKeyframeEffect()->GetLinkedEffect(),
             "Animation has already been compacted");

  // Setup the animation
  RefPtr<Animation> compactedAnimation =
      new Animation(aAnimation.GetOwnerGlobal());
  compactedAnimation->SetTimelineNoUpdate(aAnimation.GetTimeline());
  compactedAnimation->SetStartTime(Nullable<TimeDuration>(TimeDuration()));
  compactedAnimation->ShadowAnimationIndex(aAnimation);
  compactedAnimation->SwapListPosition(aAnimation);

  // Setup the effect
  RefPtr<KeyframeEffect> sourceEffect =
      aAnimation.GetEffect()->AsKeyframeEffect();
  RefPtr<CompactFillEffect> compactedEffect =
      new CompactFillEffect(*sourceEffect);
  compactedAnimation->SetEffectNoUpdate(compactedEffect);

  // Tie the effects together
  sourceEffect->SetLinkedEffect(compactedEffect);
  // (We don't need to call SetLinkedEffect on the compacted effect since its
  // constructor does this.)

#ifdef DEBUG
  Maybe<NonOwningAnimationTarget> target = sourceEffect->GetTarget();
  MOZ_ASSERT(
      target,
      "Should have a target element for the effect we are trying to compact");
  EffectSet* effectSet =
      EffectSet::GetEffectSet(target->mElement, target->mPseudoType);
  MOZ_ASSERT(
      effectSet,
      "Should have an effect set for an effect we are trying to compact");

  bool foundCompactedEffect = false;
  for (KeyframeEffect* effect : *effectSet) {
    MOZ_ASSERT(effect != sourceEffect,
               "The source effect should no longer be in the effect set");
    if (effect == compactedEffect) {
      foundCompactedEffect = true;
    }
  }

  // The following assertion ensures that the objects we generated here will
  // continue to survive after this function exits.
  MOZ_ASSERT(foundCompactedEffect,
             "The compacted effect should have been added to the effect set");
#endif
}

/* static */ void CompactAnimationUtils::RestoreAnimation(
    Animation& aAnimation) {
  // Break the link between the two effects
  MOZ_ASSERT(
      aAnimation.GetEffect() && aAnimation.GetEffect()->AsKeyframeEffect(),
      "The animation to restore should have a keyframe effect");

  RefPtr<KeyframeEffect> sourceEffect =
      aAnimation.GetEffect()->AsKeyframeEffect();

  MOZ_ASSERT(sourceEffect->GetLinkedEffect() &&
                 sourceEffect->GetLinkedEffect()->AsCompactFillEffect(),
             "The animation to restore should be linked to a compacted effect");
  RefPtr<KeyframeEffect> compactedEffect = sourceEffect->GetLinkedEffect();

  sourceEffect->SetLinkedEffect(nullptr);
  compactedEffect->SetLinkedEffect(nullptr);

  // Restore the animation's position in the animation list.
  aAnimation.SwapListPosition(*compactedEffect->GetAnimation());

  // Cancel the compacted animation
  //
  // This needs to happen after breaking the link or else it will cause the
  // linked source animation to also be canceled.
  MOZ_ASSERT(
      compactedEffect->GetAnimation(),
      "The animation to restore should be linked to a compacted animation");
  compactedEffect->GetAnimation()->Cancel();

  // Check that the EffectSet registration has been updated
#ifdef DEBUG
  Maybe<NonOwningAnimationTarget> target = sourceEffect->GetTarget();
  MOZ_ASSERT(
      target,
      "Should have a target element for the effect we are trying to restore");
  EffectSet* effectSet =
      EffectSet::GetEffectSet(target->mElement, target->mPseudoType);
  MOZ_ASSERT(
      effectSet || !aAnimation.IsRelevant(),
      "Should have an effect set for an effect we are trying to restore if "
      "its animation is relevant");

  if (effectSet) {
    bool foundSourceEffect = false;
    for (KeyframeEffect* effect : *effectSet) {
      MOZ_ASSERT(effect != compactedEffect,
                 "The compacted effect should no longer be in the effect set");
      if (effect == sourceEffect) {
        foundSourceEffect = true;
      }
    }

    MOZ_ASSERT(foundSourceEffect == aAnimation.IsRelevant(),
               "The source effect should have been re-added to the effect set "
               "but only if its animation is relevant");
  }
#endif
}

/*static*/ void CompactAnimationUtils::CombineEffects(
    EffectSet& aEffectSet, const ComputedStyle* aStyle) {
  if (!aEffectSet.MayHaveCompactFillEffects() ||
      !aEffectSet.MayNeedCompacting()) {
    return;
  }

  nsTArray<RefPtr<KeyframeEffect>> effects;
  for (KeyframeEffect* effect : aEffectSet) {
    effects.AppendElement(effect);
  }
  effects.Sort(KeyframeEffectComparator());

  nsTArray<CompactFillEffect*> updatedEffects;

  CompactFillEffect* previousEffect = nullptr;
  for (KeyframeEffect* effect : effects) {
    CompactFillEffect* compactFillEffect = effect->AsCompactFillEffect();
    if (!compactFillEffect || !compactFillEffect->CanBeCombined()) {
      previousEffect = nullptr;
      continue;
    }

    if (previousEffect && previousEffect->CombineWith(*compactFillEffect)) {
      if (updatedEffects.IsEmpty() ||
          updatedEffects.LastElement() != previousEffect) {
        updatedEffects.AppendElement(previousEffect);
      }
      // The animations associated with |effect| and |previousEffect| will have
      // different animation indices so which should we use for the animation of
      // the combined result?
      //
      // The answer is, it doesn't matter. CombineWith above will ensure there
      // are no interleaving animations in existence so it won't make any
      // difference as far as compositing, getAnimations(), or event dispatch
      // order is concerned.
      aEffectSet.RemoveEffect(*effect);

      // We need to make sure the effect's animation no longer exists in the
      // global animation list or else it will prevent us from combining
      // |previousEffect| repeatedly.
      effect->GetAnimation()->RemoveFromGlobalAnimationList();
    } else {
      previousEffect = compactFillEffect;
    }
  }

  // We compact the snapshots out-of-band as otherwise this operation would be
  // O(n^2) when we have a bunch of effects that cannot be reduced further.
  for (CompactFillEffect* effect : updatedEffects) {
    effect->ReduceSnapshot(aStyle);
  }
}

}  // namespace mozilla
