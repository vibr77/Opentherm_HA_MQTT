/*
__   _____ ___ ___        Author: Vincent BESSON
 \ \ / /_ _| _ ) _ \      Release: 0.51
  \ V / | || _ \   /      Date: 20230919
   \_/ |___|___/_|_\      Description: ESP32 Mini Home Assistant Opentherm Master Thermostat
                2023      Licence: Creative Commons
______________________

Release changelog:
------------------
  +20231016: Stable integrated release, test to be done
  +20231001: First tested & stable version
  +20230919: Initial Commit
 

Todo:
-----------------------
+ Add identify button with Led on PCB             //
+ Connect reboot reason                           // OK to be tested
+ Check on MQTT disconnection MQTT Server Log
+ MQTT disconnect -> watchdog reboot to be fixed  // OK
+ Link OT Log [OK]                                // OK
+ Test the default config button
+ Write the doc                                   // Ongoing
+ WIFI & SETTING CAPTIVE PORTAL             ex:      https://github.com/adamff-dev/ESP8266-Captive-Portal/blob/master/WiFi_Captive_Portal.ino
*/

#include <esp_task_wdt.h>
#define WDT_TIMEOUT   30

#include <Arduino.h>
#include <WiFi.h>

#include "ESPAsyncWebServer/ESPAsyncWebServer.h"

#include <WiFiUdp.h>

#include <ArduinoOTA.h>
#include <ESPmDNS.h>

#include "RemoteDebug.h"

#define MASTER_STATUS_CH_ENABLED 0x1
#define MASTER_STATUS_DHW_ENABLED 0x2
#define MASTER_STATUS_COOLING_ENABLED 0x4

#include <OpenTherm.h>
#include <PubSubClient.h>

#define MQTT_MAX_PACKET_SIZE 256

#include "include/main.h"
#include "include/config.h"
#include "include/pwd.h"
#include "include/mqtt_com.h"

#include "Adafruit_HTU31D.h"

Adafruit_HTU31D htu = Adafruit_HTU31D();

RemoteDebug Debug;

OpenTherm ot(OT_IN_PIN, OT_OUT_PIN);
WiFiClient espClient;
PubSubClient client((const char*)MQTT_SERVER, MQTT_PORT,  espClient);
AsyncWebServer server(80);
sensors_event_t humidity, temp; 

int status = WL_IDLE_STATUS;

void IRAM_ATTR handleInterrupt() {
    ot.handleInterrupt();
}
/* KEEP FOR REFERENCE FROM THE LIB
typedef enum {
    ESP_RST_UNKNOWN,    //!< Reset reason can not be determined
    ESP_RST_POWERON,    //!< Reset due to power-on event
    ESP_RST_EXT,        //!< Reset by external pin (not applicable for ESP32)
    ESP_RST_SW,         //!< Software reset via esp_restart
    ESP_RST_PANIC,      //!< Software reset due to exception/panic
    ESP_RST_INT_WDT,    //!< Reset (software or hardware) due to interrupt watchdog
    ESP_RST_TASK_WDT,   //!< Reset due to task watchdog
    ESP_RST_WDT,        //!< Reset due to other watchdogs
    ESP_RST_DEEPSLEEP,  //!< Reset after exiting deep sleep mode
    ESP_RST_BROWNOUT,   //!< Brownout reset (software or hardware)
    ESP_RST_SDIO,       //!< Reset over SDIO
} esp_reset_reason_t;
*/

uint8_t requests[] = {
  OpenThermMessageID::Status, // READ
  OpenThermMessageID::TSet, // WRITE
  OpenThermMessageID::Tboiler, // READ
  OpenThermMessageID::Tdhw, // READ
  OpenThermMessageID::TdhwSet, // WRITE
  OpenThermMessageID::RelModLevel, // READ
  OpenThermMessageID::MaxRelModLevelSetting, // WRITE
  OpenThermMessageID::Tret // WRITE
};

const byte requests_count = sizeof(requests) / sizeof(uint8_t);
byte req_idx = 0;


float pid(float sp, float pv, float pv_last, float& ierr, float dt) {
  float KP = 10;
  float KI = 0.02;

  float op=0;
/*
  if (bHeatingMode==false || bCentralHeatingEnable==false){
    op=oplo;
    ESP_LOGI("MAIN","PID op:[%f]=oplo:[%f] bHeatingMode==false || bCentralHeatingEnable==false ",op,oplo);
  
    return op;
  }
*/ 

  // calculate the error
  float error = sp - pv;
  // calculate the integral error
  ierr = ierr + KI * error * dt;
  // calculate the measurement derivative
  //float dpv = (pv - pv_last) / dt;
  // calculate the PID output
  float P = KP * error; //proportional contribution
  float I = ierr; //integral contribution
  op = P + I;
  // implement anti-reset windup
  if ((op < oplo) || (op > ophi)) {
    I = I - KI * error * dt;
    // clip output
    op = max(oplo, min(ophi, op));
  }
  ierr = I;

  LOGI("MAIN","PID op:[%f] pv:[%f] dt:[%f] sp:[%f] P:[%f] I:[%f]",op,pv,dt,sp,P,I);
  return op;
}

String MacAddr;

void connectWIFI(){
  
  while (WiFi.status() != WL_CONNECTED) {                     // attempt to connect to Wifi network
    status = WiFi.begin(WIFI_SSID, WIFI_KEY); 
    WiFi.setSleep(false);                
    uint8_t timeout = 10;
    while (timeout && (WiFi.status() != WL_CONNECTED)) { // wait 10 seconds for connection:
      switchLed(LED_RED,true);      
      timeout--;
      delay(1000);
    }
    if (WiFi.status()==WL_CONNECTED){
       switchLed(LED_RED,false);
      LOGI("MAIN","WiFi connected to ssid:%s",WIFI_SSID);
      
    }
  } 
  return;
}

void connectMQTT(){

  if (WiFi.status() != WL_CONNECTED ){
    LOGI("MAIN","WiFi not connected to ssid:%s",WIFI_SSID);
    connectWIFI();
  }else{
    LOGI("MAIN","WiFi connected to ssid:%s",WIFI_SSID);
  }

  client.setBufferSize(4096);

  if (WiFi.status() == WL_CONNECTED ){ 
    client.setKeepAlive(5);
    uint8_t timeout = 10;
    while (timeout && !client.connect(MQTT_DEVICENAME, MQTT_USER, MQTT_PASS)){
      toggleLed(LED_RED);
      timeout--;
      delay(1000);
    }

    if (client.connect(MQTT_DEVICENAME, MQTT_USER, MQTT_PASS)) {
      LOGI("MAIN","MQTT connected to server:%s, deviceName:%s",MQTT_USER,MQTT_DEVICENAME);
      switchLed(LED_RED,false);
      
      unsigned long now = millis();
      lastMqttUpdate = now+20000; // Delay MQTT Data update by 20 sec after cnx

      bool subRes;
      client.setCallback(callback);

      client.subscribe(TEMP_SETPOINT_SET_TOPIC);
      client.subscribe(TEMP_SETPOINT_STATE_TOPIC);

      client.subscribe(MODE_SET_TOPIC);
      client.subscribe(MODE_STATE_TOPIC);

      client.subscribe(CURRENT_TEMP_STATE_TOPIC);
      client.subscribe(CURRENT_TEMP_SET_TOPIC);
      
      client.subscribe(TEMP_DHW_STATE_TOPIC);
      client.subscribe(TEMP_DHW_SET_TOPIC);
      
      client.subscribe(STATE_DHW_SET_TOPIC);

      client.subscribe(CENTRAL_HEATING_STATE_TOPIC);
      client.subscribe(WATER_HEATING_STATE_TOPIC);
      
      client.subscribe(ENABLE_CHEATING_SET_TOPIC);
      client.subscribe(ENABLE_CHEATING_STATE_TOPIC);
      client.subscribe(ENABLE_WHEATING_SET_TOPIC);
      client.subscribe(ENABLE_WHEATING_STATE_TOPIC);

      client.subscribe(ENABLE_OT_LOG_STATE_TOPIC);
      client.subscribe(ENABLE_OT_LOG_SET_TOPIC);

      client.subscribe(MAX_MODULATION_LEVEL_STATE_TOPIC);
      client.subscribe(MAX_MODULATION_LEVEL_SET_TOPIC);

      client.subscribe(LBAND_TEMP_STATE_TOPIC);
      client.subscribe(LBAND_TEMP_SET_TOPIC);

      client.subscribe(HBAND_TEMP_STATE_TOPIC);
     
      client.subscribe(HBAND_TEMP_SET_TOPIC);
  
      client.subscribe(NOSP_OVERRIDE_TEMP_STATE_TOPIC);
      client.subscribe(NOSP_OVERRIDE_TEMP_SET_TOPIC);

      client.subscribe(TEMP_BOILER_STATE_TOPIC);

      client.subscribe(FLAME_STATUS_STATE_TOPIC);
      client.subscribe(FLAME_LEVEL_STATE_TOPIC);

      client.subscribe(TEMP_DHW_STATE_TOPIC);
      client.subscribe(TEMP_DHW_SET_TOPIC);

      client.subscribe(SETPOINT_OVERRIDE_SET_TOPIC);
      
      client.subscribe(INIT_DEFAULT_VALUES_TOPIC);

      LOGI("MAIN","Connected to MQTT");
     
    } else {
      LOGE("MAIN","NOT Connected to MQTT");
      switchLed(LED_RED,true);
    }
  }
  
  return;
}

void getBootReasonMessage(char *buffer, int bufferlength){
#if defined(ARDUINO_ARCH_ESP32)

    esp_reset_reason_t reset_reason = esp_reset_reason();

    switch (reset_reason){
    case ESP_RST_UNKNOWN:
      snprintf(buffer, bufferlength, "Reset reason can not be determined");
      break;
    case ESP_RST_POWERON:
      snprintf(buffer, bufferlength, "Reset due to power-on event");
      break;
    case ESP_RST_EXT:
      snprintf(buffer, bufferlength, "Reset by external pin (not applicable for ESP32)");
      break;
    case ESP_RST_SW:
      snprintf(buffer, bufferlength, "Software reset via esp_restart");
      break;
    case ESP_RST_PANIC:
      snprintf(buffer, bufferlength, "Software reset due to exception/panic");
      break;
    case ESP_RST_INT_WDT:
      snprintf(buffer, bufferlength, "Reset (software or hardware) due to interrupt watchdog");
      break;
    case ESP_RST_TASK_WDT:
      snprintf(buffer, bufferlength, "Reset due to task watchdog");
      break;
    case ESP_RST_WDT:
      snprintf(buffer, bufferlength, "Reset due to other watchdogs");
      break;
    case ESP_RST_DEEPSLEEP:
      snprintf(buffer, bufferlength, "Reset after exiting deep sleep mode");
      break;
    case ESP_RST_BROWNOUT:
      snprintf(buffer, bufferlength, "Brownout reset (software or hardware)");
      break;
    case ESP_RST_SDIO:
      snprintf(buffer, bufferlength, "Reset over SDIO");
      break;
    }

    if (reset_reason == ESP_RST_DEEPSLEEP){
      esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

      switch (wakeup_reason){
      case ESP_SLEEP_WAKEUP_UNDEFINED:
        snprintf(buffer, bufferlength, "In case of deep sleep: reset was not caused by exit from deep sleep");
        break;
      case ESP_SLEEP_WAKEUP_ALL:
        snprintf(buffer, bufferlength, "Not a wakeup cause: used to disable all wakeup sources with esp_sleep_disable_wakeup_source");
        break;
      case ESP_SLEEP_WAKEUP_EXT0:
        snprintf(buffer, bufferlength, "Wakeup caused by external signal using RTC_IO");
        break;
      case ESP_SLEEP_WAKEUP_EXT1:
        snprintf(buffer, bufferlength, "Wakeup caused by external signal using RTC_CNTL");
        break;
      case ESP_SLEEP_WAKEUP_TIMER:
        snprintf(buffer, bufferlength, "Wakeup caused by timer");
        break;
      case ESP_SLEEP_WAKEUP_TOUCHPAD:
        snprintf(buffer, bufferlength, "Wakeup caused by touchpad");
        break;
      case ESP_SLEEP_WAKEUP_ULP:
        snprintf(buffer, bufferlength, "Wakeup caused by ULP program");
        break;
      case ESP_SLEEP_WAKEUP_GPIO:
        snprintf(buffer, bufferlength, "Wakeup caused by GPIO (light sleep only)");
        break;
      case ESP_SLEEP_WAKEUP_UART:
        snprintf(buffer, bufferlength, "Wakeup caused by UART (light sleep only)");
        break;
      }
    }
    else{
      snprintf(buffer, bufferlength, "Unknown reset reason %d", reset_reason);
    }
#endif

#if defined(ARDUINO_ARCH_ESP8266)

    rst_info *resetInfo;

    resetInfo = ESP.getResetInfoPtr();

    switch (resetInfo->reason)
    {

    case REASON_DEFAULT_RST:
        snprintf(buffer, bufferlength, "Normal startup by power on");
        break;

    case REASON_WDT_RST:
        snprintf(buffer, bufferlength, "Hardware watch dog reset");
        break;

    case REASON_EXCEPTION_RST:
        snprintf(buffer, bufferlength, "Exception reset, GPIO status won't change");
        break;

    case REASON_SOFT_WDT_RST:
        snprintf(buffer, bufferlength, "Software watch dog reset, GPIO status won't change");
        break;

    case REASON_SOFT_RESTART:
        snprintf(buffer, bufferlength, "Software restart ,system_restart , GPIO status won't change");
        break;

    case REASON_DEEP_SLEEP_AWAKE:
        snprintf(buffer, bufferlength, "Wake up from deep-sleep");
        break;

    case REASON_EXT_SYS_RST:
        snprintf(buffer, bufferlength, "External system reset");
        break;

    default:
        snprintf(buffer, bufferlength, "Unknown reset cause %d", resetInfo->reason);
        break;
    };

#endif
}

void web_otcmd(AsyncWebServerRequest * request){

  int paramsNr = request->params();
  Serial.println(paramsNr);
 
  for(int i=0;i<paramsNr;i++){
 
    AsyncWebParameter* p = request->getParam(i);
    Serial.print("Param name: ");
    Serial.println(p->name());
    Serial.print("Param value: ");
    Serial.println(p->value());
    Serial.println("------");
  }
 
  request->send(200, "text/plain", "message received");

  AsyncWebParameter* p = request->getParam(1);
  int ii=p->value().toInt()*256;
  
  unsigned int data = ii ;
  unsigned long rqst = ot.buildRequest(OpenThermRequestType::READ,OpenThermMessageID::TrSet,data);
  unsigned long resp = ot.sendRequest(rqst);
  
  Serial.println(resp);
  //logOTRequest(resp);

}

void switchLed(LED_NOTIF led, bool newState){
  
  switch(led){
    case LED_BLUE :
      if (newState==true){
        digitalWrite(BLUE_LED_PIN,HIGH);
        led_b=true;
      }else{
        digitalWrite(BLUE_LED_PIN,LOW);
        led_b=false;
      }
      break;
    case LED_GREEN :
      if (newState==true){
        digitalWrite(GREEN_LED_PIN,HIGH);
        led_g=true;
      }else{
        digitalWrite(GREEN_LED_PIN,LOW);
        led_g=false;
      }
      break;
    case LED_RED :
      if (newState==true){
        digitalWrite(RED_LED_PIN,HIGH);
        led_r=true;
      }else{
        digitalWrite(RED_LED_PIN,LOW);
        led_r=false;
      }
      break;
    default :
    break;
  }
}

void toggleLed(LED_NOTIF led){

  switch(led){
    case LED_BLUE :
      if (led_b==true){
        led_b=false;
        digitalWrite(BLUE_LED_PIN,LOW);
      }else{
        led_b=true;
        digitalWrite(BLUE_LED_PIN,HIGH);
      }
      break;
    case LED_GREEN :
      if (led_g==true){
        led_g=false;
        digitalWrite(GREEN_LED_PIN,LOW);
      }else{
        led_g=true;
        digitalWrite(GREEN_LED_PIN,HIGH);
      }
      break;
    case LED_RED :
      if (led_r==true){
        led_r=false;
        digitalWrite(RED_LED_PIN,LOW);
      }else{
        led_r=true;
        digitalWrite(RED_LED_PIN,HIGH);
      }
      break;
    default :
    break;
  
  }
}

void setup(){

  Serial.begin(115200);
  delay(1000);

  
  pinMode(GREEN_LED_PIN,OUTPUT);
  pinMode(BLUE_LED_PIN,OUTPUT);
  pinMode(RED_LED_PIN,OUTPUT);
  
  switchLed(LED_RED,true);
  switchLed(LED_BLUE,true);
  
  if (!htu.begin(0x40)) {
    ESP_LOGE("MAIN","HTU31D sensor could not be found on I2C addr 0x40"); // DO NOT CHANGED
    
    while (1);
  }

  switchLed(LED_GREEN,true);
 
  WiFi.mode(WIFI_STA); //Optional
  WiFi.begin(WIFI_SSID, WIFI_KEY);

  LOGI("MAIN","Connecting to  WiFi SSID:%s",WIFI_SSID);
  
  while(WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(500);
    toggleLed(LED_RED);
  }

  Debug.begin(MQTT_DEVICENAME);      // start Remote debugger
  
  switchLed(LED_RED,false);

  Serial.setDebugOutput(true);
  
  LOGI("MAIN","Connected to the WiFi network %s",WIFI_SSID);
  LOGI("MAIN","Local IP %s",WiFi.localIP().toString().c_str());
        
   // ArduinoOTA.setHostname("monEsp"); // on donne une petit nom a notre module
   // ArduinoOTA.begin(); // initialisation de l'OTA

  connectMQTT();
  delay(1000); 

  // Home Automation Discovery MQTT
  MQTT_DiscoveryMsg_Climate();
  MQTT_DiscoveryMsg_Sensor_Flaming();
  MQTT_DiscoveryMsg_Sensor_FlameLevel();
  MQTT_DiscoveryMsg_Sensor_BoilerTemperature();
  MQTT_DiscoveryMsg_Sensor_BoilerReturnTemperature();
  MQTT_DiscoveryMsg_Sensor_BoilerTargetTemperature();
  MQTT_DiscoveryMsg_Sensor_dwhTemperature();
  MQTT_DiscoveryMsg_Sensor_IntegralError();

  MQTT_DiscoveryMsg_Sensor_InternalTemperature();
  MQTT_DiscoveryMsg_Sensor_InternalHumidity();
  
  MQTT_DiscoveryMsg_Number_MaxModulationLevel();
  MQTT_DiscoveryMsg_Number_LowBandTemperature();
  MQTT_DiscoveryMsg_Number_HighBandTemperature();
  MQTT_DiscoveryMsg_Number_NospTempOverride();
  MQTT_DiscoveryMsg_Number_dwh_temp();
  
  MQTT_DiscoveryMsg_Sensor_LeadingDevice();

  MQTT_DiscoveryMsg_Sensor_CentralHeating();
  MQTT_DiscoveryMsg_Sensor_WaterHeating();
  MQTT_DiscoveryMsg_Switch_EnableCentralHeating();
  MQTT_DiscoveryMsg_Switch_EnableWaterHeating();
  MQTT_DiscoveryMsg_Switch_EnableLog();
  MQTT_DiscoveryMsg_Text_Log();

  MQTT_DiscoveryMsg_Text_WIFI_SSID();
  MQTT_DiscoveryMsg_Text_WIFI_RSSI();
  MQTT_DiscoveryMsg_Text_IpAddr();
  MQTT_DiscoveryMsg_Text_MacAddr();
  MQTT_DiscoveryMsg_Text_PingAlive();
  MQTT_DiscoveryMsg_Button_InitDefValues();

  publishAvailable();

  #define BOOT_REASON_MESSAGE_SIZE 150
  char bootReasonMessage [BOOT_REASON_MESSAGE_SIZE];
  getBootReasonMessage(bootReasonMessage, BOOT_REASON_MESSAGE_SIZE);
  publishToTopicStr(bootReasonMessage,OT_LOG_STATE_TOPIC,"text",false); 

  unsigned long now = millis();
  startTime=now/1000;

  //lastMqttUpdate = now+10000; // Shift 10 sec in the future to avoid updatedata before subscribed data to be managed

  // Web Server Page Management
  server.on("/ot", web_otcmd);  // Chargement de la page d'accueil
  //server.onNotFound(handleNotFound);  // Chargement de la page "Not found"
  server.begin();  // Initialisation du serveur web

  ot.begin(handleInterrupt, processResponse); //OpenTherm Start
  
  esp_task_wdt_init(WDT_TIMEOUT, true);             // enable panic so ESP32 restarts
  esp_task_wdt_add(NULL);                           // add current thread to WDT watch
  
  switchLed(LED_GREEN,false);
  switchLed(LED_BLUE,true);

  ArduinoOTA.setHostname(MQTT_DEVICENAME);
  ArduinoOTA.setPassword(OTA_PASS);

  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      LOGW("MAIN","Start updating firmware" );
    })
    .onEnd([]() {
      LOGI("MAIN","\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      LOGI("MAIN","Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      LOGE("MAIN","Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR){ LOGE("MAIN","Auth Failed");}
      else if (error == OTA_BEGIN_ERROR) {LOGE("MAIN","Begin Failed");}
      else if (error == OTA_CONNECT_ERROR) {LOGE("MAIN","Connect Failed");}
      else if (error == OTA_RECEIVE_ERROR) {LOGE("MAIN","Receive Failed");}
      else if (error == OTA_END_ERROR){ LOGE("MAIN","End Failed");}
    });

  ArduinoOTA.begin();

}



void updateDataDiag(){
  // send MQTT Update
  
  int rssi = WiFi.RSSI();
  char * sRssi=(char*)malloc(16*(sizeof(char)));
  sprintf(sRssi,"%d",rssi);
  publishToTopicStr((char*)sRssi,WIFI_RSSI_STATE_TOPIC,"value",false); 
  free(sRssi);

  publishToTopicStr((char*)WIFI_SSID,WIFI_SSID_STATE_TOPIC,"value",false); 
  
  char* IpAddr=(char*)malloc(24*(sizeof(char)));
  IPAddress ip=WiFi.localIP();
  sprintf(IpAddr,"%d.%d.%d.%d",ip[0],ip[1],ip[2],ip[3]);
  publishToTopicStr((char*)IpAddr,IP_ADDR_STATE_TOPIC,"value",false); 
  free(IpAddr);

  MacAddr=WiFi.macAddress();
  publishToTopicStr((char*)MacAddr.c_str(),MAC_ADDR_STATE_TOPIC,"value",false); 
  
  pingAliveCount++;
  publishToTopicFloat(pingAliveCount,PING_ALIVE_STATE_TOPIC,"value",false);
  
}

void updateData(){
  
  publishToTopicFloat(op,TEMP_BOILER_TARGET_TEMP_STATE_TOPIC,"temp",false);
  publishToTopicFloat(boilerTemp,TEMP_BOILER_STATE_TOPIC,"temp",false);
  publishToTopicFloat(ierr,INTEGRAL_ERROR_STATE_TOPIC,"value",false);
  
  publishToTopicStr((char*)(isFlameOn ? "ON" : "OFF"),FLAME_STATUS_STATE_TOPIC,"value",false); // TODO Add condition on central heating
  publishToTopicFloat(flameLevel,FLAME_LEVEL_STATE_TOPIC,"value",false);
  
  publishToTopicFloat(boilerTemp,TEMP_BOILER_STATE_TOPIC,"temp",false);
  publishToTopicFloat(boilerReturnTemp,TEMP_BOILER_RETURN_STATE_TOPIC,"temp",false);
  publishToTopicStr((char*)(isCentralHeatingActive ? "ON" : "OFF"),CENTRAL_HEATING_STATE_TOPIC,"value",false); // TODO Add condition on central heating
  publishToTopicStr((char*)(isHotWaterActive ? "ON" : "OFF"),WATER_HEATING_STATE_TOPIC,"value",false); // TODO Add condition on central heating
  
  publishToTopicFloat(dwhTemp,ACTUAL_TEMP_DHW_STATE_TOPIC,"temp",false);
  
  publishToTopicFloat(temp.temperature,INTERNAL_TEMP_STATE_TOPIC,"temp",false);
  publishToTopicFloat(humidity.relative_humidity,INTERNAL_HUMIDITY_STATE_TOPIC,"value",false);

}

void processResponse(unsigned long response, OpenThermResponseStatus status) {
  
  char msg[64];
  switchLed(LED_GREEN,true);
  if (!ot.isValidResponse(response)) {
    LOGE("MAIN","OT Invalid reponse:0x%lx status:%d",response,status);
    
    if (bOtLogEnable==true){
      snprintf(msg,64,"Inv Resp:0x%lx,status:%d",response,status);
      publishToTopicStr(msg,OT_LOG_STATE_TOPIC,"text",false); 
    }
    
    switchLed(LED_GREEN,false);
    return;
  }
  
  if (bOtLogEnable==true){
    snprintf(msg,64,"Resp:0x%lx,status:%d",response,status);
    publishToTopicStr(msg,OT_LOG_STATE_TOPIC,"text",false); 
  }

  float retT;

  byte id = (response >> 16 & 0xFF);
  switch (id){
    case OpenThermMessageID::Status:
      boiler_status = response & 0xFF;
        
      isFlameOn=response & 0x8;
      isHotWaterActive=response & 0x4;
      isCentralHeatingActive=response & 0x2;
      break;

    case OpenThermMessageID::TSet:
      retT = (response & 0xFFFF) / 256.0;
      LOGI("MAIN","Boiler Response 0x%lx Set CentralHeating target temp:[%f]",response,retT);
      break;

    case OpenThermMessageID::Tboiler:
      retT = (response & 0xFFFF) / 256.0;
      boilerTemp=retT;
      LOGI("MAIN","Boiler Response: 0x%lx CentralHeating Current temp:[%f]",response,retT);
      break;

    case OpenThermMessageID::Tdhw:
      retT = (response & 0xFFFF) / 256.0;
      dwhTemp=retT;
      LOGI("MAIN","Boiler Response: 0x%lx Domestic Water Heating Current temp:[%f]",response,retT);
      break;

    case OpenThermMessageID::RelModLevel:
      flameLevel = (response & 0xFFFF) / 256.0;
      LOGI("MAIN","Boiler Response: 0x%lx Flame Level:[%f]",response,flameLevel);
      break;

    case OpenThermMessageID::MaxRelModLevelSetting:
      retT = (response & 0xFFFF) / 256.0;
      LOGI("MAIN","Boiler Response: 0x%lx Max Modulation Level:[%f]",response,retT);
      break;

     case OpenThermMessageID::Tret:
      retT = (response & 0xFFFF) / 256.0;
      boilerReturnTemp=retT;
      LOGI("MAIN","Boiler Response: 0x%lx Return water temperature:[%f]",response,retT);
      break;

    default:
      LOGI("MAIN","Boiler Response:[0x%lx] id:[%d]",response,id);
  }
  switchLed(LED_GREEN,false);
}

unsigned int buildRequest(byte req_idx){
  uint16_t status;
  byte id = requests[req_idx];
  switch (id){
    case OpenThermMessageID::Status:
      status = 0;
      if (bCentralHeatingEnable && bHeatingMode) status |= MASTER_STATUS_CH_ENABLED;
      if (bWaterHeatingEnable) status |= MASTER_STATUS_DHW_ENABLED;
        //if (CoolingEnabled) status |= MASTER_STATUS_COOLING_ENABLED;        // No cooling 
      status <<= 8;
      return ot.buildRequest(OpenThermMessageType::READ, OpenThermMessageID::Status, status);
    case OpenThermMessageID::TSet:
      return ot.buildRequest(OpenThermMessageType::WRITE, OpenThermMessageID::TSet, ((uint16_t)op) << 8);
    case OpenThermMessageID::TdhwSet:
      return ot.buildRequest(OpenThermMessageType::WRITE, OpenThermMessageID::TdhwSet, ((uint16_t)dwhTarget) << 8);
    case OpenThermMessageID::Tboiler:
      return ot.buildRequest(OpenThermMessageType::READ, OpenThermMessageID::Tboiler, 0);
    case OpenThermMessageID::Tdhw:
      return ot.buildRequest(OpenThermMessageType::READ, OpenThermMessageID::Tdhw, 0);
    case OpenThermMessageID::MaxRelModLevelSetting:
      return ot.buildRequest(OpenThermMessageType::WRITE, OpenThermMessageID::MaxRelModLevelSetting, ((uint16_t)MaxModLevel) << 8);
    case OpenThermMessageID::RelModLevel:
      return ot.buildRequest(OpenThermMessageType::READ, OpenThermMessageID::RelModLevel, 0);
    case OpenThermMessageID::Tret:
      return ot.buildRequest(OpenThermMessageType::READ, OpenThermMessageID::Tret, 0);
  }
  return 0;
}

void handleOpenTherm() {
  //if (ot.isReady()) { // <------------ to reactivate after test
    unsigned long now = millis();
    if (now - lastUpdate > statusUpdateInterval_ms) { // 1 Request every 1 sec
      lastUpdate = now;
      new_ts = millis();
      dt = (new_ts - ts) / 1000.0;
      ts = new_ts;
      op = pid(sp, t, t_last, ierr, dt);
      t_last=t;
      /* 
      if (now - lastPSet <= spOverrideTimeout_ms) {
        op = nosp_override;
      }
      */
      unsigned int request = buildRequest(req_idx);                 // Rotating the Request
      ot.sendRequestAync(request);
      LOGI("MAIN","Thermostat Request:[0x%lx]",request);
      if (bOtLogEnable==true){
        char msg[32];
        snprintf(msg,32,"Req:0x%lx",request);
        publishToTopicStr(msg,OT_LOG_STATE_TOPIC,"text",false); 
      }
      req_idx++;
    
      if (req_idx >= requests_count) {
        req_idx = 0;
      }
    }
    ot.process(); // Check for Reponse and process
  //}
}

void loop(){

// WARNING DO NOT PUT DELAY OR IT WILL NOT ENABLE MQTT TO CATCH SUBSCRIBE MESSAGE
// PLEASE MAKE SURE MQTT CLIENT ID IS UNIQUE OTHERWISE YOU WILL GET DISCONNECTION
  ArduinoOTA.handle();
  toggleLed(LED_BLUE);
  
  if (!client.connected()) {
    switchLed(LED_RED,true);
    LOGI("MAIN","MQTT reconnection reason client.state:%d",client.state());
    connectMQTT();
  }
  client.loop();                // MQTT Loop to process subscribre request
  esp_task_wdt_reset();         // Reset the Watchdog
  handleOpenTherm();            // Process Opentherm Response & Request

  unsigned long now = millis();
  if (now - lastMqttUpdate > UpdateMqttInterval_ms) {
    lastMqttUpdate = now;
    LogMasterParam();
    updateData();

    htu.getEvent(&humidity, &temp);// populate temp and humidity objects with fresh data
    LOGI("MAIN","Controller internal sensor temperature:[%.2f°C], humidity:[%.2f\%]",temp.temperature,humidity.relative_humidity);
    
  }

  if (now - lastUpdateDiag > UpdateDiagInterval_ms) {
    updateDataDiag();
    lastUpdateDiag = now;
  }
  Debug.handle();
  //ArduinoOTA.poll();
   
}