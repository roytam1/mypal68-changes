/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef __GMPTestMonitor_h__
#define __GMPTestMonitor_h__

#include "nsThreadUtils.h"
#include "mozilla/SystemGroup.h"

class GMPTestMonitor {
 public:
  GMPTestMonitor() : mFinished(false) {}

  void AwaitFinished() {
    MOZ_ASSERT(NS_IsMainThread());
    mozilla::SpinEventLoopUntil([&]() { return mFinished; });
    mFinished = false;
  }

 private:
  void MarkFinished() {
    MOZ_ASSERT(NS_IsMainThread());
    mFinished = true;
  }

 public:
  void SetFinished() {
    mozilla::SystemGroup::Dispatch(mozilla::TaskCategory::Other,
                                   mozilla::NewNonOwningRunnableMethod(
                                       "GMPTestMonitor::MarkFinished", this,
                                       &GMPTestMonitor::MarkFinished));
  }

 private:
  bool mFinished;
};

#endif  // __GMPTestMonitor_h__
