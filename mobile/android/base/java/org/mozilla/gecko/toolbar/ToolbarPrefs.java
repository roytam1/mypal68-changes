/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.gecko.toolbar;

import org.mozilla.gecko.PrefsHelper;
import org.mozilla.gecko.Tab;
import org.mozilla.gecko.Tabs;
import org.mozilla.gecko.util.ThreadUtils;

class ToolbarPrefs {
    private static final String PREF_AUTOCOMPLETE_ENABLED = "browser.urlbar.autocomplete.enabled";
    private static final String PREF_TRIM_URLS = "browser.urlbar.trimURLs";

    private static final String[] PREFS = {
        PREF_AUTOCOMPLETE_ENABLED,
        PREF_TRIM_URLS
    };

    private final TitlePrefsHandler HANDLER = new TitlePrefsHandler();

    private volatile boolean enableAutocomplete;
    private volatile boolean trimUrls;

    ToolbarPrefs() {
        // Skip autocompletion while Gecko is loading.
        // We will get the correct pref value once Gecko is loaded.
        enableAutocomplete = false;
        trimUrls = true;
    }

    boolean shouldAutocomplete() {
        return enableAutocomplete;
    }

    boolean shouldTrimUrls() {
        return trimUrls;
    }

    void open() {
        PrefsHelper.addObserver(PREFS, HANDLER);
    }

    void close() {
        PrefsHelper.removeObserver(HANDLER);
    }

    private void triggerTitleChangeListener() {
        ThreadUtils.postToUiThread(new Runnable() {
            @Override
            public void run() {
                final Tabs tabs = Tabs.getInstance();
                final Tab tab = tabs.getSelectedTab();
                if (tab != null) {
                    tabs.notifyListeners(tab, Tabs.TabEvents.TITLE);
                }
            }
        });
    }

    private class TitlePrefsHandler extends PrefsHelper.PrefHandlerBase {
        @Override
        public void prefValue(String pref, boolean value) {
            if (PREF_AUTOCOMPLETE_ENABLED.equals(pref)) {
                enableAutocomplete = value;

            } else if (PREF_TRIM_URLS.equals(pref)) {
                // Handles PREF_TRIM_URLS, which should usually be a boolean.
                if (value != trimUrls) {
                    trimUrls = value;
                    triggerTitleChangeListener();
                }
            }
        }
    }
}
