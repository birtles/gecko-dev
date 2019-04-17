/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/FillAnimationRegistry.h"

#include "mozilla/CompactFillEffect.h"
#include "mozilla/dom/Animation.h"

namespace mozilla {

static nsTArray<uint64_t> AnimationIndices(
    const nsTArray<CompactFillEffect*>& aEffects) {
  nsTArray<uint64_t> result;
  for (auto effect : aEffects) {
    MOZ_ASSERT(effect->GetAnimation());
    result.AppendElement(effect->GetAnimation()->EffectiveAnimationIndex());
  }

  return result;
}

dom::FillAnimation* FillAnimationRegistry::GetFillAnimationForEffects(
    const nsTArray<CompactFillEffect*>& aEffects) {
  FillAnimationHashMap::Ptr p = mHashMap.lookup(AnimationIndices(aEffects));
  if (!p) {
    return nullptr;
  }

  // If the FillAnimation has been GC'ed or canceled, drop it so we can add
  // a new one in its place.
  if (!p->value() || p->value()->PlayState() == AnimationPlayState::Idle) {
    mHashMap.remove(p);
    return nullptr;
  }

  return p->value();
}

void FillAnimationRegistry::RegisterFillAnimation(
    dom::FillAnimation& aFillAnimation,
    const nsTArray<CompactFillEffect*>& aEffects) {
  // We're assuming the caller has already called GetFillAnimationForEffects
  // before calling this. HashTable has assertions to check this anyway.
  Unused << mHashMap.putNew(AnimationIndices(aEffects), &aFillAnimation);
}

void FillAnimationRegistry::KeepFillAnimationAlive(
    dom::FillAnimation& aFillAnimation) {
  Unused << mPreservedAnimations.put(
      RefPtr<dom::FillAnimation>(&aFillAnimation));
}

void FillAnimationRegistry::Compact() {
  for (auto iter = mHashMap.modIter(); !iter.done(); iter.next()) {
    // Drop any dangling weak references or canceled animations
    if (!iter.get().value() ||
        iter.get().value()->PlayState() == AnimationPlayState::Idle) {
      iter.remove();
    }
  }
  for (auto iter = mPreservedAnimations.modIter(); !iter.done(); iter.next()) {
    // Stop keeping alive any FillAnimations that no longer need it
    if (!iter.get()->ShouldKeepAlive()) {
      iter.remove();
    }
  }
}

/*static*/ HashNumber FillAnimationRegistry::FillAnimationHasher::hash(
    const Lookup& aLookup) {
  HashNumber result = 0;
  for (auto animationIndex : aLookup) {
    result = AddToHash(result, animationIndex);
  }
  return result;
}

/*static*/ bool FillAnimationRegistry::FillAnimationHasher::match(
    const Key& aKey, const Lookup& aLookup) {
  return aKey == aLookup;
}

}  // namespace mozilla
