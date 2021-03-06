/* * This Source Code Form is subject to the terms of the Mozilla Public
 * * License, v. 2.0. If a copy of the MPL was not distributed with this
 * * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "FuzzerRegistry.h"

namespace mozilla {

class FuzzerRunner {
 public:
  int Run(int* argc, char*** argv);

#ifdef LIBFUZZER
  void setParams(LibFuzzerDriver aDriver);

 private:
  LibFuzzerDriver mFuzzerDriver;
#endif
};

extern FuzzerRunner* fuzzerRunner;

}  // namespace mozilla
