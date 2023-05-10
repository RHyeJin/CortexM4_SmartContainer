package net.kcci.HomeIot;

import android.os.AsyncTask;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.ProgressBar;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;

import androidx.fragment.app.Fragment;
public class Fragment2Dashboard extends Fragment {
    MainActivity mainActivity;
    EditText editTextSec;
    EditText editTextMin;
    EditText editTextHour;
    ImageView imageViewAir;
    ImageView imageViewClock;
    Button buttonSetTimer;
    TextView textViewPwm;
    ImageButton imageButtonLeft;
    ImageButton imageButtonRight;
    ProgressBar progressBar;
    TextView textView;
    TextView textViewProg;
    String temp;

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.fragment2dashboard, container, false);
        mainActivity = (MainActivity) getActivity();
        imageViewAir = view.findViewById(R.id.imageViewAir);
        imageViewClock = view.findViewById(R.id.imageViewClock);
        editTextHour = view.findViewById(R.id.editTextHour);
        editTextMin = view.findViewById(R.id.editTextMin);
        editTextSec = view.findViewById(R.id.editTextSec);
        imageButtonLeft = view.findViewById(R.id.imageButtonLeft);
        imageButtonRight = view.findViewById(R.id.imageButtonRight);
        buttonSetTimer = view.findViewById(R.id.buttonSetTimer);
        textViewPwm = view.findViewById(R.id.textViewPwm);
        progressBar = view.findViewById(R.id.progressBar);
        textView = view.findViewById(R.id.textView);
        textViewProg = view.findViewById(R.id.textViewProg);

        editTextHour.setHint("0");
        editTextMin.setHint("0");
        editTextSec.setHint("0");
        progressBar.setMax(100);
        progressBar.setIndeterminate(false);
        textViewProg.setHint("0%");

        buttonSetTimer.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if (ClientThread.socket != null) {
                    mainActivity.clientThread.sendData(ClientThread.cortexId + "TIMESET"+"@"
                            +editTextHour.getText()+"@"+editTextMin.getText()+"@"+editTextSec.getText()+"\n");
                } else
                    Toast.makeText(getActivity(),"login 확인", Toast.LENGTH_SHORT).show();
            }
        });
        imageButtonLeft.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                String temp = (String) textViewPwm.getText();
                int res = (Integer.parseInt(temp)-10);
                temp = String.valueOf(res);
                textViewPwm.setText(temp);
                if (ClientThread.socket != null) {
                    mainActivity.clientThread.sendData(ClientThread.cortexId + "MOT"+"@"+"PWM"+"@"+textViewPwm.getText()+"\n");
                } else
                    Toast.makeText(getActivity(), "login 확인", Toast.LENGTH_SHORT).show();
            }
        });
        imageButtonRight.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                String temp = (String) textViewPwm.getText();
                int res = (Integer.parseInt(temp)+10);
                temp = String.valueOf(res);
                textViewPwm.setText(temp);
                if (ClientThread.socket != null) {
                    mainActivity.clientThread.sendData(ClientThread.cortexId + "MOT"+"@"+"PWM"+"@"+textViewPwm.getText()+"\n");
                } else
                    Toast.makeText(getActivity(), "login 확인", Toast.LENGTH_SHORT).show();
            }
        });
        return view;
    }
    public void recvDataProcess(String recvData) {

        String[] splitLists = recvData.toString().split("\\[|]|@|\\n");
        if(splitLists[2].equals("PROGRESS")) {
            temp = splitLists[3] + "%";
            float ab = Float.parseFloat(splitLists[3]);
            int pr = (int) ab;
            textViewProg.setText(temp);
            progressBar.setProgress(pr);
        }
    }
}
