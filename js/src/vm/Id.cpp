/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "js/Id.h"
#include "js/RootingAPI.h"

static const jsid voidIdValue = JSID_VOID;
static const jsid emptyIdValue = JSID_EMPTY;
const JS::HandleId JSID_VOIDHANDLE =
    JS::HandleId::fromMarkedLocation(&voidIdValue);
const JS::HandleId JSID_EMPTYHANDLE =
    JS::HandleId::fromMarkedLocation(&emptyIdValue);
