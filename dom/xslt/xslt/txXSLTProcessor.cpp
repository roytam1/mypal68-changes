/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "txXSLTProcessor.h"
#include "txInstructions.h"
#include "nsGkAtoms.h"
#include "txLog.h"
#include "txStylesheetCompileHandlers.h"
#include "txStylesheetCompiler.h"
#include "txExecutionState.h"
#include "txExprResult.h"

TX_LG_IMPL

/* static */
bool txXSLTProcessor::init() {
  TX_LG_CREATE;

  if (!txHandlerTable::init()) return false;

  extern bool TX_InitEXSLTFunction();
  if (!TX_InitEXSLTFunction()) return false;

  return true;
}

/* static */
void txXSLTProcessor::shutdown() { txHandlerTable::shutdown(); }

/* static */
nsresult txXSLTProcessor::execute(txExecutionState& aEs) {
  nsresult rv = NS_OK;
  txInstruction* instr;
  while ((instr = aEs.getNextInstruction())) {
    rv = instr->execute(aEs);
    NS_ENSURE_SUCCESS(rv, rv);
  }

  return NS_OK;
}
