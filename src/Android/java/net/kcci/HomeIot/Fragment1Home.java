package net.kcci.HomeIot;

import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ProgressBar;
import android.widget.ScrollView;
import android.widget.TextView;
import android.widget.Toast;

import androidx.fragment.app.Fragment;

import java.text.SimpleDateFormat;
import java.util.Date;


public class Fragment1Home extends Fragment {
    MainActivity mainActivity;
    ScrollView scrollViewRecv;
    TextView textView;
    TextView textViewRecv;
    TextView textViewTitle;

    SimpleDateFormat dataFormat = new SimpleDateFormat("yy.MM.dd HH:mm:ss");

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.fragment1home, container, false);
        mainActivity = (MainActivity) getActivity();
        scrollViewRecv = view.findViewById(R.id.scrollViewRecv);
        textViewRecv = view.findViewById(R.id.textViewRecv);
        textViewTitle = view.findViewById(R.id.textViewTitle);

        textViewTitle.setText("DATABASE");

        return view;
    }

    void recvDataProcess(String strRecvData) {
        Date date = new Date();
        String data="";
        String tokDate = dataFormat.format(date);
        String strDate = dataFormat.format(date);
        strRecvData += '\n';
        strDate = strDate + " " + strRecvData;
        String[] splitLists = strDate.toString().split("\\[|]|@|\\n");
        if(splitLists[2].equals("VAL")) {
            String[] tokArray = splitLists[3].toString().split(",");
            data += tokDate + ", ";
            data += "Illu : " + tokArray[0];
            data += ", Temp : " + tokArray[1];
            data += ", Humi : " + tokArray[2];
            data += ", Dist : " + tokArray[3];
            data += '\n';
            textViewRecv.append(data);
            scrollViewRecv.fullScroll(View.FOCUS_DOWN);
        }
    }
}
