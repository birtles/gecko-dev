/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/FillSnapshot.h"

#include "mozilla/dom/BaseKeyframeTypesBinding.h"  // For CompositeOperation
#include "mozilla/ServoBindings.h"
#include "nsCSSPropertyID.h"
#include "nsCSSProps.h"
#include "nsString.h"

namespace mozilla {

#ifdef DEBUG
nsCString SerializeCompositeOperation(dom::CompositeOperation aMode) {
  switch (aMode) {
    case dom::CompositeOperation::Replace:
      return NS_LITERAL_CSTRING("replace").AsString();

    case dom::CompositeOperation::Add:
      return NS_LITERAL_CSTRING("add").AsString();

    case dom::CompositeOperation::Accumulate:
      return NS_LITERAL_CSTRING("accumulate").AsString();

    default:
      MOZ_ASSERT_UNREACHABLE("Unexpected composite mode");
      return NS_LITERAL_CSTRING("<<error>>").AsString();
  }
}

nsCString SerializeFillValue(nsCSSPropertyID aProperty,
                             const FillValue& aFillValue) {
  nsAutoString value;
  if (aFillValue.mSpecifiedValue) {
    Servo_DeclarationBlock_SerializeOneValue(
        aFillValue.mSpecifiedValue, aProperty, &value, nullptr, nullptr);
  } else {
    value = NS_LITERAL_STRING("<none>");
  }

  nsAutoCString result;
  result.AppendPrintf("%s (%s)", NS_ConvertUTF16toUTF8(value).get(),
                      SerializeCompositeOperation(aFillValue.mComposite).get());
  return std::move(result);
}

void DumpFillSnapshot(const FillSnapshot& aSnapshot) {
  for (auto& p : aSnapshot) {
    printf("%s\n", nsCString(nsCSSProps::GetStringValue(p.mProperty)).get());

    // Values
    if (p.mFromValue) {
      printf("  from: %s\n",
             SerializeFillValue(p.mProperty, p.mFromValue.ref()).get());
    } else {
      printf("  from: <null>\n");
    }
    printf("  to: %s\n", SerializeFillValue(p.mProperty, p.mToValue).get());

    if (p.mLastValue) {
      printf("  last: %s\n",
             SerializeFillValue(p.mProperty, p.mLastValue.ref()).get());
    } else {
      printf("  last: <null>\n");
    }

    // Timing
    printf("  portion:           %f\n", p.mPortion);
    printf("  current iteration: %lu\n", p.mCurrentIteration);
  }
}
#endif

}  // namespace mozilla
