/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_mscom_ProcessRuntimeShared_h
#define mozilla_mscom_ProcessRuntimeShared_h

#include "mozilla/Assertions.h"
#include "mozilla/Attributes.h"
#include "mozilla/Types.h"

namespace mozilla {
namespace mscom {
namespace detail {

MFBT_API bool& BeginProcessRuntimeInit();
MFBT_API void EndProcessRuntimeInit();

}  // namespace detail

class MOZ_RAII ProcessInitLock final {
 public:
  ProcessInitLock()
      : mIsProcessInitialized(detail::BeginProcessRuntimeInit()) {}

  ~ProcessInitLock() { detail::EndProcessRuntimeInit(); }

  bool IsInitialized() const { return mIsProcessInitialized; }

  void SetInitialized() {
    MOZ_ASSERT(!mIsProcessInitialized);
    mIsProcessInitialized = true;
  }

  ProcessInitLock(const ProcessInitLock&) = delete;
  ProcessInitLock(ProcessInitLock&&) = delete;
  ProcessInitLock operator=(const ProcessInitLock&) = delete;
  ProcessInitLock operator=(ProcessInitLock&&) = delete;

 private:
  bool& mIsProcessInitialized;
};

}  // namespace mscom
}  // namespace mozilla

#endif  // mozilla_mscom_ProcessRuntimeShared_h
