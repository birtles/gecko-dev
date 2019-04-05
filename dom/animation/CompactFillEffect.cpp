/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "CompactFillEffect.h"

#include "mozilla/KeyframeEffectParams.h"
#include "mozilla/TimingParams.h"

namespace mozilla {

NS_IMPL_ADDREF_INHERITED(CompactFillEffect, KeyframeEffect)
NS_IMPL_RELEASE_INHERITED(CompactFillEffect, KeyframeEffect)

NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(CompactFillEffect)
NS_INTERFACE_MAP_END_INHERITING(KeyframeEffect)

TimingParams FillTimingParams() {
  return TimingParams(TimeDuration::Forever(),  // duration
                      TimeDuration(),           // delay
                      TimeDuration(),           // end-delay
                      1.0f,                     // iterations
                      0.0f,                     // iteration start
                      dom::PlaybackDirection::Normal, dom::FillMode::Both,
                      Nothing());  // timing function
}

CompactFillEffect::CompactFillEffect(dom::Document* aDocument,
                                     const OwningAnimationTarget& aTarget)
    : dom::KeyframeEffect(aDocument, Some(aTarget), FillTimingParams(),
                          KeyframeEffectParams()) {}

}  // namespace mozilla
