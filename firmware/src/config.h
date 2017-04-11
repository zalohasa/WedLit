#ifndef LIGHTER_CONFIG_H
#define LIGHTER_CONFIG_H

#include <Arduino.h>

constexpr const char* COMMAND_CYCLE = "cy";
constexpr const char* COMMAND_PLAY = "pl";
constexpr const char* COMMAND_PRINT = "pr";
constexpr const char* COMMAND_CLEAR = "cl";
constexpr const char* COMMAND_STOP = "sp";
constexpr const char* COMMAND_INPUT_KEYFRAMES = "in";
constexpr const char* COMMAND_END_INPUT_KEYFRAMES = "en";
constexpr const char* COMMAND_LEDS_OFF = "of";
constexpr const char* COMMAND_DISCOVERY = "dd";
constexpr const char* COMMAND_ELAPSED_TIME = "et";
constexpr const char* COMMAND_SYNC = "ss";
constexpr const char* COMMAND_INC_DRIFT = "ud";
constexpr const char* COMMAND_DEC_DRIFT = "du";
constexpr const char* COMMAND_CH_CHANNEL = "ch";
constexpr const char* COMMAND_TIME_PING = "tp";



constexpr uint8_t MAJOR_VERSION = 0;
constexpr uint8_t MINOR_VERSION = 1;
constexpr size_t COMMAND_LENGHT = 2;
constexpr unsigned int UDP_PORT = 1234;
constexpr size_t KEYFRAME_PACKET_SIZE = 8;
constexpr size_t PACKET_BUFFER_SIZE = 4;
constexpr unsigned long TIME_MS_BETWEEN_FRAMES=19; //~50FPS - Ms to wait between frames.
//constexpr unsigned long TIME_MS_BETWEEN_FRAMES=9; //~100FPS
//constexpr unsigned long TIME_MS_BETWEEN_FRAMES=38; //~25FPS
constexpr uint16_t NUMBER_OF_PIXELS = 24; //Number of leds in the led array

constexpr size_t EEPROM_MAP_SIZE = 8; //Size of the eeprom to map
constexpr int CHANNEL_ID_EEPROM_ADDRESS = 0; //Addres of the channel id in the EEPROM

const char* ssid = "WedLit";
const char* password = "WedLit2017";

#endif //LIGHTER_CONFIG_H
