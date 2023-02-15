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

    private static final String ARG_PARAM_TRAITOR = "traitor";

    private boolean isTraitor;

    public EvacuationFragment() {
        // Required empty public constructor
    }

    /**
     * Use this factory method to create a new instance of
     * this fragment using the provided parameters.
     *
     * @return A new instance of fragment EvacuationFragment.
     */
    public static EvacuationFragment newInstance(boolean isTraitor) {
        EvacuationFragment fragment = new EvacuationFragment();
        Bundle args = new Bundle();
        args.putBoolean(ARG_PARAM_TRAITOR, isTraitor);
        fragment.setArguments(args);
        return fragment;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (getArguments() != null) {
            isTraitor = getArguments().getBoolean(ARG_PARAM_TRAITOR);
        } else {
            throw new RuntimeException();
        }
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

        TextView uid = me.findViewById(R.id.text_uid);
        uid.setText(Integer.toString(AP.uid));

        b1.setBackgroundColor(Color.GRAY);
        b2.setBackgroundColor(Color.GRAY);
        b3.setBackgroundColor(Color.GRAY);
        b4.setBackgroundColor(Color.GRAY);

        return me;
    }
}