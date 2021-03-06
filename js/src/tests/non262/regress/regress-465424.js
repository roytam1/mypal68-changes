/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

//-----------------------------------------------------------------------------
var BUGNUMBER = 465424;
var summary = 'TM: issue with post-decrement operator';
var actual = '';
var expect = '';


//-----------------------------------------------------------------------------
test();
//-----------------------------------------------------------------------------

function test()
{
  printBugNumber(BUGNUMBER);
  printStatus (summary);

  expect = '0,1,2,3,4,';

  for (let j=0;j<5;++j) { jj=j; print(actual += '' + (jj--) + ',') }

  reportCompare(expect, actual, summary);
}
