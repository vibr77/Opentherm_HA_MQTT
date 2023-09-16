/*
__   _____ ___ ___        Author: Vincent BESSON
 \ \ / /_ _| _ ) _ \      Release: 0.34
  \ V / | || _ \   /      Date: 20230709
   \_/ |___|___/_|_\      Description: ESP32 Mini Home Assistant Opentherm Master Thermostat
                2023      Licence: Creative Commons
______________________

Release changelog:
  +20230910: Initial Commit
*/ 


#include <Arduino.h>
#include <WiFi.h>

#include "ESPAsyncWebServer/ESPAsyncWebServer.h"

#include <WiFiUdp.h>
//#include <ArduinoOTA.h>

//#include <RemoteDebug.h>

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


//RemoteDebug Debug;

OpenTherm ot(OT_IN_PIN, OT_OUT_PIN);
WiFiClient espClient;
PubSubClient client((const char*)MQTT_SERVER, MQTT_PORT,  espClient);
AsyncWebServer server(80); 

int status = WL_IDLE_STATUS;

void IRAM_ATTR handleInterrupt() {
    ot.handleInterrupt();
}

 


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

  //Serial.println("sp=" + String(sp) + " pv=" + String(pv) + " dt=" + String(dt) + " op=" + String(op) + " P=" + String(P) + " I=" + String(I));
  ESP_LOGI("MAIN","PID op:[%f] pv:[%f] dt:[%f] sp:[%f] P:[%f] I:[%f]",op,pv,dt,sp,P,I);
  return op;
}

String strMsgId[100] {
	"Status", // flag8 / flag8  Master and Slave Status flags.
	"TSet", // f8.8  Control setpoint  ie CH  water temperature setpoint (°C)
	"MConfigMMemberIDcode", // flag8 / u8  Master Configuration Flags /  Master MemberID Code
	"SConfigSMemberIDcode", // flag8 / u8  Slave Configuration Flags /  Slave MemberID Code
	"Command", // u8 / u8  Remote Command
	"ASFflags", // / OEM-fault-code  flag8 / u8  Application-specific fault flags and OEM fault code
	"RBPflags", // flag8 / flag8  Remote boiler parameter transfer-enable & read/write flags
	"CoolingControl", // f8.8  Cooling control signal (%)
	"TsetCH2", // f8.8  Control setpoint for 2e CH circuit (°C)
	"TrOverride", // f8.8  Remote override room setpoint
	"TSP", // u8 / u8  Number of Transparent-Slave-Parameters supported by slave
	"TSPindexTSPvalue", // u8 / u8  Index number / Value of referred-to transparent slave parameter.
	"FHBsize", // u8 / u8  Size of Fault-History-Buffer supported by slave
	"FHBindexFHBvalue", // u8 / u8  Index number / Value of referred-to fault-history buffer entry.
	"MaxRelModLevelSetting", // f8.8  Maximum relative modulation level setting (%)
	"MaxCapacityMinModLevel", // u8 / u8  Maximum boiler capacity (kW) / Minimum boiler modulation level(%)
	"TrSet", // f8.8  Room Setpoint (°C)
	"RelModLevel", // f8.8  Relative Modulation Level (%)
	"CHPressure", // f8.8  Water pressure in CH circuit  (bar)
	"DHWFlowRate", // f8.8  Water flow rate in DHW circuit. (litres/minute)
	"DayTime", // special / u8  Day of Week and Time of Day
	"Date", // u8 / u8  Calendar date
	"Year", // u16  Calendar year
	"TrSetCH2", // f8.8  Room Setpoint for 2nd CH circuit (°C)
	"Tr", // f8.8  Room temperature (°C)
	"Tboiler", // f8.8  Boiler flow water temperature (°C)
	"Tdhw", // f8.8  DHW temperature (°C)
	"Toutside", // f8.8  Outside temperature (°C)
	"Tret", // f8.8  Return water temperature (°C)
	"Tstorage", // f8.8  Solar storage temperature (°C)
	"Tcollector", // f8.8  Solar collector temperature (°C)
	"TflowCH2", // f8.8  Flow water temperature CH2 circuit (°C)
	"Tdhw2", // f8.8  Domestic hot water temperature 2 (°C)
	"Texhaust", // s16  Boiler exhaust temperature (°C)
	"TdhwSetUBTdhwSetLB", // s8 / s8  DHW setpoint upper & lower bounds for adjustment  (°C)
	"MaxTSetUBMaxTSetLB", // s8 / s8  Max CH water setpoint upper & lower bounds for adjustment  (°C)
	"HcratioUBHcratioLB", // s8 / s8  OTC heat curve ratio upper & lower bounds for adjustment
	"TdhwSet" // f8.8  DHW setpoint (°C)    (Remote parameter 1)
	"MaxTSet", // f8.8  Max CH water setpoint (°C)  (Remote parameters 2)
	"Hcratio", // f8.8  OTC heat curve ratio (°C)  (Remote parameter 3)
	"RemoteOverrideFunction" , // flag8 / -  Function of manual and program changes in master and remote room setpoint.
	"OEMDiagnosticCode" , // u16  OEM-specific diagnostic/service code
	"BurnerStarts", // u16  Number of starts burner
	"CHPumpStarts", // u16  Number of starts CH pump
	"DHWPumpValveStarts", // u16  Number of starts DHW pump/valve
	"DHWBurnerStarts", // u16  Number of starts burner during DHW mode
	"BurnerOperationHours", // u16  Number of hours that burner is in operation (i.e. flame on)
	"CHPumpOperationHours", // u16  Number of hours that CH pump has been running
	"DHWPumpValveOperationHours", // u16  Number of hours that DHW pump has been running or DHW valve has been opened
	"DHWBurnerOperationHours", // u16  Number of hours that burner is in operation during DHW mode
	"OpenThermVersionMaster", // f8.8  The implemented version of the OpenTherm Protocol Specification in the master.
	"OpenThermVersionSlave", // f8.8  The implemented version of the OpenTherm Protocol Specification in the slave.
	"MasterVersion", // u8 / u8  Master product version number and type
	"SlaveVersion", // u8 / u8  Slave product version number and type
};


void connectWIFI(){
  
  while (WiFi.status() != WL_CONNECTED) {                     // attempt to connect to Wifi network
    status = WiFi.begin(WIFI_SSID, WIFI_KEY);                 
    uint8_t timeout = 10;
    while (timeout && (WiFi.status() != WL_CONNECTED)) {       // wait 10 seconds for connection:
      timeout--;
      delay(1000);
    }
    if (WiFi.status()==WL_CONNECTED){
      ESP_LOGI("MAIN","WiFi connected to ssid:%s",WIFI_SSID);
    }
  } 
  return;
}

void connectMQTT(){

  if (WiFi.status() != WL_CONNECTED ){
    connectWIFI();
  }
  client.setBufferSize(4096);

  if (WiFi.status() == WL_CONNECTED ){ 
    
      if (client.connect(MQTT_DEVICENAME, MQTT_USER, MQTT_PASS)) {

        unsigned long now = millis();
        lastMqttUpdate = now+20000; // Delay MQTT Data update by 20 sec after cnx

        bool subRes;
        client.setCallback(callback);

        client.subscribe(TEMP_SETPOINT_SET_TOPIC.c_str());
        client.subscribe(TEMP_SETPOINT_STATE_TOPIC.c_str());

        client.subscribe(MODE_SET_TOPIC.c_str());
        client.subscribe(MODE_STATE_TOPIC.c_str());

        client.subscribe(CURRENT_TEMP_STATE_TOPIC.c_str());
        client.subscribe(CURRENT_TEMP_SET_TOPIC.c_str());
        
        client.subscribe(TEMP_DHW_STATE_TOPIC.c_str());
        client.subscribe(TEMP_DHW_SET_TOPIC.c_str());
        
        client.subscribe(STATE_DHW_SET_TOPIC.c_str());

        client.subscribe(CENTRAL_HEATING_STATE_TOPIC.c_str());
        client.subscribe(WATER_HEATING_STATE_TOPIC.c_str());
        
        client.subscribe(ENABLE_CHEATING_SET_TOPIC.c_str());
        client.subscribe(ENABLE_CHEATING_STATE_TOPIC.c_str());
        client.subscribe(ENABLE_WHEATING_SET_TOPIC.c_str());
        client.subscribe(ENABLE_WHEATING_STATE_TOPIC.c_str());

        client.subscribe(ENABLE_OT_LOG_STATE_TOPIC.c_str());
        client.subscribe(ENABLE_OT_LOG_SET_TOPIC.c_str());

        client.subscribe(MAX_MODULATION_LEVEL_STATE_TOPIC.c_str());
        client.subscribe(MAX_MODULATION_LEVEL_SET_TOPIC.c_str());

        client.subscribe(LBAND_TEMP_STATE_TOPIC.c_str());
        client.subscribe(LBAND_TEMP_SET_TOPIC.c_str());

        client.subscribe(HBAND_TEMP_STATE_TOPIC.c_str());
       
        client.subscribe(HBAND_TEMP_SET_TOPIC.c_str());
    
        client.subscribe(NOSP_OVERRIDE_TEMP_STATE_TOPIC.c_str());
        client.subscribe(NOSP_OVERRIDE_TEMP_SET_TOPIC.c_str());

        client.subscribe(TEMP_BOILER_STATE_TOPIC.c_str());

        client.subscribe(FLAME_STATUS_STATE_TOPIC.c_str());
        client.subscribe(FLAME_LEVEL_STATE_TOPIC.c_str());

        client.subscribe(TEMP_DHW_STATE_TOPIC.c_str());
        client.subscribe(TEMP_DHW_SET_TOPIC.c_str());

        //client.subscribe(SETPOINT_OVERRIDE_STATE_TOPIC.c_str());
        client.subscribe(SETPOINT_OVERRIDE_SET_TOPIC.c_str());

        ESP_LOGI("MAIN","Connected to MQTT");
       
      } else {
        ESP_LOGE("MAIN","NOT Connected to MQTT");
      }
    }
  
  return;
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

   /* 
    
    unsigned int data = 0xFFFF;
    unsigned long rqst = ot.buildRequest(OpenThermRequestType::READ,OpenThermMessageID::RelModLevel,data);
    unsigned long response = ot.sendRequest(rqst);
    Serial.println(response);
    logOTRequest(response);
    
    */

    AsyncWebParameter* p = request->getParam(1);
    int ii=p->value().toInt()*256;
    
    unsigned int data = ii ;
    unsigned long rqst = ot.buildRequest(OpenThermRequestType::READ,OpenThermMessageID::TrSet,data);
    unsigned long resp = ot.sendRequest(rqst);
    
    Serial.println(resp);
    logOTRequest(resp);

}

void logOTRequest(unsigned long response){
    
    OpenThermMessageID id = ot.getDataID(response);
    uint16_t data = ot.getUInt(response);
    float f = ot.getFloat(response);
    OpenThermMessageType mtype=ot.getMessageType(response);
    const char * mstr=ot.messageTypeToString(mtype);

    Serial.print(mstr);
    Serial.print(" ->" + strMsgId[id] + "=");
    Serial.println(data/256);
}

void setup(){

    Serial.begin(115200);
    delay(1000);

    //Debug.begin("monEsp");  

    WiFi.mode(WIFI_STA); //Optional
    WiFi.begin(WIFI_SSID, WIFI_KEY);
    Serial.println("\nConnecting");

    while(WiFi.status() != WL_CONNECTED){
        Serial.print(".");
        delay(500);
    }

    Serial.setDebugOutput(true);
    
    ESP_LOGI("MAIN","Connected to the WiFi network %s",WIFI_SSID);
    ESP_LOGI("MAIN","Local IP %s",WiFi.localIP().toString().c_str());
          
     ////ArduinoOTA.setHostname("monEsp"); // on donne une petit nom a notre module
     //ArduinoOTA.begin(); // initialisation de l'OTA

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
    
    MQTT_DiscoveryMsg_Number_MaxModulationLevel();
    MQTT_DiscoveryMsg_Number_LowBandTemperature();
    MQTT_DiscoveryMsg_Number_HighBandTemperature();
    MQTT_DiscoveryMsg_Number_NospTempOverride();
    MQTT_DiscoveryMsg_Number_dwh_temp();
    
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

    publishAvailable();
    //publishInitValue2();
    //publishInitValue5(19);

     unsigned long now = millis();
     startTime=now/1000;


    lastMqttUpdate = now+10000; // Shift 10 sec in the future to avoid updatedata before subscribed data to be managed

    // Web Server Page Management
     server.on("/ot", web_otcmd);  // Chargement de la page d'accueil
     //server.onNotFound(handleNotFound);  // Chargement de la page "Not found"
     server.begin();  // Initialisation du serveur web

    
    ot.begin(handleInterrupt, processResponse); //OpenTherm Start
}
void updateDataDiag(){
  // send MQTT Update
  
  int rssi = WiFi.RSSI();
  char * sRssi=(char*)malloc(16*(sizeof(char)));
  sprintf(sRssi,"%d",rssi);
  publishToTopicStr((char*)sRssi,WIFI_RSSI_STATE_TOPIC.c_str(),"value",false); 
  free(sRssi);

  publishToTopicStr((char*)WIFI_SSID,WIFI_SSID_STATE_TOPIC.c_str(),"value",false); 
  
  char* IpAddr=(char*)malloc(24*(sizeof(char)));
  IPAddress ip=WiFi.localIP();
  sprintf(IpAddr,"%d.%d.%d.%d",ip[0],ip[1],ip[2],ip[3]);
  publishToTopicStr((char*)IpAddr,IP_ADDR_STATE_TOPIC.c_str(),"value",false); 
  free(IpAddr);

  String MacAddr=WiFi.macAddress();
  publishToTopicStr((char*)MacAddr.c_str(),MAC_ADDR_STATE_TOPIC.c_str(),"value",false); 
  
  unsigned long now = millis()/1000;
  char * PingAlive=(char *)malloc(32*sizeof(char));
  sprintf(PingAlive,"%lu",now);
  publishToTopicStr(PingAlive,PING_ALIVE_STATE_TOPIC.c_str(),"value",false);
  
}
void updateData(){
  
  publishToTopicFloat(op,TEMP_BOILER_TARGET_TEMP_STATE_TOPIC.c_str(),"temp",false);
  publishToTopicFloat(boilerTemp,TEMP_BOILER_STATE_TOPIC.c_str(),"temp",false);
  publishToTopicFloat(ierr,INTEGRAL_ERROR_STATE_TOPIC.c_str(),"value",false);
  
  //publishToTopicStr((char*)(bHeatingMode ? "heat" : "off"),MODE_STATE_TOPIC.c_str(),"mode",false); // TODO Add condition on central heating
  
  publishToTopicStr((char*)(isFlameOn ? "ON" : "OFF"),FLAME_STATUS_STATE_TOPIC.c_str(),"value",false); // TODO Add condition on central heating
  publishToTopicFloat(flameLevel,FLAME_LEVEL_STATE_TOPIC.c_str(),"value",false);
  
  publishToTopicFloat(boilerTemp,TEMP_BOILER_STATE_TOPIC.c_str(),"temp",false);
  publishToTopicFloat(boilerReturnTemp,TEMP_BOILER_RETURN_STATE_TOPIC.c_str(),"temp",false);
  publishToTopicStr((char*)(isCentralHeatingActive ? "ON" : "OFF"),CENTRAL_HEATING_STATE_TOPIC.c_str(),"value",false); // TODO Add condition on central heating
  publishToTopicStr((char*)(isHotWaterActive ? "ON" : "OFF"),WATER_HEATING_STATE_TOPIC.c_str(),"value",false); // TODO Add condition on central heating
  
  publishToTopicFloat(dwhTemp,ACTUAL_TEMP_DHW_STATE_TOPIC.c_str(),"temp",false);
  
}

void processResponse(unsigned long response, OpenThermResponseStatus status) {
  if (!ot.isValidResponse(response)) {
      Serial.println("Invalid response: " + String(response, HEX) + ", status=" + String(ot.getLastResponseStatus()));
      return;
  }
 
  float retT;
  byte id = (response >> 16 & 0xFF);
  switch (id){
    case OpenThermMessageID::Status:
        boiler_status = response & 0xFF;
        
        isFlameOn=response & 0x8;
        isHotWaterActive=response & 0x4;
        isCentralHeatingActive=response & 0x2;

        ESP_LOGI("MAIN","Boiler status:[%s]",String(boiler_status, BIN).c_str());
        break;
    case OpenThermMessageID::TSet:
        retT = (response & 0xFFFF) / 256.0;
        ESP_LOGI("MAIN","Boiler Response Set CentralHeating target temp:[%f]",retT);
        break;
    case OpenThermMessageID::Tboiler:
        retT = (response & 0xFFFF) / 256.0;
        boilerTemp=retT;
        ESP_LOGI("MAIN","Boiler Response CentralHeating Current temp:[%f]",retT);
        break;
     case OpenThermMessageID::Tdhw:
        retT = (response & 0xFFFF) / 256.0;
        dwhTemp=retT;
        ESP_LOGI("MAIN","Boiler Response Domestic Water Heating Current temp:[%f]",retT);
        break;
    case OpenThermMessageID::RelModLevel:
        flameLevel = (response & 0xFFFF) / 256.0;
        ESP_LOGI("MAIN","Boiler Response Flame Level:[%f]",flameLevel);
        break;
    case OpenThermMessageID::MaxRelModLevelSetting:
        retT = (response & 0xFFFF) / 256.0;
        ESP_LOGI("MAIN","Boiler Response Max Modulation Level:[%f]",retT);
     case OpenThermMessageID::Tret:
        retT = (response & 0xFFFF) / 256.0;
        boilerReturnTemp=retT;
        ESP_LOGI("MAIN","Boiler Response Return water temperature:[%f]",retT);
    default:
        ESP_LOGI("MAIN","Boiler Response:[%s] id:[%s]",String(response, HEX).c_str(),String(id).c_str());
  }
}

unsigned int buildRequest(byte req_idx){
  uint16_t status;
  byte id = requests[req_idx];
  switch (id){
    case OpenThermMessageID::Status:
        status = 0;
        if (bCentralHeatingEnable && bHeatingMode) status |= MASTER_STATUS_CH_ENABLED;
        if (bWaterHeatingEnable) status |= MASTER_STATUS_DHW_ENABLED;
        //if (CoolingEnabled) status |= MASTER_STATUS_COOLING_ENABLED;
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
   /* if (now - lastPSet <= spOverrideTimeout_ms) {
      op = nosp_override;
    }*/
    unsigned int request = buildRequest(req_idx);  // Rotating the Request
    ot.sendRequestAync(request);
    ESP_LOGI("MAIN","Thermostat Request:[%s]",String(request, HEX).c_str());
    req_idx++;
  
    if (req_idx >= requests_count) {
      req_idx = 0;
    }
  }
  ot.process(); // Check for Reponse and process
  //}
}

void loop(){
  
  if (!client.connected()) {
    connectMQTT();
  }
 
  client.loop();               // MQTT Loop to process subscribre request

  handleOpenTherm();           // Process Opentherm Response & Request

  unsigned long now = millis();
  if (now - lastMqttUpdate > UpdateMqttInterval_ms) {
    lastMqttUpdate = now;
    LogMasterParam();
    updateData();
  }

  if (now - lastUpdateDiag > UpdateDiagInterval_ms) {
    updateDataDiag();
    lastUpdateDiag = now;
  }

}