
#include <Arduino.h>
#include <WiFi.h>

#include "ESPAsyncWebServer/ESPAsyncWebServer.h"

#include <WiFiUdp.h>
#include <OpenTherm.h>
#include "config.h"

void connectWIFI();
void connectMQTT();
void IRAM_ATTR handleInterrupt();
void web_otcmd(AsyncWebServerRequest * request);
void logOTRequest(unsigned long response);
void updateData();
void updateDataDiag();
void processResponse(unsigned long response, OpenThermResponseStatus status) ;
unsigned int buildRequest(byte req_idx);
void handleOpenTherm();

float oplo=LOW_BAND_TEMP,
      ophi=HIGH_BAND_TEMP,
      sp = 18,                  //set point
      t = 15,                   //current temperature
      t_last = 0,               //prior temperature
      ierr = 25,                //integral error
      dt = 0,                   //time between measurements
      op = 0;                   //PID controller output

uint8_t boiler_status = 0;

bool bCentralHeatingEnable=false;
bool bWaterHeatingEnable=false;
bool bHeatingMode=false;

bool bCentralHeating=false;
bool bWaterHeating=false;

bool isFlameOn=false,isCentralHeatingActive=false,isHotWaterActive=false;

int MaxModLevel=100;
float flameLevel=0,boilerTemp=0,dwhTemp=0,dwhTarget=45;
float nosp_override=NO_SP_TEMP_OVERRIDE;

float bParamChanged=false;        // Flag when value need to be pass to the boiler (eg: dwhTarget, bCentralHeatingEnable,...)


const unsigned long extTempTimeout_ms = 60 * 1000;
const unsigned long statusUpdateInterval_ms = 1000;
const unsigned long UpdateMqttInterval_ms = 5000;

const unsigned long spOverrideTimeout_ms = 30 * 1000;
const unsigned long pOverrideTimeout_ms = 30 * 1000;
const unsigned long UpdateDiagInterval_ms = 60 * 1000;

unsigned long ts = 0, new_ts = 0; //timestamp
unsigned long lastUpdate = 0;
unsigned long lastUpdateDiag = 0;
unsigned long lastMqttUpdate=0;
unsigned long lastTempSet = 0;

unsigned long startTime=0;   // timestamp of startup time
unsigned long lastSpSet = 0; // Timestamp of last SetPoint change
unsigned long lastPSet = 0;  // Timestamp of last Temperature Change