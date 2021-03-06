/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WEBGPU_BlendState_H_
#define WEBGPU_BlendState_H_

#include "nsWrapperCache.h"
#include "ObjectModel.h"

namespace mozilla {
namespace webgpu {

class Device;

class BlendState final : public ChildOf<Device> {
 public:
  WEBGPU_DECL_GOOP(BlendState)

 private:
  BlendState() = delete;
  virtual ~BlendState();
};

}  // namespace webgpu
}  // namespace mozilla

#endif  // WEBGPU_BlendState_H_
