/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef GFX_WEBRENDERCANVASRENDERER_H
#define GFX_WEBRENDERCANVASRENDERER_H

#include "ShareableCanvasRenderer.h"

namespace mozilla {
namespace layers {

class RenderRootStateManager;

class WebRenderCanvasRenderer : public ShareableCanvasRenderer {
 public:
  explicit WebRenderCanvasRenderer(RenderRootStateManager* aManager)
      : mManager(aManager) {}

  void Initialize(const CanvasInitializeData& aData) override;

  CompositableForwarder* GetForwarder() override;

 protected:
  RenderRootStateManager* mManager;
};

class WebRenderCanvasRendererAsync : public WebRenderCanvasRenderer {
 public:
  explicit WebRenderCanvasRendererAsync(RenderRootStateManager* aManager)
      : WebRenderCanvasRenderer(aManager) {}
  virtual ~WebRenderCanvasRendererAsync();

  WebRenderCanvasRendererAsync* AsWebRenderCanvasRendererAsync() override {
    return this;
  }

  void Initialize(const CanvasInitializeData& aData) override;
  bool CreateCompositable() override;

  void ClearCachedResources() override;
  void Destroy() override;

  void UpdateCompositableClientForEmptyTransaction();

  Maybe<wr::PipelineId> GetPipelineId() { return mPipelineId; }

 protected:
  Maybe<wr::PipelineId> mPipelineId;
};

}  // namespace layers
}  // namespace mozilla

#endif
