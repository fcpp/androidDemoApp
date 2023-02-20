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

import static org.foldr.fcpp.androidDemo.Constants.LOG_TAG;

import android.Manifest;
import android.annotation.SuppressLint;
import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothManager;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.graphics.Color;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.os.Bundle;
import android.provider.Settings;
import android.util.Log;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.Toolbar;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.core.app.ActivityCompat;
import androidx.fragment.app.FragmentActivity;
import androidx.fragment.app.FragmentTransaction;
import androidx.preference.Preference;
import androidx.preference.PreferenceFragmentCompat;

/**
 * Setup display fragments and ensure the device supports Bluetooth.
 */
public class MainActivity extends FragmentActivity
        implements ActivityCompat.OnRequestPermissionsResultCallback,
        AP.OkHttpWrapper.JSONFormatter {

    private BluetoothAdapter mBluetoothAdapter;
    private Toolbar mToolbar;
    AP application;
    private LocationListener locationListener;

    @SuppressLint("MissingPermission")
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        application = (AP) getApplication();
        /* Note that FCPP is already running by the time we install the logger. */
        application.jsonhttpFormatter = this;
        application.fcpp_start("vulnerability_detection"); // untested after refactoring :-)
        /* TODO:
            init FCPP params here.
         */
        locationListener = new LocationListener() {
            @Override
            public void onLocationChanged(@NonNull Location location) {
                // Call into FCPP:
                AP.set_double("position_latitude", location.getLatitude());
                AP.set_double("position_longitude", location.getLongitude());
                AP.set_double("position_accuracy", location.getAccuracy());
            }
        };

        // TODO: Should be dead code?
        if (!application.locationManager.isProviderEnabled(LocationManager.GPS_PROVIDER)) {
            Log.d(LOG_TAG, "Should not have happened?");
            Intent settingsIntent = new Intent(Settings.ACTION_LOCATION_SOURCE_SETTINGS);
            startActivity(settingsIntent);
        }
        application.locationManager.requestLocationUpdates(LocationManager.GPS_PROVIDER, 10000, 10,
                locationListener);
        setContentView(R.layout.activity_main);
        setTitle(R.string.activity_main_title);
//        mToolbar = findViewById(R.id.AdvertiserToolbar);
//        assert mToolbar != null;

        if (savedInstanceState == null) {
            if (checkPermissions(this, this)) {
                doAllTheThings();
            }
        }
    }

    @SuppressLint("MissingPermission")
    private void doAllTheThings() {
            mBluetoothAdapter = ((BluetoothManager) getSystemService(Context.BLUETOOTH_SERVICE))
                    .getAdapter();

            // Is Bluetooth supported on this device?
            if (mBluetoothAdapter != null) {

                // Is Bluetooth turned on?
                if (mBluetoothAdapter.isEnabled()) {

                    // Are Bluetooth Advertisements supported on this device?
                    if (mBluetoothAdapter.isMultipleAdvertisementSupported()) {

                        if (!mBluetoothAdapter.isLe2MPhySupported()) {
                            showErrorText(R.string.ble_2mphy_failed);
                            return;
                        }
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

    public static boolean checkPermissions(Activity activity, Context context){
        int PERMISSION_ALL = 1;
        String[] PERMISSIONS = {
                Manifest.permission.ACCESS_FINE_LOCATION,
                Manifest.permission.BLUETOOTH,
                Manifest.permission.BLUETOOTH_ADMIN,
                Manifest.permission.BLUETOOTH_PRIVILEGED
                , Manifest.permission.BLUETOOTH_SCAN
                , Manifest.permission.BLUETOOTH_CONNECT
                , Manifest.permission.BLUETOOTH_ADVERTISE
        };

        if(!hasPermissions(context, PERMISSIONS)){
            ActivityCompat.requestPermissions( activity, PERMISSIONS, PERMISSION_ALL);
            return false;
        } else {
            return true;
        }
    }

    @SuppressLint("MissingSuperCall")
    @Override
    public void onRequestPermissionsResult(int requestCode,
                                           @NonNull String[] permissions,
                                           @NonNull int[] grantResults) {
        // We fail HARD later if not granted; we don't care.
        doAllTheThings();
    }
    public static boolean hasPermissions(Context context, String... permissions) {
        if (context != null && permissions != null) {
            for (String permission : permissions) {
                if (ActivityCompat.checkSelfPermission(context, permission) != PackageManager.PERMISSION_GRANTED) {
                    return false;
                }
            }
        }
        return true;
    }

    @Override
    public void onDestroy() {
        Log.d(LOG_TAG, "Shutting down.");
        if (locationListener != null)
            application.locationManager.removeUpdates(locationListener);
        // Stop the advertiser. Note that there's also the AdvertiserService.running static flag.
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

    public static class BLEPreferencesFragments extends PreferenceFragmentCompat {
        @Override
        public void onCreatePreferences(@Nullable Bundle savedInstanceState, @Nullable String rootKey) {
            setPreferencesFromResource(R.xml.preferences, rootKey);
            {
                Preference p;
                p = findPreference(getString(R.string.prefs_fcpp_diameter));
                p.setOnPreferenceChangeListener(new Preference.OnPreferenceChangeListener() {
                    @Override
                    public boolean onPreferenceChange(@NonNull Preference preference, Object newValue) {
                        // TODO: error handling
                        int v = Integer.valueOf((String)newValue);
                        AP.set_diameter(v);
                        return true;
                    }
                });
                p = findPreference(getString(R.string.prefs_fcpp_period));
                p.setOnPreferenceChangeListener(new Preference.OnPreferenceChangeListener() {
                    @Override
                    public boolean onPreferenceChange(@NonNull Preference preference, Object newValue) {
                        // TODO: error handling
                        float v = Float.parseFloat(((String)newValue));
                        AP.set_round_period(v);
                        return true;
                    }
                });
                p = findPreference(getString(R.string.prefs_fcpp_retain));
                p.setOnPreferenceChangeListener(new Preference.OnPreferenceChangeListener() {
                    @Override
                    public boolean onPreferenceChange(@NonNull Preference preference, Object newValue) {
                        // TODO: error handling
                        float v = Float.parseFloat(((String)newValue));
                        AP.set_retain_time(v);
                        return true;
                    }
                });
            }
            { // Patch uid into preferences dialog:
                Preference uid_prefs = findPreference(getString(R.string.prefs_uid));
                uid_prefs.setOnPreferenceChangeListener(new Preference.OnPreferenceChangeListener() {
                    @Override
                    public boolean onPreferenceChange(@NonNull Preference preference, Object newValue) {
                        Toast.makeText(getContext(), "Please restart the app now!", Toast.LENGTH_LONG).show();
                        return true;
                    }
                });
                uid_prefs.setTitle(uid_prefs.getTitle() + ": "+ AP.uid);
            }
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
        transaction.replace(R.id.advertiser_fragment_container, advertiserFragment);

        // TODO: hook up to menu or something instead.
        PreferenceFragmentCompat preferencesFragment = new BLEPreferencesFragments();
        transaction.replace(R.id.preferences_fragment_container, preferencesFragment);

        transaction.commit();
    }

    private void showErrorText(int messageId) {
        showErrorText(this, messageId);
    }

    public static void showErrorText(Activity me, int messageId) {
        Log.e(LOG_TAG, me.getString(messageId));
        TextView view = me.findViewById(R.id.error_textview);
        view.setText(me.getString(messageId));
        view.setBackgroundColor(Color.RED);
    }

    @NonNull
    public String getJSONHTTPFormatter() {
        String json = String.format("{ \"uid\":%d, \"degree\":%d, \"round_count\":%d,"
                        +"\"nbr_lags\":\"%s\",\"hop_dist\":%d,\"global_clock\":%f,"
                        +"\"im_weak\":%b,\"some_weak\":%b, \"min_uid\":%d"
                        +"}"
                ,AP.uid,AP.get_degree(),AP.get_round_count(),AP.get_nbr_lags(),AP.get_hop_dist(),AP.get_global_clock()
                ,AP.get_im_weak(), AP.get_some_weak(), AP.get_min_uid());
        return json;
    }

    @Override
    public String getHTTPUrl() {
        return "https://www.foldr.org/fcpp/entry";
    }
}