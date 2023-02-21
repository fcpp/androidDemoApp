package org.foldr.fcpp.androidDemo.evacuation1;

import static org.foldr.fcpp.androidDemo.Constants.LOG_TAG;
import static org.foldr.fcpp.androidDemo.evacuation1.EvacuationFragment.*;

import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.SwitchCompat;

import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.TextView;
import android.widget.Toast;

import org.foldr.fcpp.androidDemo.R;

import java.util.Random;

public class EvacuationParameters extends AppCompatActivity implements View.OnClickListener {

    private boolean group_left;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_evacuation_1_parameters);
        group_left = new Random().nextBoolean();
        ((TextView) findViewById(R.id.param_group)).setText(group_left ? "← Left" : "Right →");
    }

    @Override
    public void onClick(View view) {
        Log.d(LOG_TAG, "Starting experiment with parameters TODO.");
        String diameter = ((TextView) findViewById(R.id.param_diameter)).getText().toString();
        String retain = ((TextView) findViewById(R.id.param_retain)).getText().toString();
        String delay = ((TextView) findViewById(R.id.param_delay)).getText().toString();
        String evacuation_time = ((TextView) findViewById(R.id.param_evacuation_time)).getText().toString();
        boolean traitor = ((SwitchCompat) findViewById(R.id.param_traitor)).isChecked();
        Intent i = new Intent(this, EvacuationActivity.class);
        // We fail hard on conversion errors below.
        // We directly pass set most important values on to FCPP who's already running.
        try {
            i.putExtra(ARG_PARAM_DIAMETER, Integer.valueOf(diameter));
            i.putExtra(ARG_PARAM_RETAIN, Float.valueOf(retain));
            i.putExtra(ARG_PARAM_ROUND_PERIOD, Float.valueOf(delay));
            i.putExtra(ARG_PARAM_IS_GROUP_LEFT, group_left);
            i.putExtra(ARG_PARAM_TRAITOR, traitor);
            i.putExtra(ARG_PARAM_EVACUATION_TIME, Integer.valueOf(evacuation_time));
            startActivity(i);
            finish(); // terminate prefs dialog and continue.
        }  catch (NumberFormatException e) {
            // Shouldn't really happen since the UI has validation already.
            // TODO: could use setErrorText() on UI elements.
            Toast.makeText(this, "There's a problem with the parameters: "+e.getMessage(), Toast.LENGTH_LONG).show();
        }
    }
}