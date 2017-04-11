package com.z.wedlit;

import android.content.Intent;
import android.util.Log;
import android.os.Bundle;
import android.support.design.widget.FloatingActionButton;
import android.support.design.widget.Snackbar;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.view.View;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ListView;

public class MainActivity extends AppCompatActivity {

    public static final String SERVER = "10.0.4.1";
    public static final int PORT = 3210;

    ArrayAdapter<String> list;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);

        final ListView listview = (ListView) findViewById(R.id.listview);
        //String[] values = new String[]{"pepe","paco","toño","chechu"};

        list = new ArrayAdapter<String>(this, R.layout.row, R.id.anim_name);
        listview.setAdapter(list);

        listview.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> adapterView, View view, int i, long l)
            {
                String anim = list.getItem(i);
                SendCommandTask cmd = new SendCommandTask(SERVER, PORT, "play:"+anim);
                cmd.execute();

                Log.i("MAIN", "Playing animation " + anim);

                Intent intent = new Intent(MainActivity.this, Playing.class);
                startActivity(intent);
            }

        });

        UpdateAnimsTask upList = new UpdateAnimsTask(SERVER, PORT, list);
        upList.execute();

    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return true;
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

        return super.onOptionsItemSelected(item);
    }
}
