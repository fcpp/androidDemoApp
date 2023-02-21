package org.foldr.fcpp.androidDemo;

import static org.foldr.fcpp.androidDemo.Constants.LOG_TAG;

import android.annotation.SuppressLint;
import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothManager;
import android.bluetooth.le.AdvertiseCallback;
import android.bluetooth.le.AdvertiseData;
import android.bluetooth.le.AdvertisingSet;
import android.bluetooth.le.AdvertisingSetCallback;
import android.bluetooth.le.AdvertisingSetParameters;
import android.bluetooth.le.BluetoothLeAdvertiser;
import android.content.Context;
import android.content.Intent;
import android.os.Build;
import android.os.Handler;
import android.os.IBinder;
import android.util.Log;
import android.widget.Toast;

import androidx.core.app.NotificationCompat;
import androidx.preference.PreferenceManager;

import com.google.common.io.BaseEncoding;

import java.util.concurrent.TimeUnit;

/**
 * Manages BLE Advertising independent of the main app.
 * If the app goes off screen (or gets killed completely) advertising can continue because this
 * Service is maintaining the necessary Callback in memory.
 */
public class AdvertiserService extends Service {

    private static final String TAG = AdvertiserService.class.getSimpleName();

    private static final int FOREGROUND_NOTIFICATION_ID = 1;

    /**
     * A global variable to let AdvertiserFragment check if the Service is running without needing
     * to start or bind to it.
     * This is the best practice method as defined here:
     * https://groups.google.com/forum/#!topic/android-developers/jEvXMWgbgzE
     */
    public static boolean running = false;

    public static final String ADVERTISING_FAILED =
        "org.foldr.fcpp.androidDemo.advertising_failed";

    public static final String ADVERTISING_FAILED_EXTRA_CODE = "failureCode";
    public static final String ADVERTISING_FAILED_EXTRA_MSG = "failureMsg";

    public static final String ADVERTISING_STORAGE =
            "org.foldr.fcpp.androidDemo.advertising_storage";
    public static final String ADVERTISING_STORAGE_EXTRA_CODE = "storageCode";

    public static final int ADVERTISING_TIMED_OUT = 6;

    private BluetoothLeAdvertiser mBluetoothLeAdvertiser;

    private AdvertisingSetCallback mAdvertiseCallback;

    private Handler mHandler;

    private Runnable timeoutRunnable;

    /**
     * Length of time to allow advertising before automatically shutting off. (10 minutes)
     */
    private long TIMEOUT = TimeUnit.MILLISECONDS.convert(10, TimeUnit.MINUTES);

    private AP mAp;
    private AdvertisingSetParameters parameters;
    private boolean ble_toast_only_once = true;

    @Override
    public void onCreate() {
        Log.d(TAG, "Service: Starting Advertising Service");
        running = true;
        mAp = (AP) getApplication();
        initialize();

        new Thread(new Runnable() {
            @Override
            public void run() {
                while (running) {
                    startAdvertising();
                    /* TODO: stupid here since only running when advertising.
                     * OTOH, it should be running continuously anyways. */
                    sendStorageIntent();
                    try {
                        Thread.sleep(1000);
                    } catch (InterruptedException e) {
                        // NOP
                    }
                }
            }
        }).start();
        // no timeout for now.
        // setTimeout();
        super.onCreate();
    }

    @Override
    public void onDestroy() {
        /**
         * Note that onDestroy is not guaranteed to be called quickly or at all. Services exist at
         * the whim of the system, and onDestroy can be delayed or skipped entirely if memory need
         * is critical.
         */
        running = false;
        stopAdvertising();
        // mHandler.removeCallbacks(timeoutRunnable);
        // stopForeground(true);
        super.onDestroy();
    }

    /**
     * Required for extending service, but this will be a Started Service only, so no need for
     * binding.
     */
    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    /**
     * Get references to system Bluetooth objects if we don't have them already.
     */
    private void initialize() {
        if (mBluetoothLeAdvertiser == null) {
            BluetoothManager mBluetoothManager = (BluetoothManager) getSystemService(Context.BLUETOOTH_SERVICE);
            if (mBluetoothManager != null) {
                BluetoothAdapter mBluetoothAdapter = mBluetoothManager.getAdapter();
                if (mBluetoothAdapter != null) {
                    mBluetoothLeAdvertiser = mBluetoothAdapter.getBluetoothLeAdvertiser();
                } else {
                    Toast.makeText(this, getString(R.string.bt_null), Toast.LENGTH_LONG).show();
                }
            } else {
                Toast.makeText(this, getString(R.string.bt_null), Toast.LENGTH_LONG).show();
            }
        }
        parameters = getAdvertisingSetParameters();
    }

    /**
     * Starts a delayed Runnable that will cause the BLE Advertising to timeout and stop after a
     * set amount of time.
     */
    private void setTimeout(){
        mHandler = new Handler();
        timeoutRunnable = () -> {
            Log.d(TAG, "AdvertiserService has reached timeout of "+TIMEOUT+" milliseconds, stopping advertising.");
            sendFailureIntent(ADVERTISING_TIMED_OUT);
            stopSelf();
        };
        mHandler.postDelayed(timeoutRunnable, TIMEOUT);
    }

    /**
     * Starts BLE Advertising.
     */
    @SuppressLint("MissingPermission")
    private void startAdvertising() {
        // goForeground();

        AdvertiseData data = buildAdvertiseData();
        if (mAdvertiseCallback != null) {
            // TODO: There's some way to update the data we're advertising.
            stopAdvertising();
        }
        if (mAdvertiseCallback == null) {
                // Double-check that we get the same data as set in AP. Uncomment for safety-belt.
                // assert PreferenceManager.getDefaultSharedPreferences(this).contains(getString(R.string.prefs_legacy));

                mAdvertiseCallback = new AdvertisingSetCallback() {
                    @Override
                    public void onAdvertisingSetStarted(AdvertisingSet advertisingSet, int txPower, int status) {
                        if (status != AdvertisingSetCallback.ADVERTISE_SUCCESS) {
                            Log.e(LOG_TAG, "onAdvertisingSetStarted(): txPower:" + txPower + " , status: "
                                    + status);
                            sendFailureIntent(status);
                        }
                    }

                    @Override
                    public void onAdvertisingSetStopped(AdvertisingSet advertisingSet) {
//                        Log.i(LOG_TAG, "onAdvertisingSetStopped():");
                    }
                };
                if (mBluetoothLeAdvertiser != null) {
                    try {
                        /* If we're running e.g. in the emulator, this will trip.
                         * Handle this a bit more gracefully. I'm still sceptical of interspersing
                         * the code with special paths just for emulation...
                         */
                        mBluetoothLeAdvertiser.startAdvertisingSet(parameters, data, null, null, null, mAdvertiseCallback);
                    } catch (IllegalArgumentException e) {
                        if (ble_toast_only_once) {
                            ble_toast_only_once = false;
                            Log.e(LOG_TAG, "Advertising failed.", e);
                            // not the greatest error code but it'll do:
                            sendFailureIntent(AdvertiseCallback.ADVERTISE_FAILED_INTERNAL_ERROR);
                        }
                    }
                }
        }
    }

    private AdvertisingSetParameters getAdvertisingSetParameters() {
        AdvertisingSetParameters parameters = (new AdvertisingSetParameters.Builder())
                .setLegacyMode(PreferenceManager.getDefaultSharedPreferences(this)
                        .getBoolean(getString(R.string.prefs_legacy), false))
                .setConnectable(false)
                // TODO: Tunables here. Note that we're currently only calling this once on startup.
                .setInterval(AdvertisingSetParameters.INTERVAL_HIGH)
                .setTxPowerLevel(AdvertisingSetParameters.TX_POWER_MEDIUM)
                .setPrimaryPhy(BluetoothDevice.PHY_LE_1M)
                // XXX #18 .setSecondaryPhy(BluetoothDevice.PHY_LE_2M)
                .build();
        return parameters;
    }

    /**
     * Move service to the foreground, to avoid execution limits on background processes.
     *
     * Callers should call stopForeground(true) when background work is complete.
     */
    private void goForeground() {
        Intent notificationIntent = new Intent(this, MainActivity.class);
        PendingIntent pendingIntent = PendingIntent.getActivity(this, 0,
            notificationIntent, 0);
        /* Lifted literally from:
           https://developer.android.com/training/notify-user/build-notification */
        createNotificationChannel("CHAN_VS");
        Notification n = new NotificationCompat.Builder(this, "CHAN_VS")
            .setContentTitle("Advertising device via Bluetooth")
            .setContentText("This device is discoverable to others nearby.")
            .setSmallIcon(R.drawable.ic_launcher)
            .setContentIntent(pendingIntent)
            .build();
        startForeground(FOREGROUND_NOTIFICATION_ID, n);
    }

    private void createNotificationChannel(String channelId) {
        // Create the NotificationChannel, but only on API 26+ because
        // the NotificationChannel class is new and not in the support library
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            CharSequence name = getString(R.string.channel_name);
            String description = getString(R.string.channel_description);
            int importance = NotificationManager.IMPORTANCE_DEFAULT;
            NotificationChannel channel = new NotificationChannel(channelId, name, importance);
            channel.setDescription(description);
            // Register the channel with the system; you can't change the importance
            // or other notification behaviors after this
            NotificationManager notificationManager = getSystemService(NotificationManager.class);
            notificationManager.createNotificationChannel(channel);
        }
    }

    /**
     * Stops BLE Advertising.
     */
    @SuppressLint("MissingPermission")
    private void stopAdvertising() {
        Log.d(TAG, "Service: Stopping Advertising");
        if (mAdvertiseCallback != null) {
            mBluetoothLeAdvertiser.stopAdvertisingSet(mAdvertiseCallback);
            mAdvertiseCallback = null;
        }
    }

    /**
     * Returns an AdvertiseData object which includes the Service UUID and Device Name.
     */
    private AdvertiseData buildAdvertiseData() {

        /**
         * Note: There is a strict limit of 31 Bytes on packets sent over BLE Advertisements.
         *  This includes everything put into AdvertiseData including UUIDs, device info, &
         *  arbitrary service or manufacturer data.
         *  Attempting to send packets over this limit will result in a failure with error code
         *  AdvertiseCallback.ADVERTISE_FAILED_DATA_TOO_LARGE. Catch this error in the
         *  onStartFailure() method of an AdvertiseCallback implementation.
         */

        AdvertiseData.Builder dataBuilder = new AdvertiseData.Builder();
        dataBuilder.addServiceUuid(Constants.Service_UUID);
        dataBuilder.setIncludeDeviceName(true);
        byte[] data = mAp.getMsg(); /* Blocking! */
        // Log.d(LOG_TAG, BaseEncoding.base16().lowerCase().encode(data));
        assert data != null;
        dataBuilder.addServiceData(Constants.Service_UUID, data);

        return dataBuilder.build();
    }

    /**
     * Builds and sends a broadcast intent indicating Advertising has failed. Includes the error
     * code as an extra. This is intended to be picked up by the {@code AdvertiserFragment}.
     */
    private void sendFailureIntent(int errorCode) {
        sendFailureIntentMsg(errorCode, null);
    }
    private void sendFailureIntentMsg(int errorCode, String msg){
        Intent failureIntent = new Intent();
        failureIntent.setAction(ADVERTISING_FAILED);
        failureIntent.putExtra(ADVERTISING_FAILED_EXTRA_CODE, errorCode);
        if (msg != null) {
            failureIntent.putExtra(ADVERTISING_FAILED_EXTRA_MSG, msg);
        }
        sendBroadcast(failureIntent);
    }

    /**
     * Logs AP node contents. We don't necessarily have a UI, so we broadcast and hope
     * that there's a logger :-)
     */
    private void sendStorageIntent(){
        Intent failureIntent = new Intent();
        failureIntent.setAction(ADVERTISING_STORAGE);
        failureIntent.putExtra(ADVERTISING_STORAGE_EXTRA_CODE, AP.get_storage());
        sendBroadcast(failureIntent);
    }
}
