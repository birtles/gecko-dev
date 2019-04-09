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

}  // namespace mozilla
