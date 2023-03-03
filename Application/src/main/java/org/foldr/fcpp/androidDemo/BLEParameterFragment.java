package org.foldr.fcpp.androidDemo;

import static org.foldr.fcpp.androidDemo.Constants.LOG_BT_TAG;

import android.bluetooth.le.AdvertisingSetParameters;
import android.content.Intent;
import android.os.Bundle;

import androidx.fragment.app.Fragment;

import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.FrameLayout;

import com.google.android.material.slider.Slider;

public class BLEParameterFragment extends Fragment {

    public static final String ARG_PARAM_BLE_POWER_LEVEL = "BLE_POWER_LEVEL";

    public BLEParameterFragment() {
        // Required empty public constructor
    }

    public static void setExtraBLEPowerLevel(Intent i, float power) {
        int the_power = AdvertisingSetParameters.TX_POWER_MEDIUM;
        switch (Math.round(power)) {
            case 0:
                the_power = AdvertisingSetParameters.TX_POWER_ULTRA_LOW;
                break;
            case 1:
                the_power = AdvertisingSetParameters.TX_POWER_LOW;
                break;
            case 2:
                the_power = AdvertisingSetParameters.TX_POWER_MEDIUM;
                break;
            case 3:
                the_power = AdvertisingSetParameters.TX_POWER_HIGH;
                break;
            default:
                assert false; // not reached
        }
        i.putExtra(ARG_PARAM_BLE_POWER_LEVEL, the_power);
        Log.d(LOG_BT_TAG, "power: "+the_power);
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // Inflate the layout for this fragment
        FrameLayout me = (FrameLayout) inflater.inflate(R.layout.fragment_ble_parameter, container, false);
        return me;
    }
}