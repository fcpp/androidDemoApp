package org.foldr.fcpp.androidDemo;

import static org.foldr.fcpp.androidDemo.Constants.LOG_BT_TAG;

import android.app.Activity;
import android.bluetooth.le.AdvertisingSetParameters;
import android.bluetooth.le.ScanSettings;
import android.content.Intent;
import android.os.Bundle;

import androidx.fragment.app.Fragment;

import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.FrameLayout;
import android.widget.Spinner;

import com.google.android.material.slider.Slider;

public class BLEParameterFragment extends Fragment {

    public static final String ARG_PARAM_BLE_POWER_LEVEL = "BLE_POWER_LEVEL";
    public static final String ARG_PARAM_BLE_SCAN_MODE = "BLE_SCAN_MODE";

    public BLEParameterFragment() {
        // Required empty public constructor
    }

    public static void setExtraBLEPowerLevel(Activity me, Intent i) {
        float power = ((Slider) me.findViewById(R.id.param_power)).getValue();
        int[] POWERS = {AdvertisingSetParameters.TX_POWER_ULTRA_LOW,
                AdvertisingSetParameters.TX_POWER_LOW,
                AdvertisingSetParameters.TX_POWER_MEDIUM,
                AdvertisingSetParameters.TX_POWER_HIGH};
        int the_power = POWERS[Math.round(power)];
        i.putExtra(ARG_PARAM_BLE_POWER_LEVEL, the_power);
        Log.d(LOG_BT_TAG, "power: "+the_power);

        Spinner spinner = me.findViewById(R.id.scan_mode);
        // Order from arrays.xml:
        int[] BLEScanMode = {ScanSettings.SCAN_MODE_BALANCED, ScanSettings.SCAN_MODE_LOW_LATENCY, ScanSettings.SCAN_MODE_LOW_POWER};
        int scan_mode = BLEScanMode[spinner.getSelectedItemPosition()];
        i.putExtra(ARG_PARAM_BLE_SCAN_MODE, scan_mode);
        Log.d(LOG_BT_TAG, "scan mode: "+scan_mode);
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        FrameLayout me = (FrameLayout) inflater.inflate(R.layout.fragment_ble_parameter, container, false);
        // Scan mode settings:
        Spinner scan_mode = me.findViewById(R.id.scan_mode);
        ArrayAdapter<CharSequence> adapter = ArrayAdapter.createFromResource(getContext(),
                R.array.scan_mode, android.R.layout.simple_spinner_item);
        // Specify the layout to use when the list of choices appears
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        scan_mode.setAdapter(adapter);
        scan_mode.setSelection(1); // Low Latency

        return me;
    }
}