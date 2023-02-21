package org.foldr.fcpp.androidDemo.friendfinding1;

import android.graphics.Color;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
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

    private boolean isTraitor;
    private boolean is_group_left;

    public FriendFindingFragment() {
        // Required empty public constructor
    }

    /**
     * Use this factory method to create a new instance of
     * this fragment using the provided parameters.
     *
     * @return A new instance of fragment EvacuationFragment.
     */
    public static FriendFindingFragment newInstance() {
        FriendFindingFragment fragment = new FriendFindingFragment();
        return fragment;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // Inflate the layout for this fragment
        FrameLayout me = (FrameLayout) inflater.inflate(R.layout.fragment_friendfinding, container, false);
        TextView b1 = me.findViewById(R.id.result_1);
        TextView b2 = me.findViewById(R.id.result_2);
        TextView b3 = me.findViewById(R.id.result_3);
        TextView b4 = me.findViewById(R.id.result_4);
        b1.setBackgroundColor(Color.GRAY);
        b2.setBackgroundColor(Color.GRAY);
        b3.setBackgroundColor(Color.GRAY);
        b4.setBackgroundColor(Color.GRAY);

        TextView uid = me.findViewById(R.id.text_uid);
        uid.setText(Integer.toString(AP.uid));

        TextView version = me.findViewById(R.id.text_version);
        // TODO: should probably come from FCPP-code! (#16)
        version.setText("v1.0");

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
            }
        }, 1000);

        return me;
    }
}