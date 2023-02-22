package org.foldr.fcpp.androidDemo.friendfinding1;

import static org.foldr.fcpp.androidDemo.friendfinding1.FriendFindingParameters.ARG_PARAM_USE_LAGS;

import android.graphics.Color;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.EditText;
import android.widget.FrameLayout;
import android.widget.TextView;

import androidx.fragment.app.Fragment;

import org.foldr.fcpp.androidDemo.AP;
import org.foldr.fcpp.androidDemo.R;

/**
 * A simple {@link Fragment} subclass.
 * Use the {@link FriendFindingFragment#newInstance} factory method to
 * create an instance of this fragment.
 */
public class FriendFindingFragment extends Fragment {

    static final String ARG_PARAM_ROUND_PERIOD = "round_period";
    static final String ARG_PARAM_DIAMETER = "diameter";
    static final String ARG_PARAM_RETAIN = "retain_time";
    static final int[] STATE_COLORS = {Color.GRAY, Color.GREEN, Color.YELLOW, Color.RED};
    static final String[] STATE_TEXTS = {"?", "✓", "x", "X"};

    public FriendFindingFragment() {
        // Required empty public constructor
    }

    /**
     * Use this factory method to create a new instance of
     * this fragment using the provided parameters.
     *
     * @return A new instance of fragment EvacuationFragment.
     */
    public static FriendFindingFragment newInstance(boolean use_lags) {
        FriendFindingFragment fragment = new FriendFindingFragment();
        Bundle args = new Bundle();
        args.putBoolean(ARG_PARAM_USE_LAGS, use_lags);
        fragment.setArguments(args);
        return fragment;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        boolean useLags = getArguments().getBoolean(ARG_PARAM_USE_LAGS);
        AP.set_bool("use_lags", useLags);
    }
    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // Inflate the layout for this fragment
        FrameLayout me = (FrameLayout) inflater.inflate(R.layout.fragment_friendfinding, container, false);

        TextView uid = me.findViewById(R.id.text_uid);
        uid.setText(Integer.toString(AP.uid));

        TextView version = me.findViewById(R.id.text_version);
        // TODO: should probably come from FCPP-code! (#16)
        version.setText("v1.0");

        Button searchButton = me.findViewById(R.id.search);
        searchButton.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                // Code here executes on main thread after user presses search button
                Button searchButton = me.findViewById(R.id.search);
                EditText friendID = me.findViewById(R.id.friend_id);
                int fid = AP.get_int("friend_requested");
                if (fid > 0) { // reporting to have found the friend
                    AP.set_int("friend_requested", 0);
                    friendID.setText("");
                    searchButton.setText("Search!");
                    searchButton.setEnabled(false);
                } else { // starting to search for a friend
                    friendID.setEnabled(false);
                    int ID = 0;
                    if (friendID.getText().length() > 0) ID = Integer.valueOf(friendID.getText().toString());
                    AP.set_int("friend_requested", ID);
                    searchButton.setText("Found!");
                }
            }
        });

        TextView state_rg = me.findViewById(R.id.text_state_rg);
        state_rg.setText(STATE_TEXTS[0]); // The XML was allergic to `?`.
        state_rg.setBackgroundColor(STATE_COLORS[0]);
        // Set up refresh every 5 secs.
        state_rg.postDelayed(new Runnable() {
            @Override
            public void run() {
                if (AP.is_stopping) return;
                state_rg.setBackgroundColor(STATE_COLORS[AP.get_int("not_alone")]);
                state_rg.setText(STATE_TEXTS[AP.get_int("not_alone")]);
                // Log.d(LOG_TAG, AP.get_nbr_lags());
                state_rg.postDelayed(this, 250); // while(true)...
                TextView distanceView = me.findViewById(R.id.dist_score);
                double dist = AP.get_double("distance_score");
                distanceView.setText(String.format("%.2f", dist));
                float k = (float)(dist / AP.get_int("diameter"));
                distanceView.setBackgroundColor(Color.argb(1.0f, 1-k, 0.0f, k));
                EditText friendID = me.findViewById(R.id.friend_id);
                int ID = 0;
                if (friendID.getText().length() > 0) ID = Integer.valueOf(friendID.getText().toString());
                searchButton.setEnabled(ID > 0);
            }
        }, 1000);

        return me;
    }
}