/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "2D.h"
#include "DataSourceSurfaceWrapper.h"

namespace mozilla {
namespace gfx {

already_AddRefed<DataSourceSurface> DataSourceSurface::GetDataSurface() {
  RefPtr<DataSourceSurface> surface =
      IsDataSourceSurface() ? this : new DataSourceSurfaceWrapper(this);
  return surface.forget();
}

}  // namespace gfx
}  // namespace mozilla
