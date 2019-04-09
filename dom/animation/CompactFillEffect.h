/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_CompactFillEffect_h
#define mozilla_CompactFillEffect_h

#include "mozilla/dom/KeyframeEffect.h"
#include "mozilla/FillSnapshot.h"
#include "nsTArray.h"

namespace mozilla {

class ComputedStyle;

class CompactFillEffect : public dom::KeyframeEffect {
 protected:
  virtual ~CompactFillEffect() {}

 public:
  explicit CompactFillEffect(dom::KeyframeEffect& aOriginalEffect);

  NS_DECL_ISUPPORTS_INHERITED
  NS_DECL_CYCLE_COLLECTION_SCRIPT_HOLDER_CLASS_INHERITED(CompactFillEffect,
                                                         KeyframeEffect)

  CompactFillEffect* AsCompactFillEffect() override { return this; }
  void SetLinkedEffect(KeyframeEffect* aLinkedEffect) override;
  void NotifyAnimationCanceled() override;

  void AddReferencingEffect(KeyframeEffect& aKeyframeEffect);

  // Recompute the effect's stored fill style.
  void UpdateFill(FillSnapshot&& aFill, nsChangeHint aCumulativeChangeHint,
                  const ComputedStyle* aStyle);

 protected:
  nsTArray<AnimationProperty> BuildProperties(
      const ComputedStyle* aStyle) override;

  FillSnapshot mFillSnapshot;

  // Temporary measure to keep the original effect alive so we can return it
  // from GetAnimations until we introduce FillAnimations for this purpose.
  RefPtr<KeyframeEffect> mOriginalEffect;

  // Any KeyframeEffect-like objects that currently reference this effect.
  //
  // For indefinitely-filling animations returned from GetAnimations, we
  // return KeyframeEffect objects (implemented as FillEffects, but not
  // exposed as such to JS) that represent one or more CompactFillEffects.
  //
  // We need to track such references for two reasons:
  //
  // 1.  We won't coalesce this effect with other CompactFillEffects while such
  //     references still exist (since otherwise FillEffects would need
  //     a means to reference a subset of a CompactFillEffect).
  //
  // 2.  If we are canceled (via our |mLinkedEffect| for example) we need to
  //     cancel any such referencing effects too.
  //
  // However, we have no need to keep such objects alive. In fact, we hope
  // they disappear so we can be coalesced with adjacent CompactFillEffects.
  nsTArray<WeakPtr<KeyframeEffect>> mReferencingEffects;
};

}  // namespace mozilla

#endif  // mozilla_CompactFillEffect_h
