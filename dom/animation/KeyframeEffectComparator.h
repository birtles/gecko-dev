/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_KeyframeEffectComparator_h
#define mozilla_KeyframeEffectComparator_h

#include "mozilla/dom/Animation.h"
#include "mozilla/dom/KeyframeEffect.h"

namespace mozilla {

class KeyframeEffectComparator {
 public:
  bool Equals(const dom::KeyframeEffect* a,
              const dom::KeyframeEffect* b) const {
    return a == b;
  }

  bool LessThan(const dom::KeyframeEffect* a,
                const dom::KeyframeEffect* b) const {
    MOZ_ASSERT(a->GetAnimation() && b->GetAnimation());
    MOZ_ASSERT(
        Equals(a, b) ||
        a->GetAnimation()->HasLowerCompositeOrderThan(*b->GetAnimation()) !=
            b->GetAnimation()->HasLowerCompositeOrderThan(*a->GetAnimation()));
    return a->GetAnimation()->HasLowerCompositeOrderThan(*b->GetAnimation());
  }
};

}  // namespace mozilla

#endif  // mozilla_KeyframeEffectComparator_h
