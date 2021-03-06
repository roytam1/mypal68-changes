/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "nsTArray.h"
#include "mozilla/TimeStamp.h"

namespace mozilla {
namespace dom {
class MIDIMessage;

/**
 * Set of utility functions for dealing with MIDI Messages.
 *
 */
namespace MIDIUtils {

// Takes a nsTArray of bytes and parses it into zero or more MIDI messages.
// Returns number of bytes parsed.
uint32_t ParseMessages(const nsTArray<uint8_t>& aByteBuffer,
                       const TimeStamp& aTimestamp,
                       nsTArray<MIDIMessage>& aMsgArray);
// Returns true if a message is a sysex message.
bool IsSysexMessage(const MIDIMessage& a);
}  // namespace MIDIUtils
}  // namespace dom
}  // namespace mozilla
