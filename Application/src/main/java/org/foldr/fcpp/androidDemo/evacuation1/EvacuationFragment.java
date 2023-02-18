package org.foldr.fcpp.androidDemo.evacuation1;

import android.graphics.Color;
import android.os.Bundle;

import androidx.fragment.app.Fragment;

import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.FrameLayout;
import android.widget.TextView;

import org.foldr.fcpp.androidDemo.AP;
import org.foldr.fcpp.androidDemo.R;

/**
 * A simple {@link Fragment} subclass.
 * Use the {@link EvacuationFragment#newInstance} factory method to
 * create an instance of this fragment.
 */
public class EvacuationFragment extends Fragment {

    static final String ARG_PARAM_TRAITOR = "traitor";
    static final String ARG_PARAM_IS_GROUP_LEFT = "evacuation_group";
    static final String ARG_PARAM_EVACUATION_TIME = "evacuation_time";

    private boolean isTraitor;
    private boolean is_group_left;

    public EvacuationFragment() {
        // Required empty public constructor
    }

    /**
     * Use this factory method to create a new instance of
     * this fragment using the provided parameters.
     *
     * @return A new instance of fragment EvacuationFragment.
     */
    public static EvacuationFragment newInstance(boolean isTraitor, boolean is_group_left, int evacuation_time) {
        EvacuationFragment fragment = new EvacuationFragment();
        Bundle args = new Bundle();
        args.putBoolean(ARG_PARAM_TRAITOR, isTraitor);
        args.putBoolean(ARG_PARAM_IS_GROUP_LEFT, is_group_left);
        args.putInt(ARG_PARAM_EVACUATION_TIME, evacuation_time);
        fragment.setArguments(args);
        return fragment;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (getArguments() == null) {
            throw new RuntimeException();
        }
        isTraitor = getArguments().getBoolean(ARG_PARAM_TRAITOR);
        is_group_left = getArguments().getBoolean(ARG_PARAM_IS_GROUP_LEFT);
        AP.set_bool(ARG_PARAM_IS_GROUP_LEFT, is_group_left);
        AP.set_int(ARG_PARAM_EVACUATION_TIME, getArguments().getInt(ARG_PARAM_EVACUATION_TIME));
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // Inflate the layout for this fragment
        FrameLayout me = (FrameLayout) inflater.inflate(R.layout.fragment_evacuation, container, false);
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

        TextView state_rg = me.findViewById(R.id.text_state_rg);
        state_rg.setText("?"); // The XML was allergic to `?`.
        state_rg.setBackgroundColor(Color.GRAY);
        // Set up refresh every 5 secs.
        state_rg.postDelayed(new Runnable() {
            @Override
            public void run() {
                // TODO -- first approximation, should at least contain self.
                // But VS thinks this is from the old example, so it might need changing.
                if ("{*:inf}".equals(AP.get_nbr_lags())) {
                    state_rg.setBackgroundColor(Color.GREEN);
                    state_rg.setText("✓");
                } else {
                    state_rg.setBackgroundColor(Color.RED);
                    state_rg.setText("x");
                }
                // Log.d(LOG_TAG, AP.get_nbr_lags());
                state_rg.postDelayed(this, 250); // while(true)...
                b1.setBackgroundColor(AP.get_bool("evacuation_done") ? Color.GREEN : Color.RED);
                b2.setBackgroundColor(AP.get_bool("homogeneous_group") ? Color.GREEN : Color.RED);
                b3.setBackgroundColor(AP.get_bool("traitor_free") ? Color.GREEN : Color.RED);
                b4.setBackgroundColor(AP.get_bool(ARG_PARAM_IS_GROUP_LEFT) == is_group_left? Color.GREEN : Color.RED);
            }
        }, 5000);

        TextView traitorView = me.findViewById(R.id.text_traitor);
        traitorView.setText(isTraitor ? "You're the traitor!" : "You behave normally.");
        traitorView.setBackgroundColor(isTraitor ? Color.RED : Color.GREEN);

        TextView group_rg = me.findViewById(R.id.text_group_lr);
        group_rg.setText("Group:"+(is_group_left ? GROUP.Left : GROUP.Right));
        if (isTraitor) {
            group_rg.setBackgroundColor(Color.YELLOW);
            group_rg.setTextColor(Color.BLACK);
        }
        return me;
    }

    public enum GROUP {
        Left, Right
    }
}