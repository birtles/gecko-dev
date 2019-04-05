/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "CompactFillEffect.h"

#include "mozilla/AnimationTarget.h"
#include "mozilla/KeyframeEffectParams.h"
#include "mozilla/TimingParams.h"
#include "nsContentUtils.h"

namespace mozilla {

NS_IMPL_ADDREF_INHERITED(CompactFillEffect, KeyframeEffect)
NS_IMPL_RELEASE_INHERITED(CompactFillEffect, KeyframeEffect)

NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(CompactFillEffect)
NS_INTERFACE_MAP_END_INHERITING(KeyframeEffect)

static Maybe<OwningAnimationTarget> ToOwningAnimationTarget(
    const Maybe<const NonOwningAnimationTarget>& aTarget) {
  Maybe<OwningAnimationTarget> result;
  if (!aTarget) {
    return result;
  }

  const NonOwningAnimationTarget& target = aTarget.ref();
  result = Some(OwningAnimationTarget{target.mElement, target.mPseudoType});

  return result;
}

TimingParams FillTimingParams() {
  return TimingParams(TimeDuration::Forever(),  // duration
                      TimeDuration(),           // delay
                      TimeDuration(),           // end-delay
                      1.0f,                     // iterations
                      0.0f,                     // iteration start
                      dom::PlaybackDirection::Normal, dom::FillMode::Both,
                      Nothing());  // timing function
}

CompactFillEffect::CompactFillEffect(dom::KeyframeEffect& aOriginalEffect)
    : dom::KeyframeEffect(aOriginalEffect.GetOwnerDocument(),
                          ToOwningAnimationTarget(aOriginalEffect.GetTarget()),
                          FillTimingParams(), KeyframeEffectParams()) {
  MOZ_ASSERT(!aOriginalEffect.AsCompactFillEffect(),
             "The original effect should not itself be a compact fill effect");
  mLinkedEffect = &aOriginalEffect;
}

nsTArray<AnimationProperty> CompactFillEffect::BuildProperties(
    const ComputedStyle* aComputedValues) {
  MOZ_ASSERT(aComputedValues);

  nsTArray<AnimationProperty> result;
  // If mTarget is null, return an empty property array.
  if (!mTarget) {
    return result;
  }

  nsPresContext* presContext =
      nsContentUtils::GetContextForContent(mTarget->mElement);
  if (!presContext) {
    return result;
  }

  result.SetCapacity(mFillSnapshot.Length());

  for (FillPropertySnapshot& propertySnapshot : mFillSnapshot) {
    AnimationProperty property;
    property.mProperty = propertySnapshot.mProperty;

    AnimationPropertySegment segment;
    segment.mFromKey = 0.f;

    // FillTimingParams() sets an infinite duration such that we will always be
    // sampled with a progress of 0.0. That's fine because the timing function
    // we set below will ensure we use the correct progress within the interval.
    //
    // As such, it doesn't matter what value we set for the mToKey but we try to
    // preserve the invariant that the last segment ends at 1.0.
    //
    // If we are going to add another value to support iterationComposite then
    // we need to make room for that segment too.
    segment.mToKey = propertySnapshot.mLastValue ? .5f : 1.f;

    if (propertySnapshot.mToValue.mSpecifiedValue) {
      segment.mToValue =
          AnimationValue(presContext->StyleSet()->ComputeAnimationValue(
              mTarget->mElement, propertySnapshot.mToValue.mSpecifiedValue,
              aComputedValues));
    }
    segment.mToComposite = propertySnapshot.mToValue.mComposite;

    if (propertySnapshot.mFromValue) {
      if (propertySnapshot.mFromValue->mSpecifiedValue) {
        segment.mFromValue =
            AnimationValue(presContext->StyleSet()->ComputeAnimationValue(
                mTarget->mElement, propertySnapshot.mFromValue->mSpecifiedValue,
                aComputedValues));
      }
      segment.mFromComposite = propertySnapshot.mFromValue->mComposite;
    } else {
      segment.mFromValue = segment.mToValue;
      segment.mFromComposite = propertySnapshot.mToValue.mComposite;
    }

    segment.mTimingFunction =
        Some(ComputedTimingFunction::Fixed(propertySnapshot.mPortion));

    property.mSegments.AppendElement(std::move(segment));

    // Add an extra segment for the last value if needed
    if (propertySnapshot.mLastValue) {
      AnimationPropertySegment lastSegment;
      lastSegment.mFromKey = .5f;
      lastSegment.mToKey = 1.f;

      // We don't actually use the from value for this segment but if we leave
      // the value as null then AnimationPropertySegment::HasReplaceableValues
      // will return false meaning that KeyframeEffect will end up calculating
      // a base style even when it isn't needed.

      if (propertySnapshot.mLastValue->mSpecifiedValue) {
        lastSegment.mFromValue =
            AnimationValue(presContext->StyleSet()->ComputeAnimationValue(
                mTarget->mElement, propertySnapshot.mLastValue->mSpecifiedValue,
                aComputedValues));
      }
      lastSegment.mFromComposite = dom::CompositeOperation::Replace;

      lastSegment.mToValue = lastSegment.mFromValue;
      lastSegment.mToComposite = propertySnapshot.mLastValue->mComposite;

      property.mSegments.AppendElement(std::move(lastSegment));
      property.mCurrentIteration = Some(propertySnapshot.mCurrentIteration);
    }

    result.AppendElement(std::move(property));
  }

  return result;
}

}  // namespace mozilla
