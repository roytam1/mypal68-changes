/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "ClientManagerChild.h"
#include "ClientManagerParent.h"

namespace mozilla {
namespace dom {

PClientManagerChild* AllocClientManagerChild() {
  MOZ_ASSERT_UNREACHABLE(
      "Default ClientManagerChild allocator should not be invoked");
  return nullptr;
}

bool DeallocClientManagerChild(PClientManagerChild* aActor) {
  delete aActor;
  return true;
}

PClientManagerParent* AllocClientManagerParent() {
  return new ClientManagerParent();
}

bool DeallocClientManagerParent(PClientManagerParent* aActor) {
  delete aActor;
  return true;
}

void InitClientManagerParent(PClientManagerParent* aActor) {
  auto actor = static_cast<ClientManagerParent*>(aActor);
  actor->Init();
}

}  // namespace dom
}  // namespace mozilla
