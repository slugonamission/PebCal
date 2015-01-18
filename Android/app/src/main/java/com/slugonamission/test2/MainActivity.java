package com.slugonamission.test2;

import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Toast;

import com.getpebble.android.kit.PebbleKit;
import com.getpebble.android.kit.util.PebbleDictionary;

import java.util.UUID;

public class MainActivity extends Activity {

    private static final UUID PebbleCalApp = UUID.fromString("372d6b1a-faa1-4df2-9b71-88fb0bbaaf2e");
    private static final int KEY_COMMAND = 1;
    private static final int KEY_DAYS_USED = 2;
    private static final int KEY_YEAR = 3;
    private static final int KEY_MONTH = 4;
    private static final int KEY_DAY = 5;

    private static final int CMD_NONE = 0;
    private static final int CMD_GET_DAYS_USED = 1;
    private static final int CMD_GET_AGENDA = 2;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        PebbleKit.registerReceivedDataHandler(getApplicationContext(), new PebbleKit.PebbleDataReceiver(PebbleCalApp) {
            @Override
            public void receiveData(Context context, int transaction, PebbleDictionary pebbleTuples) {
                Log.i("PEBCAL", "Got a message!");
                PebbleKit.sendAckToPebble(getApplicationContext(), transaction);

                // Build the response
                PebbleDictionary data = new PebbleDictionary();
                data.addUint32(KEY_COMMAND, CMD_GET_DAYS_USED);
                data.addUint32(KEY_DAYS_USED, 0x1000);
                data.addUint32(KEY_YEAR, 2015);
                data.addUint32(KEY_MONTH, 1);
                PebbleKit.sendDataToPebble(getApplicationContext(), PebbleCalApp, data);
            }
        });
    }


    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return true;
    }

    public void testButton(View view)
    {
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_settings) {
            return true;
        }

        if(id == R.id.action_lol)
        {
            Toast t = Toast.makeText(getApplicationContext(), "this is a toast!", Toast.LENGTH_SHORT);
            t.show();
        }

        return super.onOptionsItemSelected(item);
    }
}
