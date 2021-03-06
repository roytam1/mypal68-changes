/* Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.mozilla.gecko.sqlite;

import java.nio.ByteBuffer;
import java.util.ArrayList;

import org.mozilla.gecko.annotation.JNITarget;

import android.database.AbstractCursor;
import android.database.CursorIndexOutOfBoundsException;
import android.util.Log;

/**
 * A mutable cursor implementation backed by an array of {@code Object}s. Use
 * {@link #newRow()} to add rows. Automatically expands internal capacity
 * as needed.
 *
 * This class provides one missing feature from Android's MatrixCursor:
 * the implementation of getBlob that was inadvertently omitted from API 9 (and
 * perhaps later; it's present in 14).
 *
 * MatrixCursor is all private, so we entirely duplicate it here.
 */
public class MatrixBlobCursor extends AbstractCursor {
    private static final String LOGTAG = "GeckoMatrixCursor";

    private final String[] mColumnNames;
    private final int mColumnCount;

    private int mRowCount;
    private Throwable mAllocationStack;

    Object[] mData;

    /**
     * Constructs a new cursor with the given initial capacity.
     *
     * @param columnNames names of the columns, the ordering of which
     *  determines column ordering elsewhere in this cursor
     * @param initialCapacity in rows
     */
    @JNITarget
    public MatrixBlobCursor(final String[] columnNames, final int initialCapacity) {
        mColumnNames = columnNames;
        mColumnCount = columnNames.length;

        int capacity = initialCapacity;
        if (capacity < 1) {
            capacity = 1;
        }

        mData = new Object[mColumnCount * capacity];
        mAllocationStack = new Throwable("allocationStack");
    }

    /**
     * Constructs a new cursor.
     *
     * @param columnNames names of the columns, the ordering of which
     *  determines column ordering elsewhere in this cursor
     */
    @JNITarget
    public MatrixBlobCursor(final String[] columnNames) {
        this(columnNames, 16);
    }

    /**
     * Closes the Cursor, releasing all of its resources.
     */
    public void close() {
        mAllocationStack = null;
        mData = null;
        super.close();
    }

    /**
     * Gets value at the given column for the current row.
     */
    protected Object get(final int column) {
        if (column < 0 || column >= mColumnCount) {
            throw new CursorIndexOutOfBoundsException("Requested column: "
                    + column + ", # of columns: " +  mColumnCount);
        }
        if (mPos < 0) {
            throw new CursorIndexOutOfBoundsException("Before first row.");
        }
        if (mPos >= mRowCount) {
            throw new CursorIndexOutOfBoundsException("After last row.");
        }
        return mData[mPos * mColumnCount + column];
    }

    /**
     * Adds a new row to the end and returns a builder for that row. Not safe
     * for concurrent use.
     *
     * @return builder which can be used to set the column values for the new
     *  row
     */
    public RowBuilder newRow() {
        mRowCount++;
        int endIndex = mRowCount * mColumnCount;
        ensureCapacity(endIndex);
        int start = endIndex - mColumnCount;
        return new RowBuilder(start, endIndex);
    }

    /**
     * Adds a new row to the end with the given column values. Not safe
     * for concurrent use.
     *
     * @throws IllegalArgumentException if {@code columnValues.length !=
     *  columnNames.length}
     * @param columnValues in the same order as the the column names specified
     *  at cursor construction time
     */
    @JNITarget
    public void addRow(final Object[] columnValues) {
        if (columnValues.length != mColumnCount) {
            throw new IllegalArgumentException("columnNames.length = "
                    + mColumnCount + ", columnValues.length = "
                    + columnValues.length);
        }

        int start = mRowCount++ * mColumnCount;
        ensureCapacity(start + mColumnCount);
        System.arraycopy(columnValues, 0, mData, start, mColumnCount);
    }

    /**
     * Adds a new row to the end with the given column values. Not safe
     * for concurrent use.
     *
     * @throws IllegalArgumentException if {@code columnValues.size() !=
     *  columnNames.length}
     * @param columnValues in the same order as the the column names specified
     *  at cursor construction time
     */
    @JNITarget
    public void addRow(final Iterable<?> columnValues) {
        final int start = mRowCount * mColumnCount;

        if (columnValues instanceof ArrayList<?>) {
            addRow((ArrayList<?>) columnValues, start);
            return;
        }

        final int end = start + mColumnCount;
        int current = start;

        ensureCapacity(end);
        final Object[] localData = mData;
        for (Object columnValue : columnValues) {
            if (current == end) {
                // TODO: null out row?
                throw new IllegalArgumentException(
                        "columnValues.size() > columnNames.length");
            }
            localData[current++] = columnValue;
        }

        if (current != end) {
            // TODO: null out row?
            throw new IllegalArgumentException(
                    "columnValues.size() < columnNames.length");
        }

        // Increase row count here in case we encounter an exception.
        mRowCount++;
    }

    /** Optimization for {@link ArrayList}. */
    @JNITarget
    private void addRow(final ArrayList<?> columnValues, final int start) {
        final int size = columnValues.size();
        if (size != mColumnCount) {
            throw new IllegalArgumentException("columnNames.length = "
                    + mColumnCount + ", columnValues.size() = " + size);
        }

        final int end = start + mColumnCount;
        ensureCapacity(end);

        // Take a reference just in case someone calls ensureCapacity
        // and `data` gets replaced by a new array!
        final Object[] localData = mData;
        for (int i = 0; i < size; i++) {
            localData[start + i] = columnValues.get(i);
        }

        mRowCount++;
    }

    /**
     * Ensures that this cursor has enough capacity. If it needs to allocate
     * a new array, the existing capacity will be at least doubled.
     */
    private void ensureCapacity(final int size) {
        if (size <= mData.length) {
            return;
        }

        final Object[] oldData = mData;
        mData = new Object[Math.max(size, mData.length * 2)];
        System.arraycopy(oldData, 0, mData, 0, oldData.length);
    }

    /**
     * Builds a row, starting from the left-most column and adding one column
     * value at a time. Follows the same ordering as the column names specified
     * at cursor construction time.
     *
     * Not thread-safe.
     */
    public class RowBuilder {
        private int mIndex;
        private final int mEndIndex;

        RowBuilder(final int index, final int endIndex) {
            this.mIndex = index;
            this.mEndIndex = endIndex;
        }

        /**
         * Sets the next column value in this row.
         *
         * @throws CursorIndexOutOfBoundsException if you try to add too many
         *  values
         * @return this builder to support chaining
         */
        public RowBuilder add(final Object columnValue) {
            if (mIndex == mEndIndex) {
                throw new CursorIndexOutOfBoundsException("No more columns left.");
            }

            mData[mIndex++] = columnValue;
            return this;
        }
    }

    /**
     * Not thread safe.
     */
    public void set(final int column, final Object value) {
        if (column < 0 || column >= mColumnCount) {
            throw new CursorIndexOutOfBoundsException("Requested column: "
                    + column + ", # of columns: " +  mColumnCount);
        }
        if (mPos < 0) {
            throw new CursorIndexOutOfBoundsException("Before first row.");
        }
        if (mPos >= mRowCount) {
            throw new CursorIndexOutOfBoundsException("After last row.");
        }
        mData[mPos * mColumnCount + column] = value;
    }

    // AbstractCursor implementation.
    @Override
    public int getCount() {
        return mRowCount;
    }

    @Override
    public String[] getColumnNames() {
        return mColumnNames;
    }

    @Override
    public String getString(final int column) {
        Object value = get(column);
        if (value == null) return null;
        return value.toString();
    }

    @Override
    public short getShort(final int column) {
        final Object value = get(column);
        if (value == null) return 0;
        if (value instanceof Number) return ((Number) value).shortValue();
        return Short.parseShort(value.toString());
    }

    @Override
    public int getInt(final int column) {
        Object value = get(column);
        if (value == null) return 0;
        if (value instanceof Number) return ((Number) value).intValue();
        return Integer.parseInt(value.toString());
    }

    @Override
    public long getLong(final int column) {
        Object value = get(column);
        if (value == null) return 0;
        if (value instanceof Number) return ((Number) value).longValue();
        return Long.parseLong(value.toString());
    }

    @Override
    public float getFloat(final int column) {
        Object value = get(column);
        if (value == null) return 0.0f;
        if (value instanceof Number) return ((Number) value).floatValue();
        return Float.parseFloat(value.toString());
    }

    @Override
    public double getDouble(final int column) {
        Object value = get(column);
        if (value == null) return 0.0d;
        if (value instanceof Number) return ((Number) value).doubleValue();
        return Double.parseDouble(value.toString());
    }

    @Override
    public byte[] getBlob(final int column) {
        Object value = get(column);
        if (value == null) return null;
        if (value instanceof byte[]) {
            return (byte[]) value;
        }

        if (value instanceof ByteBuffer) {
            final ByteBuffer bytes = (ByteBuffer) value;
            byte[] byteArray = new byte[bytes.remaining()];
            bytes.get(byteArray);
            return byteArray;
        }
        throw new UnsupportedOperationException("BLOB Object not of known type");
    }

    @Override
    public boolean isNull(final int column) {
        return get(column) == null;
    }

    @Override
    protected void finalize() {
        if (!isClosed()) {
            Log.e(LOGTAG, "Cursor finalized without being closed", mAllocationStack);
        }

        super.finalize();
    }
}
