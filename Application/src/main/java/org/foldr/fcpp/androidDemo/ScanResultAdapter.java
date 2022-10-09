/*
 * Copyright (C) 2015 The Android Open Source Project
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

package org.foldr.fcpp.androidDemo;

import android.annotation.SuppressLint;
import android.bluetooth.le.ScanResult;
import android.content.Context;
import android.os.SystemClock;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.TextView;

import java.util.ArrayList;
import java.util.concurrent.TimeUnit;

/**
 * Holds and displays {@link ScanResult}s, used by {@link ScannerFragment}.
 */
public class ScanResultAdapter extends BaseAdapter {

    private ArrayList<ScanResult> mArrayList;

    private Context mContext;

    private LayoutInflater mInflater;

    ScanResultAdapter(Context context, LayoutInflater inflater) {
        super();
        mContext = context;
        mInflater = inflater;
        mArrayList = new ArrayList<>();
    }

    @Override
    public int getCount() {
        return mArrayList.size();
    }

    @Override
    public Object getItem(int position) {
        return mArrayList.get(position);
    }

    @Override
    public long getItemId(int position) {
        return getSenderUid(mArrayList.get(position));
    }

    @Override
    public View getView(int position, View view, ViewGroup parent) {

        // Reuse an old view if we can, otherwise create a new one.
        if (view == null) {
            view = mInflater.inflate(R.layout.listitem_scanresult, null);
        }

        TextView deviceNameView = (TextView) view.findViewById(R.id.device_name);
        TextView deviceAddressView = (TextView) view.findViewById(R.id.device_address);
        TextView lastSeenView = (TextView) view.findViewById(R.id.last_seen);

        ScanResult scanResult = mArrayList.get(position);

        @SuppressLint("MissingPermission") String name = scanResult.getDevice().getName();
        if (name == null) {
            name = mContext.getResources().getString(R.string.no_name);
        }
        int uid = getSenderUid(scanResult);
        deviceAddressView.setText(uid + ":"+scanResult.getDevice().getAddress());
        lastSeenView.setText(getTimeSinceString(mContext, scanResult.getTimestampNanos()));

        return view;
    }

    private int getSenderUid(ScanResult scanResult) {
        byte[] serviceData = scanResult.getScanRecord().getServiceData(Constants.Service_UUID);
        // Rotating hw-id useless here, we extract the contained UID:
        byte uid_lo = serviceData[serviceData.length-2];
        byte uid_hi = serviceData[serviceData.length-1];
        int uid = ((uid_hi << 8) & 0x0000ff00) | (uid_lo & 0x000000ff);
        return uid;
    }

    /**
     * Search the adapter for an existing device address and return it, otherwise return -1.
     */
    private int getPosition(int address) {
        int position = -1;
        for (int i = 0; i < mArrayList.size(); i++) {
            if (getSenderUid(mArrayList.get(i)) == address) {
                position = i;
                break;
            }
        }
        return position;
    }


    /**
     * Add a ScanResult item to the adapter if a result from that device isn't already present.
     * Otherwise updates the existing position with the new ScanResult.
     */
    public void add(ScanResult scanResult) {

        int existingPosition = getPosition(getSenderUid(scanResult));

        if (existingPosition >= 0) {
            // Device is already in list, update its record.
            mArrayList.set(existingPosition, scanResult);
        } else {
            // Add new Device's ScanResult to list.
            mArrayList.add(scanResult);
        }
    }

    /**
     * Clear out the adapter.
     */
    public void clear() {
        mArrayList.clear();
    }

    /**
     * Takes in a number of nanoseconds and returns a human-readable string giving a vague
     * description of how long ago that was.
     */
    public static String getTimeSinceString(Context context, long timeNanoseconds) {
        String lastSeenText = context.getResources().getString(R.string.last_seen) + " ";

        long timeSince = SystemClock.elapsedRealtimeNanos() - timeNanoseconds;
        long secondsSince = TimeUnit.SECONDS.convert(timeSince, TimeUnit.NANOSECONDS);

        if (secondsSince < 5) {
            lastSeenText += context.getResources().getString(R.string.just_now);
        } else if (secondsSince < 60) {
            lastSeenText += secondsSince + " " + context.getResources()
                    .getString(R.string.seconds_ago);
        } else {
            long minutesSince = TimeUnit.MINUTES.convert(secondsSince, TimeUnit.SECONDS);
            if (minutesSince < 60) {
                if (minutesSince == 1) {
                    lastSeenText += minutesSince + " " + context.getResources()
                            .getString(R.string.minute_ago);
                } else {
                    lastSeenText += minutesSince + " " + context.getResources()
                            .getString(R.string.minutes_ago);
                }
            } else {
                long hoursSince = TimeUnit.HOURS.convert(minutesSince, TimeUnit.MINUTES);
                if (hoursSince == 1) {
                    lastSeenText += hoursSince + " " + context.getResources()
                            .getString(R.string.hour_ago);
                } else {
                    lastSeenText += hoursSince + " " + context.getResources()
                            .getString(R.string.hours_ago);
                }
            }
        }

        return lastSeenText;
    }
}
