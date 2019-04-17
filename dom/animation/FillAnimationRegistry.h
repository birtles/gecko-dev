/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_FillAnimationRegistry_h
#define mozilla_FillAnimationRegistry_h

#include "mozilla/HashTable.h"
#include "mozilla/WeakPtr.h"
#include "nsTArray.h"

namespace mozilla {

class CompactFillEffect;

namespace dom {
class FillAnimation;
}  // namespace dom

class FillAnimationRegistry {
 public:
  dom::FillAnimation* GetFillAnimationForEffects(
      const nsTArray<CompactFillEffect*>& aEffects);
  void RegisterFillAnimation(dom::FillAnimation& aFillAnimation,
                             const nsTArray<CompactFillEffect*>& aEffects);

 private:
  struct FillAnimationHasher {
    using Key = nsTArray<uint64_t>;
    using Lookup = Key;

    static HashNumber hash(const Lookup& aLookup);
    static bool match(const Key& aKey, const Lookup& aLookup);

    static void rekey(Key& aKey, Key&& aNewKey) { aKey = std::move(aNewKey); }
  };
  using FillAnimationHashMap =
      HashMap<nsTArray<uint64_t>, WeakPtr<dom::FillAnimation>,
              FillAnimationHasher>;
  FillAnimationHashMap mHashMap;
};

}  // namespace mozilla

#endif  // mozilla_dom_FillAnimationRegistry_h
