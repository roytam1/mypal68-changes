/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_KeyframeEffectParams_h
#define mozilla_KeyframeEffectParams_h

#include "mozilla/dom/KeyframeEffectBinding.h"  // IterationCompositeOperation

namespace mozilla {

struct KeyframeEffectParams {
  dom::IterationCompositeOperation mIterationComposite =
      dom::IterationCompositeOperation::Replace;
  dom::CompositeOperation mComposite = dom::CompositeOperation::Replace;
};

}  // namespace mozilla

#endif  // mozilla_KeyframeEffectParams_h
