/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_dom_FillAnimation_h
#define mozilla_dom_FillAnimation_h

#include "nsWrapperCache.h"
#include "nsCycleCollectionParticipant.h"
#include "mozilla/dom/Animation.h"
#include "mozilla/WeakPtr.h"

namespace mozilla {

class CompactFillEffect;

namespace dom {

class FillAnimation : public Animation,
                      public SupportsWeakPtr<FillAnimation> {
 protected:
  virtual ~FillAnimation() {}

 public:
  explicit FillAnimation(nsIGlobalObject* aGlobal) : Animation(aGlobal) {
    mIsReadOnly = true;
  }

  NS_DECL_ISUPPORTS_INHERITED
  MOZ_DECLARE_WEAKREFERENCE_TYPENAME(FillAnimation)

  nsIGlobalObject* GetParentObject() const { return GetOwnerGlobal(); }
  JSObject* WrapObject(JSContext* aCx,
                       JS::Handle<JSObject*> aGivenProto) override;

  FillAnimation* AsFillAnimation() override { return this; }
  const FillAnimation* AsFillAnimation() const override { return this; }

  static already_AddRefed<FillAnimation> Create(
      const nsTArray<CompactFillEffect*>& aSourceEffects);

  // Cancels the FillAnimation but in such a way that it doesn't cancel all the
  // CompactFillEffects it wraps.
  void Invalidate() { CancelNoUpdate(CancelMode::Invalidate); }

  bool ShouldKeepAlive() const;

  uint64_t NumSourceEffects() const;
};

}  // namespace dom
}  // namespace mozilla

#endif  // mozilla_dom_FillAnimation_h
