/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/GamepadMonitoring.h"
#include "mozilla/dom/GamepadPlatformService.h"
#include "mozilla/ipc/BackgroundParent.h"

using namespace mozilla::ipc;

namespace mozilla {
namespace dom {

void MaybeStopGamepadMonitoring() {
  AssertIsOnBackgroundThread();
  RefPtr<GamepadPlatformService> service =
      GamepadPlatformService::GetParentService();
  MOZ_ASSERT(service);
  if (service->HasGamepadListeners()) {
    return;
  }
  StopGamepadMonitoring();
  service->ResetGamepadIndexes();
  service->MaybeShutdown();
}

}  // namespace dom
}  // namespace mozilla
