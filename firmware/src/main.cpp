#include <Arduino.h>
#include <NeoPixelBus.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <EEPROM.h>
#include <limits.h>

#include "Keyframes.h"
#include "config.h"

union IndexFrameWrapper
{
  char buff[sizeof(IndexFrame)];
  IndexFrame iFrame;
};

typedef uint8_t nodeId_t;
constexpr uint8_t nodeId_to_index(nodeId_t x) {return x;}

#define CHECK_COMMAND(buff,cmd) (strncmp((buff), (cmd), COMMAND_LENGHT) == 0)

void stopCurrentAnimation();

enum States {IDLE, RUNNING, CYCLING, RECEIVING_KEYFRAMES};
States currentState = IDLE;

uint8_t channel_id = 0;
WiFiUDP MyUdp;
WiFiServer server(1235);
WiFiClient tcp_client;
// char incomingPacket[PACKET_BUFFER_SIZE];
Keyframes keys;
bool running = false;
unsigned long startTime = 0;
int32_t timeDrift = 0;
Keyframe* k1 = nullptr;
Keyframe* k2 = nullptr;
nodeId_t nodeId = 1;//TODO change for something that means "unitialized"

const uint16_t PixelCount = NUMBER_OF_PIXELS;
NeoPixelBus<NeoGrbFeature, NeoEsp8266AsyncUart800KbpsMethod> strip(PixelCount);
//NeoPixelBus<NeoGrbFeature, NeoEsp8266Uart800KbpsMethod> strip(PixelCount);
char lighterhostname[15];

ADC_MODE(ADC_VCC);

//This is an ugly hack due to the fact that nither WiFiUDP nor UdpContext implements a seek function.
//TODO, implement a seek function in UdpContext AND WiFiUDP.
inline void advanceServer(WiFiUDP& server, int bytes)
{
  for (int i = 0; i < bytes; i++)
  {
    server.read();
  }
}

void setupOTA()
{
  ArduinoOTA.onStart([](){
    Serial.println("StartFirmwareUpgrade");
  });

  ArduinoOTA.onEnd([](){
    Serial.println("\nFirmware update done");
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total){
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();

}

void serial_printf_pgm(PGM_P str, ...)
{
  char orig[30];
  strcpy_P(orig, str);
  char buff[40];
  va_list args;
  va_start(args, str);
  vsnprintf(buff, 40, orig, args);
  va_end(args);
  Serial.print(buff);
}

void serial_printf(const char* fmt, ...)
{
    char buff[50];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buff, 50, fmt, args);
    va_end(args);
    Serial.print(buff);
}

void showBootInformation()
{
  serial_printf_pgm(PSTR("Wedding Lighter by Zhaf (2017)\n"));
  serial_printf_pgm(PSTR("Firmware Version: %d.%d\n"), MAJOR_VERSION, MINOR_VERSION);
  serial_printf_pgm(PSTR("Firmware Size: %d\n"), ESP.getSketchSize());
  serial_printf_pgm(PSTR("Firmware free space: %d\n"), ESP.getFreeSketchSpace());
  serial_printf_pgm(PSTR("Free heap: %d\n"), ESP.getFreeHeap());
  serial_printf_pgm(PSTR("CPU Freq: %d\n"), ESP.getCpuFreqMHz());
  serial_printf_pgm(PSTR("Vcc: %d\n"), ESP.getVcc());
  serial_printf_pgm(PSTR("Reset Reason: %s\n"),ESP.getResetReason().c_str());
  serial_printf_pgm(PSTR("Wifi address: %s\n"),WiFi.localIP().toString().c_str());
  serial_printf_pgm(PSTR("Lighter channel: %d\n"), channel_id);
}

void setup()
{
  #ifdef DEBUG
  delayMicroseconds(2000*1000); //Pause for two second before start.(without yielding)
  #endif
  sprintf(lighterhostname, "lighter-%06x", ESP.getChipId());
  Serial.begin(115200);
  Serial.println();
  Serial.println("Booting...");
  EEPROM.begin(8); //TODO check the EEPROM final size.
  //channel_id = EEPROM.read(CHANNEL_ID_EEPROM_ADDRESS);

  strip.Begin();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  bool tmp = false;
  while (WiFi.status() != WL_CONNECTED)
  {
    if (tmp)
    {
      strip.ClearTo(RgbColor(0, 15, 15));
    }
    else
    {
      strip.ClearTo(RgbColor(0, 0, 0));
    }
    tmp = !tmp;
    strip.Show();
    delay(500);
    Serial.print(".");
    optimistic_yield(1000); //Avoid WDT soft reset
  }
  Serial.println();
  MyUdp.begin(UDP_PORT);
  server.begin();
  server.setNoDelay(true);
  //Serial.println(WiFi.localIP());
  showBootInformation();

  #ifdef ENABLE_OTA
  setupOTA();
  #endif

  //Strip test sequence.
  for (int i = 0; i < 2; i++)
  {
    optimistic_yield(1000); //Avoid WDT soft reset
    strip.ClearTo(RgbColor(100, 0, 0));
    strip.Show();
    delayMicroseconds(200*1000);
    strip.ClearTo(RgbColor(0));
    strip.Show();
    delayMicroseconds(100*1000);
    strip.ClearTo(RgbColor(0, 100, 0));
    strip.Show();
    delayMicroseconds(200*1000);
    strip.ClearTo(RgbColor(0));
    strip.Show();
    delayMicroseconds(100*1000);
    strip.ClearTo(RgbColor(0, 0, 100));
    strip.Show();
    delayMicroseconds(200*1000);
    strip.ClearTo(RgbColor(0));
    strip.Show();
    delayMicroseconds(100*1000);
  }
}

uint8_t linearInterpool(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint32_t x)
{
  int a = (x - x0);
  int b = (y1 - y0);
  int c = (x1 - x0);

  int res = y0 + ((a * b) / c);

  //int res = (y0 + ((x - x0) * (y1 - y0))/(x1 - x0));
  return (uint8_t) (0x000000ff & res);
}

void gotoIdle()
{
  currentState = States::IDLE;
}

void startKeyframeAnimation()
{
  k1 = keys.getFirst();
  k2 = k1->next;
  timeDrift = 0;
  startTime = millis();
  strip.ClearTo(RgbColor(k1->r, k1->g, k1->b));
  strip.Show();
}

void playKeyframes()
{
  if (keys.size() < 2)
  {
    serial_printf_pgm(PSTR("You need at least two keyframes\n"));
    return;
  }
  currentState = States::RUNNING;
  return startKeyframeAnimation();
}

void cycleKeyframes()
{
  if (keys.size() < 2)
  {
    serial_printf_pgm(PSTR("You need at least two keyframes\n"));
    return;
  }
  currentState = States::CYCLING;
  return startKeyframeAnimation();
}

void parseCommandIdle_tcp(WiFiClient& client, char* cmd)
{
  if (CHECK_COMMAND(cmd, COMMAND_CH_CHANNEL))
  {
    channel_id = cmd[2];
    serial_printf_pgm("New channel ID: %d\n", channel_id);
    return;
  }
  else if (CHECK_COMMAND(cmd, COMMAND_INPUT_KEYFRAMES))
  {
    serial_printf_pgm(PSTR("Receiving keyframes mode\n"));
    currentState = States::RECEIVING_KEYFRAMES;
  }
  else if (CHECK_COMMAND(cmd, COMMAND_CLEAR))
  {
    keys.clearAll();
  }
  else if (CHECK_COMMAND(cmd, COMMAND_LEDS_OFF))
  {
    strip.ClearTo(RgbColor(0));
    strip.Show();
  }
}

void parseCommandIdle(char* cmd, int len)
{
  if (len != COMMAND_LENGHT)
  {
    serial_printf_pgm(PSTR("Command invalid lenght\n"));
    return;
  }
  if (CHECK_COMMAND(cmd, COMMAND_INPUT_KEYFRAMES))
  {
    serial_printf_pgm(PSTR("Receiving keyframes mode\n"));
    currentState = States::RECEIVING_KEYFRAMES;
  }
  else if (CHECK_COMMAND(cmd, COMMAND_CLEAR))
  {
    keys.clearAll();
  }
  else if (CHECK_COMMAND(cmd, COMMAND_PRINT))
  {
    keys.print();
    serial_printf_pgm(PSTR("Free heap: %d\n"), ESP.getFreeHeap());
  }
  else if (CHECK_COMMAND(cmd, COMMAND_PLAY))
  {
    Serial.println("Starting play\n");
    return playKeyframes();

  }
  else if (CHECK_COMMAND(cmd, COMMAND_CYCLE))
  {
    return cycleKeyframes();
  }
  else if (CHECK_COMMAND(cmd, COMMAND_LEDS_OFF))
  {
    strip.ClearTo(RgbColor(0));
    strip.Show();
  }
  else if (CHECK_COMMAND(cmd, COMMAND_DISCOVERY))
  {
    //Discovery packet received, send response.
    MyUdp.beginPacket(MyUdp.remoteIP(), MyUdp.remotePort());
    MyUdp.write(lighterhostname);
    MyUdp.endPacket();
  }
  else
  {
    serial_printf_pgm(PSTR("Invalid command for IDLE: %s\n"), cmd);
  }
}

void parseKeyframes_tcp(WiFiClient& client, char* cmd, int len)
{
  if (len > 0)
  {
    if (CHECK_COMMAND(cmd, COMMAND_END_INPUT_KEYFRAMES))
    {
      serial_printf_pgm(PSTR("Keyframe input mode end\n"));
      return gotoIdle();
    }
  }

  //This is the TCP implementation.
  //First byte indicates the number of keyframes, and next are the keyframe data (no index header)
  uint8_t number_of_keys = client.read();
  uint8_t buff[sizeof(uint32_t)];
  for (uint8_t i = 0; i < number_of_keys; i++)
  {

    client.read(buff, sizeof(uint32_t));
    // int inIndex = (KEYFRAME_PACKET_SIZE*i);
    //To perform the next instruction, make sure you are accesing an addres 32bit aligned(0,4,8,etc)
    //Not doing so will raise an exception 9 (LoadStoreAlignmentCause)
    uint32_t time = *((uint32_t*)(buff));
    uint8_t r = client.read();//cmd[inIndex+4];
    uint8_t g = client.read();//cmd[inIndex+5];
    uint8_t b = client.read();//cmd[inIndex+6];
    client.read();//Read the padding.

    keys.insertKeyframe(time, r, g, b);
    yield();//Let the network do its job.
  }
  // serial_printf_pgm(PSTR("Time to insert all: %d\n"), t2-t1, ESP.getFreeHeap());
  serial_printf_pgm(PSTR("Free heap: %d\n"), ESP.getFreeHeap());
}


void parseAndAddKeyframes(WiFiUDP& server, char* cmd, int len)
{
  // unsigned long t1 = micros();
  if (len > 0)
  {
    if (CHECK_COMMAND(cmd, COMMAND_END_INPUT_KEYFRAMES))
    {
      serial_printf_pgm(PSTR("Keyframe input mode end\n"));
      return gotoIdle();
    }
  }

  //Parse index header.
  //TODO this is the OP2 implementation, implement OP1 version as well (may be is not neccesary)
  // int myIndexOffset = (sizeof IndexFrame) * nodeId_to_index(channel_id);
  //Calculate my index offset based on my nodeId
  int myIndexOffset = (sizeof(IndexFrame)) * channel_id;
  //Seek the server buffer.
  advanceServer(server, myIndexOffset);
  //Read my IndexFrame
  IndexFrameWrapper wrp;
  server.read(wrp.buff, sizeof(IndexFrame));

  //Calculate the data offset. And then seek the server.
  //The data offset in the IndexFrame counts from the beginning of the array, so we need to
  //substract the bytes we've already seek before, plus the size of the IndesFrame itself that we've readed.
  int dataOffset = wrp.iFrame.keys_offset - myIndexOffset - sizeof(IndexFrame);
  advanceServer(server, dataOffset);

  // int nKeys = len / KEYFRAME_PACKET_SIZE;
  for (int i = 0; i < wrp.iFrame.nKeys; i++)
  {
    char buff[sizeof(uint32_t)];
    server.read(buff, sizeof(uint32_t));
    // int inIndex = (KEYFRAME_PACKET_SIZE*i);
    //To perform the next instruction, make sure you are accesing an addres 32bit aligned(0,4,8,etc)
    //Not doing so will raise an exception 9 (LoadStoreAlignmentCause)
    uint32_t time = *((uint32_t*)(buff));
    uint8_t r = server.read();//cmd[inIndex+4];
    uint8_t g = server.read();//cmd[inIndex+5];
    uint8_t b = server.read();//cmd[inIndex+6];
    server.read();//Read the padding.

    keys.insertKeyframe(time, r, g, b);
    yield();//Let the network do its job.
  }
  server.flush();
  // unsigned long t2 = micros();
  // serial_printf_pgm(PSTR("Time to insert all: %d\n"), t2-t1, ESP.getFreeHeap());
  serial_printf_pgm(PSTR("Free heap: %d\n"), ESP.getFreeHeap());
}

void timeDriftCorrection(uint32_t syncTime)
{
  unsigned long inTime = millis();
  uint32_t elapsed = (uint32_t) inTime - startTime;
  timeDrift = syncTime - elapsed;
  Serial.printf("Master elapsed: %d, MyElapsed: %d, DriftCorrection: %d\n", syncTime, elapsed, timeDrift);
}

void parseCommandCycling_tcp(WiFiClient& client, char* cmd, int len)
{
  if (CHECK_COMMAND(cmd, COMMAND_STOP))
  {
    return stopCurrentAnimation();
  }
}

void parseCommandCycling(WiFiUDP& server, char* cmd, int len)
{
  if ((server.available() == 6) && server.peek() == (int)'s')
  {
    //uint32_t elapsed = 0;
    server.read();//Advance the "ss" chars
    server.read();
    //The server has calculated the timeDrift for me.
    server.read((char*)(&timeDrift), sizeof(int32_t));
    Serial.printf("Sync command receivd with drift: %d\n", timeDrift);
    //return timeDriftCorrection(elapsed);
    return;
  }
  if (server.available() > 6)
  {
    return parseAndAddKeyframes(server, cmd, len);
  }
  if (len != COMMAND_LENGHT)
  {
    serial_printf_pgm(PSTR("Command with invalid lenght"));
    return;
  }
  if (CHECK_COMMAND(cmd, COMMAND_STOP))
  {
    return stopCurrentAnimation();
  }
  else if (CHECK_COMMAND(cmd, COMMAND_ELAPSED_TIME))
  {
    //Print the elapsed time of the current animation
    unsigned long inTime = millis();
    uint32_t elapsed = (uint32_t) inTime - startTime;
    serial_printf_pgm(PSTR("Elapsed time: %d\n"), elapsed);
  }
  else if (CHECK_COMMAND(cmd, COMMAND_TIME_PING))
  {
    MyUdp.beginPacket(MyUdp.remoteIP(), MyUdp.remotePort());
    unsigned long inTime = millis();
    uint32_t elapsed = (uint32_t) inTime - startTime;
    elapsed += timeDrift;
    MyUdp.write((uint8_t*)&elapsed, sizeof(uint32_t));
    MyUdp.endPacket();
    Serial.printf("Time ping command received: %d\n", elapsed);
  }
  else if (CHECK_COMMAND(cmd, COMMAND_INC_DRIFT))
  {
    timeDrift += 10;
    Serial.printf("Drift: %d\n", timeDrift);
  }
  else if (CHECK_COMMAND(cmd, COMMAND_DEC_DRIFT))
  {
    timeDrift -= 10;
    Serial.printf("Drift: %d\n", timeDrift);
  }
  else
  {
    //TODO just for testing, don't allow insert new keyframes during cycles.
    //parseAndAddKeyframes(cmd, len);
  }
}

void receiver_tcp(WiFiClient& client)
{
  char cmd[PACKET_BUFFER_SIZE];
  int len = 0;
  //If the packet is a command, then its lenght will be less than 8bytes.
  if (client.available() <= 4)
  {
    len = client.read((uint8_t*)cmd, PACKET_BUFFER_SIZE);
    if (len > 0)
    {
      cmd[len] = 0;
      Serial.printf("Command: %s\n", cmd);
    }
  }

  switch(currentState)
  {
    case States::IDLE:
    {
      return parseCommandIdle_tcp(client, cmd);
    }
    case States::RECEIVING_KEYFRAMES:
    {
      return parseKeyframes_tcp(client, cmd, len);
    }
    case States::CYCLING:
    case States::RUNNING:
    {
      return parseCommandCycling_tcp(client, cmd, len);
    }
    default:
    {
      serial_printf_pgm(PSTR("Not valid TCP command at the moment\n"));
    }
  }
}

void receiver(WiFiUDP& server)
{
  char cmd[PACKET_BUFFER_SIZE];
  int len = 0;
  //If the packet is a command, then its lenght will be less than 8bytes.
  if (server.available() <= 4)
  {
    len = server.read(cmd, PACKET_BUFFER_SIZE);
    if (len > 0)
    {
      cmd[len] = 0;
      Serial.printf("Command: %s\n", cmd);
    }
  }

  switch(currentState)
  {
    case States::IDLE:
    {
      return parseCommandIdle(cmd, len);
    }
    case States::RECEIVING_KEYFRAMES:
    {
      return parseAndAddKeyframes(server, cmd, len);
    }
    case States::CYCLING:
    case States::RUNNING:
    {
      return parseCommandCycling(server, cmd, len);
    }
    default:
    {
      serial_printf_pgm(PSTR("Not valid command at the moment\n"));
    }
  }
}

void stopCurrentAnimation()
{
  k1 = nullptr;
  k2 = nullptr;
  return gotoIdle();
}

inline void restartOrEndAnimation()
{
  Serial.println("Ending the animation\n");
  //There are no more keyframes
  if (currentState == States::CYCLING)
  {
    k1 = keys.getFirst();
    k2 = k1->next;
    //Redraw first keyframe and restart.
    strip.ClearTo(RgbColor(k1->r, k1->g, k1->b));
    strip.Show();
    timeDrift = 0;
    startTime = millis();
  }
  else
  {
    //Draw last keyframe and end.
    k2 = keys.getLast();
    strip.ClearTo(RgbColor(k2->r, k2->g, k2->b));
    strip.Show();
    return stopCurrentAnimation();
  }
}

void generateFrame()
{
  unsigned long inTime = millis();
  uint32_t elapsed = (uint32_t) inTime - startTime;
  elapsed += timeDrift;

  printf("Elapsed: %06d Last: %06d NKeys: %04d\r", elapsed, keys.getLast()->time, keys.size());

  //Check if we are between k1 and k2 (usual case)
  if (elapsed <= k2->time)
  {
    uint8_t r = linearInterpool(k1->time, k1->r, k2->time, k2->r, elapsed);
    uint8_t g = linearInterpool(k1->time, k1->g, k2->time, k2->g, elapsed);
    uint8_t b = linearInterpool(k1->time, k1->b, k2->time, k2->b, elapsed);
    strip.ClearTo(RgbColor(r, g, b));
    strip.Show();
  }
  else
  {
    //We have reached k2, time to move to the next keyframe.
    //Skip keyframes if we are late. (the K2 time is lower than elapsed)
    do {
      k1 = k2;
      k2 = k2->next;
      if (currentState == States::RUNNING)
      {
        keys.removeFirst();
      }
    } while(k2 != nullptr && k2->time < elapsed);
    if (k2 == nullptr)
    {
      return restartOrEndAnimation();
    }

    uint8_t r = linearInterpool(k1->time, k1->r, k2->time, k2->r, elapsed);
    uint8_t g = linearInterpool(k1->time, k1->g, k2->time, k2->g, elapsed);
    uint8_t b = linearInterpool(k1->time, k1->b, k2->time, k2->b, elapsed);
    strip.ClearTo(RgbColor(r, g, b));
    strip.Show();

  }
}

unsigned long lastFrameTime = 0;
unsigned int loops = 0;
unsigned int minloops = UINT_MAX;
unsigned int maxloops = 0;
bool startCount = false;
void loop()
{
  //Check if we have an incoming packet.
  int packetSize = MyUdp.parsePacket();
  if (packetSize)
  {
    //serial_printf_pgm(PSTR("Received %d bytes from %s\n"), packetSize, MyUdp.remoteIP().toString().c_str());
    receiver(MyUdp);
  }
  yield();

  if (server.hasClient())
  {
    if (!tcp_client || !tcp_client.connected())
    {
      if (tcp_client)
      {
        tcp_client.flush();
        tcp_client.stop();
      }
      tcp_client = server.available();
    }
  }
  yield();

  if (tcp_client && tcp_client.available())
  {
    receiver_tcp(tcp_client);
    //clear the buffer
    tcp_client.flush();
    tcp_client.stop();
  }
  yield();

  if ((currentState == States::CYCLING) || (currentState == States::RUNNING))
  {
    // if (startCount)
    // {
    //   loops++;
    // }
    unsigned long now = millis();
    if ((now - lastFrameTime) > TIME_MS_BETWEEN_FRAMES)
    {
      generateFrame();
      //Serial.printf("FPS: %u, LOOPS: %08u, Max: %08u, Min: %08u\r", 1000/(now - lastFrameTime), loops, maxloops, minloops);
      lastFrameTime = now;
    //   if (startCount)
    //   {
    //     if (loops > maxloops)
    //       maxloops = loops;
    //     if (loops < minloops)
    //       minloops = loops;
    //     loops = 0;
    //   }
    //   startCount = true;
    }
  }
  else
  {
    // loops = 0;
    // minloops = UINT_MAX;
    // maxloops = 0;
    // startCount = false;
    #ifdef ENABLE_OTA
    ArduinoOTA.handle();
    #endif
  }
  yield();


}
