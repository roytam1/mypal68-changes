/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_dom_AudioTrack_h
#define mozilla_dom_AudioTrack_h

#include "MediaTrack.h"

namespace mozilla {
namespace dom {

class AudioTrack : public MediaTrack {
 public:
  AudioTrack(nsIGlobalObject* aOwnerGlobal, const nsAString& aId,
             const nsAString& aKind, const nsAString& aLabel,
             const nsAString& aLanguage, bool aEnabled);

  JSObject* WrapObject(JSContext* aCx,
                       JS::Handle<JSObject*> aGivenProto) override;

  AudioTrack* AsAudioTrack() override { return this; }

  void SetEnabledInternal(bool aEnabled, int aFlags) override;

  // WebIDL
  bool Enabled() const { return mEnabled; }

  void SetEnabled(bool aEnabled);

 private:
  bool mEnabled;
};

}  // namespace dom
}  // namespace mozilla

#endif  // mozilla_dom_AudioTrack_h
