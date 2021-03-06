/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.gecko.util;

import android.graphics.Color;

public class ColorUtil {
    public static int darken(final int color, final double fraction) {
        int red = Color.red(color);
        int green = Color.green(color);
        int blue = Color.blue(color);
        red = darkenColor(red, fraction);
        green = darkenColor(green, fraction);
        blue = darkenColor(blue, fraction);
        final int alpha = Color.alpha(color);
        return Color.argb(alpha, red, green, blue);
    }

    public static int getReadableTextColor(final int backgroundColor) {
        final int greyValue = grayscaleFromRGB(backgroundColor);
        // 186 chosen rather than the seemingly obvious 128 because of gamma.
        if (greyValue < 186) {
            return Color.WHITE;
        } else {
            return Color.BLACK;
        }
    }

    public static Integer parseStringColor(final String color) {
        try {
            if (color.length() < 7) {
                return null;
            }
            return Color.argb(255,
                    Integer.valueOf(color.substring(1, 3), 16),
                    Integer.valueOf(color.substring(3, 5), 16),
                    Integer.valueOf(color.substring(5, 7), 16));
        } catch (NumberFormatException e) { }
        return null;
    }

    private static int darkenColor(final int color, final double fraction) {
        return (int) Math.max(color - (color * fraction), 0);
    }

    private static int grayscaleFromRGB(final int color) {
        final int red = Color.red(color);
        final int green = Color.green(color);
        final int blue = Color.blue(color);
        // Magic weighting taken from a stackoverflow post, supposedly related to how
        // humans perceive color.
        return (int) (0.299 * red + 0.587 * green + 0.114 * blue);
    }
}
