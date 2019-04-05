/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_CompactAnimationUtils_h
#define mozilla_CompactAnimationUtils_h

namespace mozilla {

namespace dom {
class Animation;
}

class CompactAnimationUtils {
 public:
  static void CompactAnimation(dom::Animation& aAnimation);
  static void RestoreAnimation(dom::Animation& aAnimation);
};

}  // namespace mozilla

#endif  // mozilla_CompactAnimationUtils_h
