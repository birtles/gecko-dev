/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_FillTimingParams_h
#define mozilla_FillTimingParams_h

#include "mozilla/dom/AnimationEffectBinding.h"  // PlaybackDirection, FillMode
#include "mozilla/Maybe.h"                       // Nothing
#include "mozilla/TimeStamp.h"
#include "mozilla/TimingParams.h"

namespace mozilla {

inline TimingParams FillTimingParams() {
  return TimingParams(TimeDuration(),  // duration
                      TimeDuration(),  // delay
                      TimeDuration(),  // end-delay
                      1.0f,            // iterations
                      0.0f,            // iteration start
                      dom::PlaybackDirection::Normal, dom::FillMode::Both,
                      Nothing());  // timing function
}

}  // namespace mozilla

#endif  // mozilla_FillEffect_h
