/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "WebRenderTypes.h"

#include "mozilla/ipc/ByteBuf.h"

namespace mozilla {
namespace wr {

WindowId NewWindowId() {
  static uint64_t sNextId = 1;

  WindowId id;
  id.mHandle = sNextId++;
  return id;
}

void Assign_WrVecU8(wr::WrVecU8& aVec, mozilla::ipc::ByteBuf&& aOther) {
  aVec.data = aOther.mData;
  aVec.length = aOther.mLen;
  aVec.capacity = aOther.mCapacity;
  aOther.mData = nullptr;
  aOther.mLen = 0;
  aOther.mCapacity = 0;
}

WrSpaceAndClip RootScrollNode() {
  WrSpaceAndClip sac;
  sac.clip = wr_root_clip_id();
  sac.space = wr_root_scroll_node_id();
  return sac;
}

WrSpaceAndClipChain RootScrollNodeWithChain() {
  WrSpaceAndClipChain sacc;
  sacc.clip_chain = wr::ROOT_CLIP_CHAIN;
  sacc.space = wr_root_scroll_node_id();
  return sacc;
}

RenderRoot RenderRootFromId(DocumentId id) {
  MOZ_ASSERT(id.mHandle < kRenderRootCount);
  return (RenderRoot)id.mHandle;
}

}  // namespace wr
}  // namespace mozilla
