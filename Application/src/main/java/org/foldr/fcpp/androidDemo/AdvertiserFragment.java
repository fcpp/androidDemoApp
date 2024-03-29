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

import static org.foldr.fcpp.androidDemo.BLEParameterFragment.ARG_PARAM_BLE_INTERVAL;
import static org.foldr.fcpp.androidDemo.BLEParameterFragment.ARG_PARAM_BLE_POWER_LEVEL;

import android.bluetooth.le.AdvertiseCallback;
import android.bluetooth.le.AdvertisingSetParameters;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;
import android.widget.Toast;

import androidx.appcompat.widget.SwitchCompat;
import androidx.fragment.app.Fragment;

/**
 * Allows user to start & stop Bluetooth LE Advertising of their device.
 */
public class AdvertiserFragment extends Fragment implements View.OnClickListener {

    /**
     * Lets user toggle BLE Advertising.
     */
    private SwitchCompat mSwitch;

    /**
     * Listens for notifications that the {@code AdvertiserService} has failed to start advertising.
     * This Receiver deals with Fragment UI elements and only needs to be active when the Fragment
     * is on-screen, so it's defined and registered in code instead of the Manifest.
     */
    private BroadcastReceiver advertisingFailureReceiver;

    public static final String ARG_BROADCAST_ON_FIRST_BOOT = "broadcast_on_first_boot";
    public static final String ARG_DISABLE_BROADCAST_SWITCH = "disable_broadcast_switch";
    private boolean firstBoot;
    private boolean isDisable;
    private int ble_power_level = AdvertisingSetParameters.TX_POWER_MEDIUM;
    private int ble_interval;
    private TextView tv;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        if (getArguments() != null) {
            firstBoot = getArguments().getBoolean(ARG_BROADCAST_ON_FIRST_BOOT, false);
            isDisable = getArguments().getBoolean(ARG_DISABLE_BROADCAST_SWITCH, false);
            ble_power_level = getArguments().getInt(ARG_PARAM_BLE_POWER_LEVEL, AdvertisingSetParameters.TX_POWER_MEDIUM);
            ble_interval = getArguments().getInt(ARG_PARAM_BLE_INTERVAL, AdvertisingSetParameters.INTERVAL_HIGH);
        }

        advertisingFailureReceiver = new BroadcastReceiver() {

            /**
             * Receives Advertising error codes from {@code AdvertiserService} and displays error messages
             * to the user. Sets the advertising toggle to 'false.'
             */
            @Override
            public void onReceive(Context context, Intent intent) {

                /* TODO: Should probably subclass... */
                if (AdvertiserService.ADVERTISING_FAILED.equals(intent.getAction())) {
                    int errorCode = intent.getIntExtra(AdvertiserService.ADVERTISING_FAILED_EXTRA_CODE, -1);
                    String errorMsg = intent.getStringExtra(AdvertiserService.ADVERTISING_FAILED_EXTRA_MSG);
                    mSwitch.setChecked(false);

                    String errorMessage = getString(R.string.start_error_prefix);
                    switch (errorCode) {
                        case AdvertiseCallback.ADVERTISE_FAILED_ALREADY_STARTED:
                            errorMessage += " " + getString(R.string.start_error_already_started);
                            break;
                        case AdvertiseCallback.ADVERTISE_FAILED_DATA_TOO_LARGE:
                            errorMessage += " " + getString(R.string.start_error_too_large);
                            break;
                        case AdvertiseCallback.ADVERTISE_FAILED_FEATURE_UNSUPPORTED:
                            errorMessage += " " + getString(R.string.start_error_unsupported);
                            break;
                        case AdvertiseCallback.ADVERTISE_FAILED_INTERNAL_ERROR:
                            errorMessage += " " + getString(R.string.start_error_internal);
                            break;
                        case AdvertiseCallback.ADVERTISE_FAILED_TOO_MANY_ADVERTISERS:
                            errorMessage += " " + getString(R.string.start_error_too_many);
                            break;
                        case AdvertiserService.ADVERTISING_TIMED_OUT:
                            errorMessage = " " + getString(R.string.advertising_timedout);
                            break;
                        default:
                            errorMessage += " " + getString(R.string.start_error_unknown);
                        if (errorMsg != null) {
                            errorMessage += "; " + errorMsg;
                        }
                    }

                    Toast.makeText(getActivity(), errorMessage, Toast.LENGTH_LONG).show();
                } else if (AdvertiserService.ADVERTISING_STORAGE.equals(intent.getAction())) {
                    String storage_text = intent.getStringExtra(AdvertiserService.ADVERTISING_STORAGE_EXTRA_CODE);
                    tv.setText(storage_text);
                } else {
                    Log.d("vs", intent.getAction());
                }
            }
        };
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {

        View view = inflater.inflate(R.layout.fragment_advertiser, container, false);
        tv = view.findViewById(R.id.storage_view);
        assert tv != null;

        mSwitch = view.findViewById(R.id.advertise_switch);
        if (isDisable) {
            mSwitch.setEnabled(false);
        } else {
            mSwitch.setOnClickListener(this);
        }

        View mQuit = view.findViewById(R.id.button_quit);
        mQuit.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                getActivity().stopService(new Intent(getActivity(), AdvertiserService.class));
                AdvertiserFragment.this.getActivity().finish();
                AP.fcpp_stop();
                System.exit(0);
            }
        });
        return view;
    }

    /**
     * When app comes on screen, check if BLE Advertisements are running, set switch accordingly,
     * and register the Receiver to be notified if Advertising fails.
     */
    @Override
    public void onResume() {
        super.onResume();

        mSwitch.setChecked(AdvertiserService.running);
        if (firstBoot) {
            // On first start, we immediately start broadcasting:
            firstBoot = false;
            startAdvertising();
            // Possibly out of sync for a bit:
            mSwitch.setChecked(true);
        } else {
            // Whatever the current state is:
            mSwitch.setChecked(AdvertiserService.running);
        }

        IntentFilter failureFilter = new IntentFilter(AdvertiserService.ADVERTISING_FAILED);
        getActivity().registerReceiver(advertisingFailureReceiver, failureFilter);
        IntentFilter storageFilter = new IntentFilter(AdvertiserService.ADVERTISING_STORAGE);
        getActivity().registerReceiver(advertisingFailureReceiver, storageFilter);
    }

    /**
     * When app goes off screen, unregister the Advertising failure Receiver to stop memory leaks.
     * (and because the app doesn't care if Advertising fails while the UI isn't active)
     */
    @Override
    public void onPause() {
        super.onPause();
        getActivity().unregisterReceiver(advertisingFailureReceiver);
    }

    /**
     * Returns Intent addressed to the {@code AdvertiserService} class.
     */
    private Intent getServiceIntent(Context c) {
        Intent i = new Intent(c, AdvertiserService.class);
        i.putExtra(ARG_PARAM_BLE_POWER_LEVEL, ble_power_level);
        i.putExtra(ARG_PARAM_BLE_INTERVAL, ble_interval);
        return i;
    }

    /**
     * Called when switch is toggled - starts or stops advertising.
     */
    @Override
    public void onClick(View v) {
        // Is the toggle on?
        boolean on = ((SwitchCompat) v).isChecked();

        if (on) {
            startAdvertising();
        } else {
            stopAdvertising();
        }
    }

    /**
     * Starts BLE Advertising by starting {@code AdvertiserService}.
     */
    private void startAdvertising() {
        Context c = getActivity();
        c.startService(getServiceIntent(c));
    }

    /**
     * Stops BLE Advertising by stopping {@code AdvertiserService}.
     */
    private void stopAdvertising() {
        Context c = getActivity();
        c.stopService(getServiceIntent(c));
        mSwitch.setChecked(false);
    }

}