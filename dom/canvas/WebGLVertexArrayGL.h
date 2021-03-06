/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WEBGL_VERTEX_ARRAY_GL_H_
#define WEBGL_VERTEX_ARRAY_GL_H_

#include "WebGLVertexArray.h"

namespace mozilla {

class WebGLVertexArrayGL : public WebGLVertexArray {
  friend class WebGLVertexArray;

 public:
  virtual void DeleteImpl() override;
  virtual void BindVertexArray() override;

 protected:
  explicit WebGLVertexArrayGL(WebGLContext* webgl);
  ~WebGLVertexArrayGL();
};

}  // namespace mozilla

#endif  // WEBGL_VERTEX_ARRAY_GL_H_
