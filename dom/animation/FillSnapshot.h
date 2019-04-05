/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_FillSnapshot_h
#define mozilla_FillSnapshot_h

#include "mozilla/Maybe.h"
#include "mozilla/ServoBindingTypes.h"
#include "nsTArray.h"

namespace mozilla {

namespace dom {
enum class CompositeOperation : uint8_t;
}  // namespace dom

struct FillValue {
  // We need to store the specified value because sometimes it is
  // context-sensitive. For example, 12em (depends on the font-size),
  // var(--yellow) (depends on the variable definition), etc.
  //
  // If the relevant context changes while the animation is filling we should
  // update the computed value.
  RefPtr<RawServoDeclarationBlock> mSpecifiedValue;

  dom::CompositeOperation mComposite;
};

struct FillPropertySnapshot {
  nsCSSPropertyID mProperty;

  // For most filling animations it is sufficient to store the final value and
  // fill with that. However, in some circumstances where we are filling at the
  // midpoint of some interpolation (e.g. iterationCount or iterationStart is
  // 0.5), and we have either specified values for one or more of the endpoints,
  // or we have a mix of composite modes, we need to keep both endpoints along
  // with the portion of the interval we are interpolating so that we can
  // recreate the effect when the context or underlying value changes.
  Maybe<FillValue> mFromValue;
  FillValue mToValue;

  // The portion between mFromValue and mToValue to sample. This should be 1.0
  // if mFromValue is Nothing().
  double mPortion = 1.0;

  // The last value in the series of properties for this effect. Only needed
  // when iterationComposite is 'accumulate'.
  Maybe<FillValue> mLastValue;

  // This should be 0 if mLastValue is Nothing().
  uint64_t mCurrentIteration = 0;
};

using FillSnapshot = nsTArray<FillPropertySnapshot>;

#ifdef DEBUG
void DumpFillSnapshot(const FillSnapshot& aSnapshot);
#endif

}  // namespace mozilla

#endif  // mozilla_FillSnapshot_h
