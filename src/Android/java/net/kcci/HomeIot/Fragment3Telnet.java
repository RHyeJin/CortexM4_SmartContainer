package net.kcci.HomeIot;

import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;
import androidx.fragment.app.Fragment;
import java.text.SimpleDateFormat;
import java.util.Date;
public class Fragment3Telnet extends Fragment {
    ClientThread clientThread;
    MainActivity mainActivity;
    EditText editTextIllu;
    EditText editTextHumi;
    EditText editTextTemp;
    Button buttonSetOk;
    SimpleDateFormat dataFormat = new SimpleDateFormat("yy.MM.dd HH:mm:ss");
    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.fragment3telnet, container, false);
        mainActivity = (MainActivity) getActivity();
        editTextIllu = view.findViewById(R.id.editTextIllu);
        editTextHumi = view.findViewById(R.id.editTextHumi);
        editTextTemp = view.findViewById(R.id.editTextTemp);
        buttonSetOk = view.findViewById(R.id.buttonSetOk);

        buttonSetOk.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if(ClientThread.socket != null){
                    editTextIllu.setHint("0");
                    editTextTemp.setHint("0");
                    editTextHumi.setHint("0");

                    mainActivity.clientThread.sendData(ClientThread.cortexId + "LIMIT"+"@"+editTextIllu.getText()
                            +"@"+editTextTemp.getText()+"@"+editTextHumi.getText()+"\n");
                }
                else{
                    Toast.makeText(getActivity(),"login 확인", Toast.LENGTH_SHORT).show();
                }
            }
        });
        return  view;
    }
    void recvDataProcess(String data) {
        Date date = new Date();
        String strDate = dataFormat.format(date);
        data += '\n';
        strDate = strDate + " " + data;
    }
}
