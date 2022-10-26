package org.foldr.fcpp.androidDemo;

import static org.foldr.fcpp.androidDemo.Constants.LOG_TAG;

import android.app.Application;
import android.bluetooth.le.ScanResult;
import android.content.SharedPreferences;
import android.util.Base64;
import android.util.Log;

import androidx.preference.PreferenceManager;

import java.io.IOException;
import java.util.Deque;
import java.util.UUID;
import java.util.concurrent.ConcurrentLinkedDeque;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

import okhttp3.MediaType;
import okhttp3.OkHttpClient;
import okhttp3.Request;
import okhttp3.RequestBody;
import okhttp3.Response;

public class AP extends Application {

    static private String uid;

    /* These native definitions are from ap-getters.cpp: */
    static native void fcpp_start(int uid, int diam, int per, int end);
    public static native void fcpp_stop();
    public static native long getMaxMsgSize();
    public static native long getRound();
    public static native boolean get_im_weak();
    public static native boolean get_some_weak();
    public static native int get_degree();
    public static native String get_storage();

    /* Deque for received transmissions. Read from first, append to end. */
    static Deque<ScanResult> pending = new ConcurrentLinkedDeque<>();
    /* Support for BT to advertise most recent data */
    static Lock outgoing = new ReentrantLock();
    static Condition haveNewData = outgoing.newCondition();
    private static byte[] newData = null;

    static final MediaType JSON = MediaType.get("application/json; charset=utf-8");

    static class OkHttpWrapper {
        static OkHttpClient okHttpClient = new OkHttpClient();
        // TODO: configurable
        static String url = "http://new.foldr.org:3000/entry";

        private static void httpLog(byte[] data) {
            // TODO: customize, obviously.
            String json = "{ \"uid\": "+ Integer.toString(uid.hashCode())+", \"entry\": \""+ Base64.encodeToString(data, Base64.NO_WRAP)+"\"}";
            RequestBody body = RequestBody.create(json, JSON);
            Request request = new Request.Builder()
                    .url(url)
                    .post(body)
                    .build();
            try {
                Response response = okHttpClient.newCall(request).execute();
                if (!response.isSuccessful()) {
                    Log.d(LOG_TAG, json);
                    Log.d(LOG_TAG, response.body().string());
                }
                response.close();
            } catch (IOException e) {
                Log.d(LOG_TAG,"oops:"+e.getMessage(), e);
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
            Log.d(LOG_TAG, "BLE packet TO C++:" + data.length);
            OkHttpWrapper.httpLog(data);
            return data;
        }
    }

    /* Call-back for C++ */
    public static void postMsg(byte[] data, int size){
//        byte[] fresh = new byte[size];
//        System.arraycopy(data, 0, fresh, 0, size);
//        Log.d(LOG_TAG, "Data size "+size+" from C :"+BaseEncoding.base16().lowerCase().encode(fresh));
        Log.d(LOG_TAG, "Packet size "+size+" from C.");
        outgoing.lock();
        newData = data;
        haveNewData.signalAll();
        outgoing.unlock();
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
            Log.d(LOG_TAG, "Received data!");
            // Log.d(LOG_TAG, BaseEncoding.base16().lowerCase().encode(newData));
            return newData; /* VS: let's see if this works nicely together with the finally below. */
        } catch (InterruptedException e) {
            Log.i(LOG_TAG, "Did not expect exception: "+e.getMessage());
        } finally {
            newData = null;
            outgoing.unlock();
        };
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
            Log.i(LOG_TAG, "Have UID: "+uid);
        };
        // Review: HACK to get an int.
        this.uid = uid;
        return uid.hashCode();
    }

    @Override
    public void onCreate() {
        super.onCreate();
        // TODO: proper initial values for fcpp?
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
        String prefs_diameter = prefs.getString(getString(R.string.prefs_fcpp_diameter), "1");
        String prefs_per = prefs.getString(getString(R.string.prefs_fcpp_per), "1");
        Log.i(LOG_TAG, "fccp_start: " + prefs_diameter + "," + prefs_per);
        fcpp_start(setUID(), Integer.valueOf(prefs_diameter), Integer.valueOf(prefs_per), 65000 /* XXX */);
    }

    static {
        System.loadLibrary("ap-libs");
    }

}