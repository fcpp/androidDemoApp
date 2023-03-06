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

package org.foldr.fcpp.androidDemo.friendfinding1;

import static org.foldr.fcpp.androidDemo.AdvertiserFragment.*;
import static org.foldr.fcpp.androidDemo.BLEParameterFragment.*;
import static org.foldr.fcpp.androidDemo.Constants.LOG_TAG;
import static org.foldr.fcpp.androidDemo.evacuation1.EvacuationFragment.*;
import static org.foldr.fcpp.androidDemo.friendfinding1.FriendFindingParameters.ARG_PARAM_USE_LAGS;

import android.annotation.SuppressLint;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothManager;
import android.bluetooth.le.AdvertisingSetParameters;
import android.bluetooth.le.ScanSettings;
import android.content.Context;
import android.content.Intent;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.os.Bundle;
import android.provider.Settings;
import android.util.Log;
import android.widget.Toast;
import android.widget.Toolbar;

import androidx.annotation.NonNull;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentActivity;
import androidx.fragment.app.FragmentTransaction;

import org.foldr.fcpp.androidDemo.AP;
import org.foldr.fcpp.androidDemo.AdvertiserFragment;
import org.foldr.fcpp.androidDemo.AdvertiserService;
import org.foldr.fcpp.androidDemo.Constants;
import org.foldr.fcpp.androidDemo.MainActivity;
import org.foldr.fcpp.androidDemo.R;
import org.foldr.fcpp.androidDemo.ScannerFragment;

/**
 * Setup display fragments and ensure the device supports Bluetooth.
 */
public class FriendFindingActivity extends FragmentActivity
        implements AP.OkHttpWrapper.JSONFormatter {

    private BluetoothAdapter mBluetoothAdapter;
    private Toolbar mToolbar;
    AP application;
    private LocationListener locationListener;
    private FriendFindingFragment frag;
    private int ble_power_level;
    private int ble_scan_mode;

    @SuppressLint("MissingPermission")
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        float retain = getIntent().getFloatExtra(ARG_PARAM_RETAIN, -1);
        int diameter = getIntent().getIntExtra(ARG_PARAM_DIAMETER, -1);
        ble_power_level = getIntent().getIntExtra(ARG_PARAM_BLE_POWER_LEVEL, AdvertisingSetParameters.TX_POWER_MEDIUM);
        ble_scan_mode = getIntent().getIntExtra(ARG_PARAM_BLE_SCAN_MODE, ScanSettings.SCAN_MODE_LOW_LATENCY);
        boolean use_lags = getIntent().getBooleanExtra(ARG_PARAM_USE_LAGS, true);

        // The options are for FCPP:
        frag = FriendFindingFragment.newInstance(use_lags);
        application = (AP) getApplication();
        /* Note that FCPP is already running by the time we install the logger. */
        application.jsonhttpFormatter = this;
        application.fcpp_start("friend_finding");
        AP.set_int(ARG_PARAM_DIAMETER, diameter);
        AP.set_double(ARG_PARAM_RETAIN, retain);
        AP.set_double(ARG_PARAM_ROUND_PERIOD, getIntent().getFloatExtra(ARG_PARAM_ROUND_PERIOD,1));

        locationListener = new LocationListener() {
            @Override
            public void onLocationChanged(@NonNull Location location) {
                // Call into FCPP:
                AP.set_double("position_latitude", location.getLatitude());
                AP.set_double("position_longitude", location.getLongitude());
                AP.set_double("position_accuracy", location.getAccuracy());
            }
        };
        setContentView(R.layout.activity_main);
        setTitle(R.string.activity_main_title);

        if (savedInstanceState == null) {
            boolean perms = MainActivity.checkPermissions(this, this);
            if (perms) {
                doAllTheThings();
            } // else we're waiting for the callback.
        }
    }

    @SuppressLint("MissingSuperCall")
    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions,
                                           @NonNull int[] grantResults) {
        // We fail HARD later if not granted; we don't care.
        doAllTheThings();
    }
    @SuppressLint("MissingPermission")
    private void doAllTheThings() {
            // TODO: Should be dead code?
            if (!application.locationManager.isProviderEnabled(LocationManager.GPS_PROVIDER)) {
                Intent settingsIntent = new Intent(Settings.ACTION_LOCATION_SOURCE_SETTINGS);
                startActivityForResult(settingsIntent, Constants.REQUEST_AFL);
                Log.d(LOG_TAG, "there");
                return;
            }

            application.locationManager.requestLocationUpdates(LocationManager.GPS_PROVIDER, 10000, 10,
                    locationListener);

            // TODO: Probably all this could be checked in the Parameter screen already?
            mBluetoothAdapter = ((BluetoothManager) getSystemService(Context.BLUETOOTH_SERVICE))
                    .getAdapter();

            // Is Bluetooth supported on this device?
            if (mBluetoothAdapter != null) {

                // Is Bluetooth turned on?
                if (mBluetoothAdapter.isEnabled()) {

                    // Are Bluetooth Advertisements supported on this device?
                    if (mBluetoothAdapter.isMultipleAdvertisementSupported()) {

//  #18                      if (!mBluetoothAdapter.isLe2MPhySupported()) {
//                            showErrorText(R.string.ble_2mphy_failed);
//                            return;
//                        }
                        if (!mBluetoothAdapter.isLeExtendedAdvertisingSupported()) {
                            showErrorText(R.string.ble_extended_failed);
                            return;
                        } else {
                            Log.i(LOG_TAG, "LE Extended Advertising supported.");
                        }
                        int leMaximumAdvertisingDataLength = mBluetoothAdapter.getLeMaximumAdvertisingDataLength();
                        Log.i(LOG_TAG, "leMaximumAdvertisingDataLength="+leMaximumAdvertisingDataLength);
                        if (leMaximumAdvertisingDataLength <= 31) {
                            // VS not sure if this can actually happen.
                            showErrorText(R.string.ble_advertise_too_small);
                            return;
                        }
                        setupFragments();

                    } else {

                        // Bluetooth Advertisements are not supported.
                        showErrorText(R.string.bt_ads_not_supported);
                        setupFragments();
                    }
                } else {

                    // Prompt user to turn on Bluetooth (logic continues in onActivityResult()).
                    Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
                    startActivityForResult(enableBtIntent, Constants.REQUEST_ENABLE_BT);
                }
            } else {

                // Bluetooth is not supported.
                showErrorText(R.string.bt_not_supported);
            }
        }

    @Override
    public void onDestroy() {
        Log.d(LOG_TAG, "Shutting down.");
        if (locationListener != null)
            application.locationManager.removeUpdates(locationListener);
        // Stop the advertiser. Note that there's also the AdvertiserService.running static flag.
        AP.is_stopping = true;
        stopService(new Intent(this, AdvertiserService.class));
        AP.fcpp_stop();
        super.onDestroy();
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        switch (requestCode) {
            // TODO: Review -- it looks like there's a path where we don't call
            // `setupFragments()` after the user (only) enable Fine Locations?
            case Constants.REQUEST_ENABLE_BT:

                if (resultCode == RESULT_OK) {

                    // Bluetooth is now Enabled, are Bluetooth Advertisements supported on
                    // this device?
                    if (mBluetoothAdapter.isMultipleAdvertisementSupported()) {

                        // Everything is supported and enabled, load the fragments.
                        setupFragments();

                    } else {

                        // Bluetooth Advertisements are not supported.
                        showErrorText(R.string.bt_ads_not_supported);
                        setupFragments();
                    }
                } else {

                    // User declined to enable Bluetooth, exit the app.
                    Toast.makeText(this, R.string.bt_not_enabled_leaving,
                            Toast.LENGTH_SHORT).show();
                    finish();
                }

            case Constants.REQUEST_AFL:

                if (resultCode != RESULT_OK) {
                    showErrorText(R.string.afl_failed);
                }

            default:
                super.onActivityResult(requestCode, resultCode, data);
        }
    }

    /**
     * Note that we have two entry-points. Currently we're using an extra one when the user
     * has to enable BT through the permissions dialog.
     */
    private void setupFragments() {

        FragmentTransaction transaction = getSupportFragmentManager().beginTransaction();

        ScannerFragment scannerFragment = new ScannerFragment();
        // Fragments can't access system services directly, so pass it the BluetoothAdapter
        scannerFragment.setBluetoothAdapter(mBluetoothAdapter);
        transaction.replace(R.id.scanner_fragment_container, scannerFragment);

        AdvertiserFragment advertiserFragment = new AdvertiserFragment();
        Bundle args = new Bundle();
        args.putBoolean(ARG_BROADCAST_ON_FIRST_BOOT, true);
        args.putBoolean(ARG_DISABLE_BROADCAST_SWITCH, true);
        args.putInt(ARG_PARAM_BLE_POWER_LEVEL, ble_power_level);
        args.putInt(ARG_PARAM_BLE_SCAN_MODE, ble_scan_mode);
        advertiserFragment.setArguments(args);
        transaction.replace(R.id.advertiser_fragment_container, advertiserFragment);
        transaction.replace(R.id.preferences_fragment_container, getFCPPScenarioFragmet());

        transaction.commit();
    }

    private Fragment getFCPPScenarioFragmet() {
        return frag;
    }

    private void showErrorText(int messageId) {
        MainActivity.showErrorText(this, messageId);
    }

    public String getJSONHTTPFormatter() {
        return AP.get_storage();
    }

    @Override
    public String getHTTPUrl() {
        return "https://www.foldr.org/fcpp/friendfinding";
    }
}