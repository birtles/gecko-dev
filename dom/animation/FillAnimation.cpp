/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "FillAnimation.h"

#include "mozilla/AnimationTarget.h"  // For ToOwningAnimationTarget
#include "mozilla/dom/FillAnimationBinding.h"
#include "mozilla/FillAnimationRegistry.h"
#include "mozilla/FillEffect.h"

namespace mozilla {
namespace dom {

NS_IMPL_ADDREF_INHERITED(FillAnimation, Animation)
NS_IMPL_RELEASE_INHERITED(FillAnimation, Animation)

NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(FillAnimation)
NS_INTERFACE_MAP_END_INHERITING(Animation)

JSObject* FillAnimation::WrapObject(JSContext* aCx,
                                    JS::Handle<JSObject*> aGivenProto) {
  return FillAnimation_Binding::Wrap(aCx, this, aGivenProto);
}

/* static */ already_AddRefed<FillAnimation> FillAnimation::Create(
    const nsTArray<CompactFillEffect*>& aSourceEffects) {
  MOZ_ASSERT(!aSourceEffects.IsEmpty(),
             "Should have at least one source effect");

#ifdef DEBUG
  {
    AnimationTimeline* timeline = nullptr;
    NonOwningAnimationTarget target;

    // All source effects should...
    for (CompactFillEffect* effect : aSourceEffects) {
      // ... be associated with an animation,
      MOZ_ASSERT(effect->GetAnimation(), "Should have an animation");

      // ... be associated with the same timeline,
      MOZ_ASSERT(effect->GetAnimation()->GetTimeline(),
                 "Should have a timeline");
      MOZ_ASSERT(!timeline || effect->GetAnimation()->GetTimeline() == timeline,
                 "Should have the same timeline");
      timeline = effect->GetAnimation()->GetTimeline();

      // ... and target the same element.
      MOZ_ASSERT(effect->GetTarget() && effect->GetTarget()->mElement,
                 "Should have a target element");
      MOZ_ASSERT(!target.mElement || effect->GetTarget().ref() == target,
                 "Should have the same target");
      target = effect->GetTarget().ref();
    }
  }
#endif

  // Although any effect in |aSourceEffects| would do, we use the last one here
  // since we'll want the last animation below in order to shadow its animation
  // index.
  CompactFillEffect* lastEffect = aSourceEffects.LastElement();
  RefPtr<FillEffect> fillEffect =
      new FillEffect(lastEffect->GetOwnerDocument(),
                     ToOwningAnimationTarget(lastEffect->GetTarget()).ref());
  fillEffect->Init(aSourceEffects);

  Animation* lastAnimation = lastEffect->GetAnimation();
  RefPtr<FillAnimation> fillAnimation =
      new FillAnimation(lastAnimation->GetOwnerGlobal());
  fillAnimation->SetTimelineNoUpdate(lastAnimation->GetTimeline());
  fillAnimation->SetEffectNoUpdate(fillEffect);
  fillAnimation->SetStartTime(Nullable<TimeDuration>(TimeDuration()));
  fillAnimation->ShadowAnimationIndex(*lastAnimation);

  return fillAnimation.forget();
}

bool FillAnimation::ShouldKeepAlive() const {
  // If we have no source effects, then we must have been canceled. We will
  // therefore not be in the FillAnimationRegistry and there's no need to keep
  // us alive.
  if (!NumSourceEffects()) {
    return false;
  }

  if (mListenerManager && mListenerManager->HasListeners()) {
    return true;
  }

  if (!mId.IsEmpty()) {
    return true;
  }

  return false;
}

void FillAnimation::SetId(const nsAString& aId) {
  Animation::SetId(aId);
  MaybeKeepAlive();
}

void FillAnimation::EventListenerAdded(nsAtom* aType) {
  Animation::EventListenerAdded(aType);
  MaybeKeepAlive();
}

uint64_t FillAnimation::NumSourceEffects() const {
  if (!mEffect || !mEffect->AsFillEffect()) {
    return 0;
  }

  return mEffect->AsFillEffect()->NumSourceEffects();
}

void FillAnimation::MaybeKeepAlive() {
  if (!mEffect || !mEffect->AsKeyframeEffect()) {
    return;
  }

  if (!ShouldKeepAlive()) {
    return;
  }

  Document* doc = mEffect->AsKeyframeEffect()->GetRenderedDocument();
  if (!doc) {
    return;
  }

  doc->GetOrCreateFillAnimationRegistry()->KeepFillAnimationAlive(*this);
}

}  // namespace dom
}  // namespace mozilla
