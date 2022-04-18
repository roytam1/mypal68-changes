/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef frontend_NameFunctions_h
#define frontend_NameFunctions_h

#include "mozilla/Attributes.h"

#include "js/TypeDecls.h"

namespace js {
namespace frontend {

class ParseNode;

MOZ_MUST_USE bool NameFunctions(JSContext* cx, ParseNode* pn);

} /* namespace frontend */
} /* namespace js */

#endif /* frontend_NameFunctions_h */
