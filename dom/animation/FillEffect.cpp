/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "FillEffect.h"

#include "mozilla/CompactFillEffect.h"
#include "mozilla/dom/Animation.h"
#include "mozilla/FillTimingParams.h"
#include "mozilla/KeyframeEffectParams.h"
#include "mozilla/ServoBindings.h"

namespace mozilla {

NS_IMPL_ADDREF_INHERITED(FillEffect, KeyframeEffect)
NS_IMPL_RELEASE_INHERITED(FillEffect, KeyframeEffect)

NS_IMPL_CYCLE_COLLECTION_INHERITED(FillEffect, KeyframeEffect, mSourceEffects)

NS_IMPL_CYCLE_COLLECTION_TRACE_BEGIN_INHERITED(FillEffect, KeyframeEffect)
NS_IMPL_CYCLE_COLLECTION_TRACE_END

NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(FillEffect)
NS_INTERFACE_MAP_END_INHERITING(KeyframeEffect)

FillEffect::FillEffect(dom::Document* aDocument,
                       const OwningAnimationTarget& aTarget)
    : dom::KeyframeEffect(aDocument, Some(aTarget), FillTimingParams(),
                          KeyframeEffectParams()) {}

void FillEffect::Init(const nsTArray<CompactFillEffect*>& aSourceEffects) {
  MOZ_ASSERT(mSourceEffects.IsEmpty(),
             "FillEffect should not be initialized twice");

  for (CompactFillEffect* effect : aSourceEffects) {
    MOZ_ASSERT(effect, "Should not pass null pointers to Init");
    effect->AddReferencingEffect(*this);
    mSourceEffects.AppendElement(effect);
  }
}

void FillEffect::NotifyAnimationCanceled() {
  nsTArray<RefPtr<KeyframeEffect>> sourceEffects(std::move(mSourceEffects));
  for (KeyframeEffect* effect : sourceEffects) {
    dom::Animation* animation = effect->GetAnimation();
    if (animation) {
      animation->Cancel();
    }
  }

  KeyframeEffect::NotifyAnimationCanceled();
}

void FillEffect::NotifyAnimationInvalidated() {
  mSourceEffects.Clear();

  // We don't call Cancel on the source effect's animations since the whole
  // point of this method is to allow a FillAnimation to disappear when one of
  // its source effects has been canceled _without_ also causing all the other
  // source effects to be canceled at the same time.

  KeyframeEffect::NotifyAnimationInvalidated();
}

void FillEffect::GetKeyframes(JSContext*& aCx, nsTArray<JSObject*>& aResult,
                              ErrorResult& aRv) const {
  // FillEffects and their corresponding FillAnimations are readonly so we don't
  // expect them ever be without a target effect or animation.
  if (!mTarget || !mTarget->mElement || !mAnimation) {
    return;
  }

  nsPresContext* presContext =
      nsContentUtils::GetContextForContent(mTarget->mElement);
  if (!presContext) {
    return;
  }

  // Make sure the underlying computed style is up-to-date.
  dom::Document* doc = GetRenderedDocument();
  if (doc) {
    doc->FlushPendingNotifications(
        ChangesToFlush(FlushType::Style, false /* flush animations */));
  }

  // Determine the last source effect in our (unsorted) set.
  //
  // (We could just sort the effects when they are set, but that is O(n log n)
  // and doing this is O(n) and we don't expect this to be called for most
  // animations.)
  const KeyframeEffect* lastSourceEffect = nullptr;
  for (const KeyframeEffect* effect : mSourceEffects) {
    if (!lastSourceEffect ||
        (effect->GetAnimation() &&
         lastSourceEffect->GetAnimation()->HasLowerCompositeOrderThan(
             *effect->GetAnimation()))) {
      lastSourceEffect = effect;
    }
  }

  // Compose the animated style up to and including our last source effect.
  UniquePtr<RawServoAnimationValueMap> animationValues =
      Servo_AnimationValueMap_Create().Consume();
  if (mSourceEffects.IsEmpty()) {
    return;
  }
  bool result = presContext->EffectCompositor()->GetPartialServoAnimationRule(
      mTarget->mElement, mTarget->mPseudoType, lastSourceEffect,
      mAnimation->CascadeLevel(), animationValues.get());
  if (!result) {
    return;
  }

  // KeyframeEffect::SerializeKeyframes takes an array of keyframes and a set of
  // base values. If a particular keyframe does not have a value it will use the
  // base value.
  //
  // We exploit that here by setting up a base value set which has all the
  // computed values for the properties we are animating. Then we pass in a set
  // of empty keyframes specifying those values so that SerializeKeyframes will
  // pick up the corresponding values from the base values.
  nsRefPtrHashtable<nsUint32HashKey, RawServoAnimationValue> computedValues;

  Keyframe firstKeyframe, lastKeyframe;
  firstKeyframe.mOffset = Some(0.0);
  firstKeyframe.mComputedOffset = 0.0;
  lastKeyframe.mOffset = Some(1.0);
  lastKeyframe.mComputedOffset = 1.0;

  nsCSSPropertyIDSet properties;
  for (const KeyframeEffect* effect : mSourceEffects) {
    properties |= effect->GetPropertySet();
  }

  for (nsCSSPropertyID property : properties) {
    firstKeyframe.mPropertyValues.AppendElement(PropertyValuePair(property));
    lastKeyframe.mPropertyValues.AppendElement(PropertyValuePair(property));

    RefPtr<RawServoAnimationValue> computedValue =
        Servo_AnimationValueMap_GetValue(animationValues.get(), property)
            .Consume();
    computedValues.Put(property, computedValue);
  }

  AutoTArray<Keyframe, 2> dummyKeyframes;
  dummyKeyframes.AppendElement(std::move(firstKeyframe));
  dummyKeyframes.AppendElement(std::move(lastKeyframe));

  KeyframeEffect::SerializeKeyframes(aCx, dummyKeyframes, computedValues,
                                     nullptr, aResult, aRv);
}

void FillEffect::GetProperties(
    nsTArray<dom::AnimationPropertyDetails>& aProperties,
    ErrorResult& aRv) const {
  for (const KeyframeEffect* effect : mSourceEffects) {
    effect->GetProperties(aProperties, aRv);
    if (aRv.Failed()) {
      return;
    }
  }
}

}  // namespace mozilla
