package org.foldr.fcpp.androidDemo.evacuation1;

import static org.foldr.fcpp.androidDemo.Constants.LOG_TAG;

import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.SwitchCompat;

import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.TextView;

import org.foldr.fcpp.androidDemo.AP;
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
        boolean traitor = ((SwitchCompat) findViewById(R.id.param_traitor)).isChecked();
        Intent i = new Intent(this, EvacuationActivity.class);
        // We fail hard on conversion errors below.
        // We directly pass set most important values on to FCPP who's already running.
        i.putExtra("diameter", Integer.valueOf(diameter));
        AP.set_diameter(Integer.valueOf(diameter));
        i.putExtra("retain", Integer.valueOf(retain));
        AP.set_retain_time(Integer.valueOf(retain));
        i.putExtra("delay", Integer.valueOf(delay));
        i.putExtra("group_left", group_left);
        i.putExtra("traitor", traitor);
        startActivity(i);
        finish(); // terminate prefs dialog.
    }
}