/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef nsDirIndex_h__
#define nsDirIndex_h__

#include "nsIDirIndex.h"
#include "nsString.h"
#include "mozilla/Attributes.h"

class nsDirIndex final : public nsIDirIndex {
 private:
  ~nsDirIndex() = default;

 public:
  nsDirIndex();

  NS_DECL_ISUPPORTS
  NS_DECL_NSIDIRINDEX

 protected:
  uint32_t mType;
  nsCString mContentType;
  nsCString mLocation;
  nsString mDescription;
  int64_t mSize;
  PRTime mLastModified;
};

#endif
