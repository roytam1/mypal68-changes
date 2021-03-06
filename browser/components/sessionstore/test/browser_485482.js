/* Any copyright is dedicated to the Public Domain.
 * http://creativecommons.org/publicdomain/zero/1.0/ */

"use strict";

const URL = ROOT + "browser_485482_sample.html";

/**
 * Bug 485482 - Make sure that we produce valid XPath expressions even for very
 * weird HTML documents.
 */
add_task(async function test_xpath_exp_for_strange_documents() {
  // Load a page with weird tag names.
  let tab = BrowserTestUtils.addTab(gBrowser, URL);
  let browser = tab.linkedBrowser;
  await promiseBrowserLoaded(browser);

  // Fill in some values.
  let uniqueValue = Math.random();
  await setInputValue(browser, {
    selector: "input[type=text]",
    value: uniqueValue,
  });
  await setInputChecked(browser, {
    selector: "input[type=checkbox]",
    checked: true,
  });

  // Duplicate the tab.
  let tab2 = gBrowser.duplicateTab(tab);
  let browser2 = tab2.linkedBrowser;
  await promiseTabRestored(tab2);

  // Check that we generated valid XPath expressions to restore form values.
  let text = await getInputValue(browser2, { selector: "input[type=text]" });
  is(text, uniqueValue, "generated XPath expression was valid");
  let checkbox = await getInputChecked(browser2, {
    selector: "input[type=checkbox]",
  });
  ok(checkbox, "generated XPath expression was valid");

  // Cleanup.
  gBrowser.removeTab(tab2);
  gBrowser.removeTab(tab);
});
