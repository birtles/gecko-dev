/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_CompactFillEffect_h
#define mozilla_CompactFillEffect_h

#include "mozilla/dom/KeyframeEffect.h"
#include "mozilla/FillSnapshot.h"

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
};

}  // namespace mozilla

#endif  // mozilla_CompactFillEffect_h
