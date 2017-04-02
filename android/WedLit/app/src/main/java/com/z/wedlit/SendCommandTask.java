package com.z.wedlit;

import android.os.AsyncTask;
import android.util.Log;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.net.UnknownHostException;

/**
 * Created by zalo on 2/04/17.
 */

public class SendCommandTask extends AsyncTask<Void, Void, Void> {
    String address;
    int port;
    String command;

    SendCommandTask(String address, int port, String command)
    {
        this.address = address;
        this.port = port;
        this.command = command;
    }

    @Override
    protected Void doInBackground(Void... arg)
    {
        Socket socket = null;
        try
        {
            Log.d("SENDCOMMAND", "Connecting...");
            socket = new Socket(address, port);
            OutputStream output = socket.getOutputStream();

            Log.d("SENDCOMMAND", "Sending command: " + command);
            output.write(command.getBytes());

        } catch (UnknownHostException ex)
        {
            ex.printStackTrace();
            Log.e("SENDCOMMAND", ex.toString());
        } catch (IOException ex)
        {
            ex.printStackTrace();
            Log.e("SENDCOMMAND", ex.toString());
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

}
