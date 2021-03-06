/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_TimelineMarkerEnums_h_
#define mozilla_TimelineMarkerEnums_h_

namespace mozilla {

enum class MarkerTracingType { START, END, TIMESTAMP, HELPER_EVENT };

enum class MarkerStackRequest { STACK, NO_STACK };

}  // namespace mozilla

#endif  // mozilla_TimelineMarkerEnums_h_
