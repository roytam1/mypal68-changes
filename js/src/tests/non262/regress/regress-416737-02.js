/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

//-----------------------------------------------------------------------------
var BUGNUMBER = 416737;
var summary = 'Do not assert: *pc == JSOP_GETARG';
var actual = '';
var expect = '';


//-----------------------------------------------------------------------------
test();
//-----------------------------------------------------------------------------

function test()
{
  printBugNumber(BUGNUMBER);
  printStatus (summary);

  var f = function([]){ function n(){} };
  if (typeof dis == 'function')
  {
    dis(f);
  }
  print(f);

  reportCompare(expect, actual, summary);
}
