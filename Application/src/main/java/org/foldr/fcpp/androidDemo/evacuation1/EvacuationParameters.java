package org.foldr.fcpp.androidDemo.evacuation1;

import static org.foldr.fcpp.androidDemo.Constants.LOG_TAG;

import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.SwitchCompat;

import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.TextView;

import org.foldr.fcpp.androidDemo.R;

public class EvacuationParameters extends AppCompatActivity implements View.OnClickListener {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_evacuation_1_parameters);
    }

    @Override
    public void onClick(View view) {
        Log.d(LOG_TAG, "Starting experiment with parameters TODO.");
        String diameter = ((TextView) findViewById(R.id.param_diameter)).getText().toString();
        String retain = ((TextView) findViewById(R.id.param_retain)).getText().toString();
        String delay = ((TextView) findViewById(R.id.param_delay)).getText().toString();
        String traitor = ((SwitchCompat) findViewById(R.id.param_traitor)).getText().toString();
        Intent i = new Intent(this, EvacuationActivity.class);
        i.putExtra("diameter", Integer.valueOf(diameter));
        i.putExtra("retain", Integer.valueOf(retain));
        i.putExtra("delay", Integer.valueOf(delay));
        i.putExtra(getString(R.string.param_traitor), "Yes".equals(traitor));
        startActivity(i);
        finish(); // terminate prefs dialog.
    }
}