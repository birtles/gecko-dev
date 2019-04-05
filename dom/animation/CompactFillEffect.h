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

  CompactFillEffect* AsCompactFillEffect() override { return this; }

 protected:
  nsTArray<AnimationProperty> BuildProperties(
      const ComputedStyle* aStyle) override;

  FillSnapshot mFillSnapshot;
};

}  // namespace mozilla

#endif  // mozilla_CompactFillEffect_h
