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
  void KeepFillAnimationAlive(dom::FillAnimation& aFillAnimation);
  void Compact();

  friend void ImplCycleCollectionTraverse(nsCycleCollectionTraversalCallback&,
                                          FillAnimationRegistry&, const char*,
                                          uint32_t);
  friend void ImplCycleCollectionUnlink(FillAnimationRegistry&);

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

  // Animations we are keeping alive because they have been mutated
  HashSet<RefPtr<dom::FillAnimation>> mPreservedAnimations;
};

inline void ImplCycleCollectionTraverse(
    nsCycleCollectionTraversalCallback& aCallback, FillAnimationRegistry& tmp,
    const char* aName, uint32_t aFlags) {
  for (auto iter = tmp.mPreservedAnimations.iter(); !iter.done(); iter.next()) {
    CycleCollectionNoteChild(aCallback, iter.get().get(),
                             "mPreservedAnimations", aFlags);
  }
}

inline void ImplCycleCollectionUnlink(FillAnimationRegistry& tmp) {
  tmp.mPreservedAnimations.clear();
}

}  // namespace mozilla

#endif  // mozilla_dom_FillAnimationRegistry_h
