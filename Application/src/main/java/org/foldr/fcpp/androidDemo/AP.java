package org.foldr.fcpp.androidDemo;

import static org.foldr.fcpp.androidDemo.Constants.LOG_BT_TAG;
import static org.foldr.fcpp.androidDemo.Constants.LOG_HTTP_TAG;
import static org.foldr.fcpp.androidDemo.Constants.LOG_TAG;

import android.app.Application;
import android.bluetooth.le.ScanResult;
import android.content.Context;
import android.content.SharedPreferences;
import android.location.LocationManager;
import android.util.Base64;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.preference.PreferenceManager;

import java.io.IOException;
import java.util.Deque;
import java.util.UUID;
import java.util.concurrent.ConcurrentLinkedDeque;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;
import java.util.prefs.PreferencesFactory;

import okhttp3.MediaType;
import okhttp3.OkHttpClient;
import okhttp3.Request;
import okhttp3.RequestBody;
import okhttp3.Response;

public class AP extends Application {

    public static int uid;

    /* Anyone in the app may read this to check if we're winding down.
        Mostly used by periodic updaters of the UI that shouldn't call
        into FCPP any more. You should set this before calling `fcpp_stop()`,
        maybe later we'll set this for you.
     */
    public static boolean is_stopping = false;
    public LocationManager locationManager;

    /* These native definitions are from ap-getters.cpp: */
    static native void fcpp_start(int uid, String experiment_name);
    public static native void fcpp_stop();

    public static native boolean get_bool(String attribute);
    public static native int get_int(String attribute);
    public static native double get_double(String attribute);
    public static native String get_string(String attribute);
    public static native void set_bool(String attribute, boolean value);
    public static native void set_int(String attribute, int value);
    public static native void set_string(String attribute, String value);
    public static native void set_double(String attribute, double value);

    public static native String get_storage();

    // DEPRECATED FROM HERE

    public static native String get_nbr_lags();
    public static native long get_round_count();
    public static native long get_max_msg_size();
    public static native boolean get_im_weak();
    public static native boolean get_some_weak();
    public static native int get_min_uid();
    public static native int get_degree();
    public static native float get_global_clock();
    public static native int get_hop_dist();

    public static native int get_diameter();
    public static native void set_diameter(int diam);
    public static native float get_retain_time();
    public static native void set_retain_time(float time);
    public static native float get_round_period();
    public static native void set_round_period(float time);

    // TO HERE

    /* Deque for received transmissions. Read from first, append to end. */
    static Deque<ScanResult> pending = new ConcurrentLinkedDeque<>();
    /* Support for BT to advertise most recent data */
    static Lock outgoing = new ReentrantLock();
    static Condition haveNewData = outgoing.newCondition();
    private static byte[] newData = null;

    public static OkHttpWrapper.JSONFormatter jsonhttpFormatter = null;

    public static class OkHttpWrapper {
        public interface JSONFormatter {
            String getJSONHTTPFormatter();
            String getHTTPUrl();
        }

        static OkHttpClient okHttpClient = new OkHttpClient();
        static final MediaType JSON = MediaType.get("application/json; charset=utf-8");

        static int failCounter = 0;

        static void httpLog() {
            /* Needs to be initialised from the actual activity. */
            if (jsonhttpFormatter == null) {
                return;
            }
            String jsonLogMsg = jsonhttpFormatter.getJSONHTTPFormatter();
            RequestBody body = RequestBody.create(jsonLogMsg, JSON);
            Request request = new Request.Builder()
                    .url(jsonhttpFormatter.getHTTPUrl())
                    .post(body)
                    .build();
            try {
                Response response = okHttpClient.newCall(request).execute();
                if (!response.isSuccessful()) {
                    Log.d(LOG_HTTP_TAG, jsonLogMsg);
                    Log.d(LOG_HTTP_TAG, response.message());
                }
                response.close();
            } catch (IOException e) {
                // Betting 0.05EUR that this will haunt me:
                if (failCounter < 50) {
                    failCounter++;
                    Log.d(LOG_HTTP_TAG, "oops:" + e.getMessage(), e);
                }
            }
        }
    }

    /* Call-back for C++ */
    public static byte[] getNextMsg() {
        /* Maybe we need to lock this method, not the structure?*/
        ScanResult scanResult = pending.pollFirst();
        if (scanResult == null) {
            return null;
        } else {
            byte[] data = scanResult.getScanRecord().getServiceData(Constants.Service_UUID);
            Log.d(LOG_BT_TAG, "BLE packet TO C++:" + data.length);
            // Log.d(LOG_TAG, BaseEncoding.base16().lowerCase().encode(data));
            // OkHttpWrapper.httpLog(data);
            return data;
        }
    }

    /* Call-back for C++ */
    public static void postMsg(byte[] data, int size){
//        byte[] fresh = new byte[size];
//        System.arraycopy(data, 0, fresh, 0, size);
//        Log.d(LOG_TAG, "Data size "+size+" from C :"+BaseEncoding.base16().lowerCase().encode(fresh));
        Log.d(LOG_TAG, "Packet size "+size+" from C++.");
        outgoing.lock();
        newData = data;
        haveNewData.signalAll();
        outgoing.unlock();
        // Good time to log our state this round:
        // TODO: needs probably a bit more sync'ed async!
        AP.OkHttpWrapper.httpLog();
    }

    /* For advertising: */
     byte[] getMsg() {
        try {
            outgoing.lock();
            if (newData == null) {
                Log.d(LOG_TAG, "Waiting for data from fcpp...");
                haveNewData.await();
                Log.d(LOG_TAG, "Received data from fcpp!");
            }
            if (newData == null) {
                Log.d(LOG_TAG, "Received null data!");
            } else {
                Log.d(LOG_TAG, "Received data!");
                // Log.d(LOG_TAG, BaseEncoding.base16().lowerCase().encode(newData));
            }
            return newData; /* VS: let's see if this works nicely together with the finally below. */
        } catch (InterruptedException e) {
            Log.i(LOG_TAG, "Did not expect exception: "+e.getMessage());
        } finally {
            newData = null;
            outgoing.unlock();
        }
        return null;
    }

    private int setUID() {
        /* Check if we need to initialise the persistent device id etc. */
        // VS: Do not use this.getPreferences() here!
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
        boolean prefs_bt_legacy = prefs.getBoolean(getString(R.string.prefs_legacy), false);

        String uid = prefs.getString(getString(R.string.prefs_uid), null);
        if (uid == null || uid.isEmpty()) {
            uid = UUID.randomUUID().toString();
            Log.i(LOG_TAG, "Setting UID: "+uid);
            // We always write the values here on purpose:
            prefs.edit().putString(getString(R.string.prefs_uid), uid)
                    .putBoolean(getString(R.string.prefs_legacy), prefs_bt_legacy)
                    .apply();
        } else {
            Log.i(LOG_TAG, "Have UID: "+uid+"/"+uid.hashCode());
        }
        // Review: HACK to get an int.
        // Reconstruct FCPP's treatment here.
        this.uid = Short.toUnsignedInt((short) (uid.hashCode()));
        return this.uid;
    }

    @Override
    public void onCreate() {
        super.onCreate();

        // Set up location provider
        locationManager = (LocationManager) this.getSystemService(Context.LOCATION_SERVICE);
        if (locationManager == null) {
            Log.e(LOG_TAG, "No location service :-(");
        }


    }

    public void fcpp_start(String experiment) {
        int uid = setUID();
        Log.i(LOG_TAG, "fcpp_start: " + uid);
        fcpp_start(uid, experiment);

        // TODO: proper initial values for fcpp?
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
        String prefs_diameter = prefs.getString(getString(R.string.prefs_fcpp_diameter), "1");
        String prefs_period = prefs.getString(getString(R.string.prefs_fcpp_period), "1.0");
        String prefs_retain = prefs.getString(getString(R.string.prefs_fcpp_retain), "1.0");
        int the_diameter;
        try {
             the_diameter = Integer.parseInt(prefs_diameter);
        } catch (NumberFormatException e) {
            the_diameter = 1;
            Log.d(LOG_TAG, "FCPP data error", e);
        }
        float the_period;
        try {
            the_period = Float.parseFloat(prefs_period);
        } catch (NumberFormatException e) {
            the_period = 1;
            Log.d(LOG_TAG, "FCPP data error", e);
        }
        float the_retain;
        try {
            the_retain = Float.parseFloat(prefs_retain);
        } catch (NumberFormatException e) {
            the_retain = 1;
            Log.d(LOG_TAG, "FCPP data error", e);
        }
        Log.i(LOG_TAG, "FCPP params (orig): "+prefs_diameter + "," + prefs_period+","+ prefs_retain);
        Log.i(LOG_TAG, "FCPP params (derived): "+the_diameter + "," + the_period+","+ the_retain);
        set_diameter(the_diameter);
        set_round_period(the_period);
        set_retain_time(the_retain);
    }

    static {
        System.loadLibrary("ap");
    }

}