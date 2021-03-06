/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "nsVersionComparatorImpl.h"
#include "nsVersionComparator.h"
#include "nsString.h"

NS_IMPL_ISUPPORTS(nsVersionComparatorImpl, nsIVersionComparator)

NS_IMETHODIMP
nsVersionComparatorImpl::Compare(const nsACString& aStr1,
                                 const nsACString& aStr2, int32_t* aResult) {
  *aResult = mozilla::CompareVersions(PromiseFlatCString(aStr1).get(),
                                      PromiseFlatCString(aStr2).get());

  return NS_OK;
}
