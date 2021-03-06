"use strict";

add_task(async function test_sendMessage_without_listener() {
  async function background() {
    await browser.test.assertRejects(
      browser.runtime.sendMessage("msg"),
      "Could not establish connection. Receiving end does not exist.",
      "sendMessage callback was invoked"
    );

    browser.test.notifyPass("sendMessage callback was invoked");
  }
  let extensionData = {
    background,
  };

  let extension = ExtensionTestUtils.loadExtension(extensionData);
  await extension.startup();

  await extension.awaitFinish("sendMessage callback was invoked");

  await extension.unload();
});

add_task(async function test_chrome_sendMessage_without_listener() {
  function background() {
    /* globals chrome */
    browser.test.assertEq(
      null,
      chrome.runtime.lastError,
      "no lastError before call"
    );
    let retval = chrome.runtime.sendMessage("msg");
    browser.test.assertEq(
      null,
      chrome.runtime.lastError,
      "no lastError after call"
    );
    browser.test.assertEq(
      undefined,
      retval,
      "return value of chrome.runtime.sendMessage without callback"
    );

    let isAsyncCall = false;
    retval = chrome.runtime.sendMessage("msg", reply => {
      browser.test.assertEq(undefined, reply, "no reply");
      browser.test.assertTrue(
        isAsyncCall,
        "chrome.runtime.sendMessage's callback must be called asynchronously"
      );
      browser.test.assertEq(
        undefined,
        retval,
        "return value of chrome.runtime.sendMessage with callback"
      );
      browser.test.assertEq(
        "Could not establish connection. Receiving end does not exist.",
        chrome.runtime.lastError.message
      );
      browser.test.notifyPass("finished chrome.runtime.sendMessage");
    });
    isAsyncCall = true;
  }
  let extensionData = {
    background,
  };

  let extension = ExtensionTestUtils.loadExtension(extensionData);
  await extension.startup();

  await extension.awaitFinish("finished chrome.runtime.sendMessage");

  await extension.unload();
});
