package com.z.wedlit;

import android.os.AsyncTask;
import android.util.Log;
import android.widget.ArrayAdapter;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.net.UnknownHostException;

/**
 * Created by zalo on 1/04/17.
 */

public class UpdateAnimsTask extends AsyncTask<Void, Void, Void> {

    ArrayAdapter<String> adapter;
    String address;
    String response;
    int port;

    UpdateAnimsTask(String addr, int port, ArrayAdapter<String> adapter)
    {
        this.port = port;
        this.address = addr;
        this.adapter = adapter;
    }

    @Override
    protected Void doInBackground(Void... arg)
    {
        Socket socket = null;
        try
        {
            Log.d("ANIMTASK", "Connecting...");
            socket = new Socket(address, port);
            int bytesRead;
            InputStream input = socket.getInputStream();
            OutputStream output = socket.getOutputStream();

            String cmd = "listanim";
            output.write(cmd.getBytes());

            ByteArrayOutputStream baos = new ByteArrayOutputStream(1024);
            byte[] buffer = new byte[1024];

            bytesRead = input.read(buffer);
            baos.write(buffer, 0, bytesRead);
            response = baos.toString();

        } catch (UnknownHostException ex)
        {
            ex.printStackTrace();
            Log.e("ANIMTASK", ex.toString());
        } catch (IOException ex)
        {
            ex.printStackTrace();
            Log.e("ANIMTASK", ex.toString());
        } finally
        {
            if (socket != null)
            {
                try
                {
                    socket.close();
                } catch (IOException ex)
                {
                    ex.printStackTrace();
                }
            }
        }
        return null;
    }

    @Override
    protected void onPostExecute(Void result)
    {
        String[] anims = response.split("\n");
        adapter.clear();
        adapter.addAll(anims);
    }
}
