/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
package org.mozilla.gecko.activitystream.homepanel.model;

import org.mozilla.gecko.activitystream.homepanel.StreamRecyclerAdapter;

public interface RowModel {
    StreamRecyclerAdapter.RowItemType getRowItemType();
}
