package com.z.wedlit;

import android.content.Intent;
import android.os.Bundle;
import android.support.design.widget.FloatingActionButton;
import android.support.design.widget.Snackbar;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.view.View;

public class Playing extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_playing);
        Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);


    }

    public void onStopClicked(View view)
    {
        SendCommandTask cmd = new SendCommandTask(MainActivity.SERVER, MainActivity.PORT, "stopoff");
        cmd.execute();
        Intent intent = new Intent(Playing.this, MainActivity.class);
        startActivity(intent);
    }

}
