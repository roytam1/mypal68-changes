/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_dom_RemoteWorkerServiceParent_h
#define mozilla_dom_RemoteWorkerServiceParent_h

#include "mozilla/dom/PRemoteWorkerServiceParent.h"

namespace mozilla {
namespace dom {

class RemoteWorkerManager;

class RemoteWorkerServiceParent final : public PRemoteWorkerServiceParent {
 public:
  RemoteWorkerServiceParent();
  ~RemoteWorkerServiceParent();

  void ActorDestroy(mozilla::ipc::IProtocol::ActorDestroyReason) override;

  void Initialize();

 private:
  RefPtr<RemoteWorkerManager> mManager;
};

}  // namespace dom
}  // namespace mozilla

#endif  // mozilla_dom_RemoteWorkerServiceParent_h
