/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef nsProperties_h___
#define nsProperties_h___

#include "nsIProperties.h"
#include "nsInterfaceHashtable.h"
#include "nsHashKeys.h"
#include "mozilla/Attributes.h"

typedef nsInterfaceHashtable<nsCharPtrHashKey, nsISupports>
    nsProperties_HashBase;

class nsProperties final : public nsIProperties, public nsProperties_HashBase {
 public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIPROPERTIES

  nsProperties() = default;

 private:
  ~nsProperties() = default;
};

#endif /* nsProperties_h___ */
