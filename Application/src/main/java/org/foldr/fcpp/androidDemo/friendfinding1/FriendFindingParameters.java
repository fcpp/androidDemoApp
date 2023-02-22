package org.foldr.fcpp.androidDemo.friendfinding1;

import static org.foldr.fcpp.androidDemo.Constants.LOG_BT_TAG;
import static org.foldr.fcpp.androidDemo.Constants.LOG_TAG;
import static org.foldr.fcpp.androidDemo.evacuation1.EvacuationFragment.ARG_PARAM_DIAMETER;
import static org.foldr.fcpp.androidDemo.evacuation1.EvacuationFragment.ARG_PARAM_RETAIN;
import static org.foldr.fcpp.androidDemo.evacuation1.EvacuationFragment.ARG_PARAM_ROUND_PERIOD;

import android.bluetooth.le.AdvertisingSetParameters;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.TextView;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.SwitchCompat;

import com.google.android.material.slider.Slider;

import org.foldr.fcpp.androidDemo.AP;
import org.foldr.fcpp.androidDemo.R;

public class FriendFindingParameters extends AppCompatActivity implements View.OnClickListener {

    public static final String ARG_PARAM_BLE_POWER_LEVEL = "BLE_POWER_LEVEL";
    public static final String ARG_PARAM_USE_LAGS = "USE_LAGS_FOR_DIST";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_friendfinding_1_parameters);
    }

    @Override
    public void onClick(View view) {
        Log.d(LOG_TAG, "Starting experiment with parameters TODO.");
        String diameter = ((TextView) findViewById(R.id.param_diameter)).getText().toString();
        String retain = ((TextView) findViewById(R.id.param_retain)).getText().toString();
        String delay = ((TextView) findViewById(R.id.param_delay)).getText().toString();
        float power = ((Slider) findViewById(R.id.param_power)).getValue();
        SwitchCompat lagSwitch = findViewById(R.id.param_lags);
        boolean useLags = lagSwitch.isChecked();
        Intent i = new Intent(this, FriendFindingActivity.class);
        i.putExtra(ARG_PARAM_USE_LAGS, useLags);
        // We fail hard on conversion errors below.
        // We directly pass set most important values on to FCPP who's already running.
        try {
            i.putExtra(ARG_PARAM_DIAMETER, Integer.valueOf(diameter));
            i.putExtra(ARG_PARAM_RETAIN, Float.valueOf(retain));
            i.putExtra(ARG_PARAM_ROUND_PERIOD, Float.valueOf(delay));
            setExtraBLEPowerLevel(i, power);
            startActivity(i);
            finish(); // terminate prefs dialog and continue.
        }  catch (NumberFormatException e) {
            // Shouldn't really happen since the UI has validation already.
            // TODO: could use setErrorText() on UI elements.
            Toast.makeText(this, "There's a problem with the parameters: "+e.getMessage(), Toast.LENGTH_LONG).show();
        }
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
        }
        i.putExtra(ARG_PARAM_BLE_POWER_LEVEL, the_power);
        Log.d(LOG_BT_TAG, "power: "+the_power);
    }
}