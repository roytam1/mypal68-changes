/* Any copyright is dedicated to the Public Domain.
   http://creativecommons.org/publicdomain/zero/1.0/ */

var DEVTOOLS_CHROME_ENABLED = "devtools.chrome.enabled";

function test() {
  waitForExplicitFinish();

  Services.prefs.setBoolPref(DEVTOOLS_CHROME_ENABLED, false);
  gBrowser.selectedTab = BrowserTestUtils.addTab(gBrowser);
  BrowserTestUtils.browserLoaded(gBrowser.selectedBrowser).then(function() {
    openScratchpad(runTests);
  });

  BrowserTestUtils.loadURI(
    gBrowser,
    "data:text/html,initialization test for Scratchpad"
  );
}

function runTests() {
  const sp = gScratchpadWindow.Scratchpad;
  ok(sp, "Scratchpad object exists in new window");
  is(typeof sp.run, "function", "Scratchpad.run() exists");
  is(typeof sp.inspect, "function", "Scratchpad.inspect() exists");
  is(typeof sp.display, "function", "Scratchpad.display() exists");

  const environmentMenu = gScratchpadWindow.document.getElementById(
    "sp-environment-menu"
  );
  ok(environmentMenu, "Environment menu element exists");
  ok(environmentMenu.hasAttribute("hidden"), "Environment menu is not visible");

  const errorConsoleCommand = gScratchpadWindow.document.getElementById(
    "sp-cmd-errorConsole"
  );
  ok(errorConsoleCommand, "Error console command element exists");
  is(
    errorConsoleCommand.getAttribute("disabled"),
    "true",
    "Error console command is disabled"
  );

  const chromeContextCommand = gScratchpadWindow.document.getElementById(
    "sp-cmd-browserContext"
  );
  ok(chromeContextCommand, "Chrome context command element exists");
  is(
    chromeContextCommand.getAttribute("disabled"),
    "true",
    "Chrome context command is disabled"
  );

  Services.prefs.clearUserPref(DEVTOOLS_CHROME_ENABLED);
  finish();
}
