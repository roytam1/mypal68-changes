/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef nsKeygenHandlerContent_h
#define nsKeygenHandlerContent_h

#include "mozilla/Attributes.h"
#include "nsIFormProcessor.h"
#include "nsStringFwd.h"
#include "nsTArray.h"

namespace mozilla {
namespace dom {
class Element;
}  // namespace dom
}  // namespace mozilla

class nsKeygenFormProcessorContent final : public nsIFormProcessor {
 public:
  nsKeygenFormProcessorContent();

  virtual nsresult ProcessValue(mozilla::dom::Element* aElement,
                                const nsAString& aName,
                                nsAString& aValue) override;

  virtual nsresult ProcessValueIPC(const nsAString& aOldValue,
                                   const nsAString& aChallenge,
                                   const nsAString& aKeyType,
                                   const nsAString& aKeyParams,
                                   nsAString& aNewValue) override;

  virtual nsresult ProvideContent(const nsAString& aFormType,
                                  nsTArray<nsString>& aContent,
                                  nsAString& aAttribute) override;

  NS_DECL_ISUPPORTS

 protected:
  ~nsKeygenFormProcessorContent();
};

#endif  // nsKeygenHandlerContent_h
