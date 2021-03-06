/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_WindowsConsole_h
#define mozilla_WindowsConsole_h

namespace mozilla {

// This code attaches the process to the appropriate console.
void UseParentConsole();

}  // namespace mozilla

#endif  // mozilla_WindowsConsole_h
