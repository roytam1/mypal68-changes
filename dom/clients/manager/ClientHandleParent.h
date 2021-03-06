/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef _mozilla_dom_ClientHandleParent_h
#define _mozilla_dom_ClientHandleParent_h

#include "mozilla/dom/PClientHandleParent.h"

namespace mozilla {
namespace dom {

class ClientManagerService;
class ClientSourceParent;

class ClientHandleParent final : public PClientHandleParent {
  RefPtr<ClientManagerService> mService;
  ClientSourceParent* mSource;

  // PClientHandleParent interface
  mozilla::ipc::IPCResult RecvTeardown() override;

  void ActorDestroy(ActorDestroyReason aReason) override;

  PClientHandleOpParent* AllocPClientHandleOpParent(
      const ClientOpConstructorArgs& aArgs) override;

  bool DeallocPClientHandleOpParent(PClientHandleOpParent* aActor) override;

  mozilla::ipc::IPCResult RecvPClientHandleOpConstructor(
      PClientHandleOpParent* aActor,
      const ClientOpConstructorArgs& aArgs) override;

 public:
  ClientHandleParent();
  ~ClientHandleParent();

  void Init(const IPCClientInfo& aClientInfo);

  ClientSourceParent* GetSource() const;
};

}  // namespace dom
}  // namespace mozilla

#endif  // _mozilla_dom_ClientHandleParent_h
