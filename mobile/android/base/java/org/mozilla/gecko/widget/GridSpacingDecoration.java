/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.gecko.widget;

import android.graphics.Rect;
import android.support.v7.widget.GridLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.view.View;

import org.mozilla.gecko.util.ViewUtil;

/**
 * An ItemDecoration for a GridLayoutManager that provides fixed spacing (but not fixed padding)
 * to create fixed sized items, with no spacing on the outer edges of the outer items.
 * <p>
 * So, for example, if there are 2 columns and the spacing is s, then the first column gets a right
 * padding of s/2 and the second column gets a left paddding of s/2.  If there are three columns
 * then the first column gets a right padding of 2s/3, the second column gets left and right
 * paddings of s/3, and the third column gets a left padding of 2s/3.
 * </p>
 */
public class GridSpacingDecoration extends RecyclerView.ItemDecoration {
    private final int horizontalSpacing;
    private final int verticalPadding;

    public GridSpacingDecoration(int horizontalSpacing, int verticalPadding) {
        this.horizontalSpacing = horizontalSpacing;
        this.verticalPadding = verticalPadding;
    }

    @Override
    public void getItemOffsets(Rect outRect, View view, RecyclerView parent, RecyclerView.State state) {
        final int position = parent.getChildAdapterPosition(view);
        if (position == RecyclerView.NO_POSITION) {
            return;
        }

        final GridLayoutManager layoutManager = (GridLayoutManager) parent.getLayoutManager();
        final int spanCount = layoutManager.getSpanCount();
        // If we're RTL then column counts start from the right, but view is still LTR (i.e. offsets
        // are still LTR), so compute offsets using the LTR column position (also note that this
        // only works because offsets in a given column are the same for LTR and RTL layouts).
        final int LTRColumn = ViewUtil.isLayoutRtl(parent) ? (spanCount - 1) - (position % spanCount) : position % spanCount;

        final int columnLeftOffset = (int) (((float) LTRColumn / (float) spanCount) * horizontalSpacing);
        final int columnRightOffset = (int) (((float) (spanCount - (LTRColumn + 1)) / (float) spanCount) * horizontalSpacing);

        outRect.set(columnLeftOffset, verticalPadding, columnRightOffset, verticalPadding);
    }
}
