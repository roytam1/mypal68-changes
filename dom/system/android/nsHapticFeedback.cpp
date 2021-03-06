/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "nsHapticFeedback.h"
#include "GeneratedJNIWrappers.h"

using namespace mozilla;

NS_IMPL_ISUPPORTS(nsHapticFeedback, nsIHapticFeedback)

NS_IMETHODIMP
nsHapticFeedback::PerformSimpleAction(int32_t aType) {
  java::GeckoAppShell::PerformHapticFeedback(aType == LongPress);
  return NS_OK;
}
