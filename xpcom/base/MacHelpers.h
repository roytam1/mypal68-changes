/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_MacHelpers_h
#define mozilla_MacHelpers_h

#include "nsString.h"

namespace mozilla {

nsresult GetSelectedCityInfo(nsAString& aCountryCode);

}  // namespace mozilla

#endif
