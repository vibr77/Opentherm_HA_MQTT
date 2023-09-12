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
#include <ArduinoOTA.h>

#include <RemoteDebug.h>

#include <OpenTherm.h>
#include <PubSubClient.h>

#include "include/config.h"
#include "include/mqtt_com.h"

RemoteDebug Debug;

OpenTherm ot(OT_IN_PIN, OT_OUT_PIN);
WiFiClient espClient;
PubSubClient client((const char*)MQTT_SERVER, MQTT_PORT, callback, espClient);
AsyncWebServer server(80); 

void connectWIFI();
void connectMQTT();
void logOTRequest(unsigned long response);

int status = WL_IDLE_STATUS;


void IRAM_ATTR handleInterrupt() {
    ot.handleInterrupt();
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
      ESP_LOGI(TAG,"WiFi connected to ssid:%s",WIFI_SSID);
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
    if (!client.loop()) {
      
      client.disconnect();

      if (client.connect(MQTT_DEVICENAME, MQTT_USER, MQTT_PASS)) {
        client.subscribe(TEMP_SETPOINT_SET_TOPIC.c_str());
        client.subscribe(MODE_SET_TOPIC.c_str());
        client.subscribe(CURRENT_TEMP_SET_TOPIC.c_str());
        client.subscribe(TEMP_DHW_SET_TOPIC.c_str());
        client.subscribe(STATE_DHW_SET_TOPIC.c_str());

        Serial.println("Connected to MQTT");
      } else {
       Serial.println("Not connected to MQTT");
      }
    }
  }
  return;
}

void processRequest(unsigned long request, OpenThermResponseStatus status) {
    Serial.println("T" + String(request, HEX)); //master/thermostat request

    unsigned long response = 0;
    OpenThermMessageID id = ot.getDataID(request);
    uint16_t data = ot.getUInt(request);
    float f = ot.getFloat(request);
    OpenThermMessageType mtype=ot.getMessageType(request);
    const char * mstr=ot.messageTypeToString(mtype);

    Serial.print(mstr);
    Serial.print(" ->" + strMsgId[id] + "=");
    Serial.println(data/256);

    switch(id)
    {
      case OpenThermMessageID::Status:
      {
        uint8_t statusRequest = data >> 8;
        uint8_t chEnable = statusRequest & 0x1;
        uint8_t dhwEnable = statusRequest & 0x2;
        data &= 0xFF00;
        //data |= 0x01; //fault indication
        if (chEnable) data |= 0x02; //CH active
        if (dhwEnable) data |= 0x04; //DHW active
        if (chEnable || dhwEnable) data |= 0x08; //flame on
        //data |= 0x10; //cooling active
        //data |= 0x20; //CH2 active
        //data |= 0x40; //diagnostic/service event
        //data |= 0x80; //electricity production on

        response = ot.buildResponse(OpenThermMessageType::READ_ACK, id, data);
        break;
      }
      case OpenThermMessageID::TSet:
      {
        response = ot.buildResponse(OpenThermMessageType::WRITE_ACK, id, data);
        break;
      }
      case OpenThermMessageID::Tboiler:
      {
        data = ot.temperatureToData(45);
        response = ot.buildResponse(OpenThermMessageType::READ_ACK, id, data);
        break;
      }
      default:
      {
        //build UNKNOWN-DATAID response
        response = ot.buildResponse(OpenThermMessageType::UNKNOWN_DATA_ID, ot.getDataID(request), 0);   
      }
    }
    Serial.println("B" + String(response, HEX)); //slave/boiler response

    //send response
    delay(20); //20..400ms, usually 100ms
    ot.sendResponse(response);
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

   /* unsigned int data = 0xFFFF;
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

void setup()
{
    Serial.begin(115200);
    delay(1000);

    Debug.begin("monEsp");  


    WiFi.mode(WIFI_STA); //Optional
    WiFi.begin(WIFI_SSID, WIFI_KEY);
    Serial.println("\nConnecting");

    while(WiFi.status() != WL_CONNECTED){
        Serial.print(".");
        delay(500);
    }

    Serial.println("\nConnected to the WiFi network");
    Serial.print("Local ESP32 IP: ");
    Serial.println(WiFi.localIP());


     ////ArduinoOTA.setHostname("monEsp"); // on donne une petit nom a notre module
     ArduinoOTA.begin(); // initialisation de l'OTA

    connectMQTT();
    delay(1000);

    // Home Automation Discovery MQTT
    MQTT_DiscoveryMsg_Climate();
    MQTT_DiscoveryMsg_Sensor_Flaming();
    MQTT_DiscoveryMsg_Sensor_BoilerTemperature();
    MQTT_DiscoveryMsg_Number_Modulation_Level();
    MQTT_DiscoveryMsg_Sensor_CentralHeating();
    MQTT_DiscoveryMsg_Sensor_WaterHeating();
    MQTT_DiscoveryMsg_Switch_EnableCentralHeating();
    MQTT_DiscoveryMsg_Switch_EnableWaterHeating();
    MQTT_DiscoveryMsg_Switch_EnableLog();
    MQTT_DiscoveryMsg_Text_Log();

    publishInitValue1();
    publishInitValue2();
    publishInitValue3("ONLINE");
    publishInitValue4();
    //publishInitValue6();

    // Web Server Page Management
     server.on("/ot", web_otcmd);  // Chargement de la page d'accueil
     //server.onNotFound(handleNotFound);  // Chargement de la page "Not found"
     server.begin();  // Initialisation du serveur web


    //OpenTherm Management
    ot.begin(handleInterrupt);
}


void loop(){
  
  //ot.process();
  ArduinoOTA.handle(); 
  Debug.handle();

  if (!client.connected()) {
    connectMQTT();
  }
  client.loop();
  

  bool enableCentralHeating = true;
	bool enableHotWater = true;
	bool enableCooling = false;
	
  unsigned long response = ot.setBoilerStatus(enableCentralHeating, enableHotWater, enableCooling);
	OpenThermResponseStatus responseStatus = ot.getLastResponseStatus();
	
  if (responseStatus == OpenThermResponseStatus::SUCCESS) {		
		Serial.println("Central Heating: " + 
            String(ot.isCentralHeatingActive(response) ? "on" : "off"));
		Serial.println("Hot Water: " + 
            String(ot.isHotWaterActive(response) ? "on" : "off"));
		Serial.println("Flame: " + 
            String(ot.isFlameOn(response) ? "on" : "off"));
	}
	if (responseStatus == OpenThermResponseStatus::NONE) {
		Serial.println("Error: OpenTherm is not initialized");
	}
	else if (responseStatus == OpenThermResponseStatus::INVALID) {
		Serial.println("Error: Invalid response " + String(response, HEX));
	}
	else if (responseStatus == OpenThermResponseStatus::TIMEOUT) {
		Serial.println("Error: Response timeout");
	}
	//Set Boiler Temperature to 64 degrees C
	//ot.setBoilerTemperature(64);
	//Get Boiler Temperature
	float temperature = ot.getBoilerTemperature();
	Serial.println("Boiler temperature is " + String(temperature) + " degrees C");	
	Serial.println();
	delay(1000);

  unsigned int data = 0xFFFF;
  unsigned long rqst = ot.buildRequest(OpenThermRequestType::READ,OpenThermMessageID::Tboiler,data);
  unsigned long resp = ot.sendRequest(rqst);
    Serial.println(resp);
    logOTRequest(resp);

  
  unsigned int dataw = 0xFFFF;
  unsigned long rqstw = ot.buildRequest(OpenThermRequestType::READ,OpenThermMessageID::Tdhw,dataw);
  unsigned long respw = ot.sendRequest(rqstw);
  OpenThermResponseStatus responseStatus2 = ot.getLastResponseStatus();
  Serial.print("-");
  Serial.println(responseStatus2);

    Serial.println(respw);
    logOTRequest(respw);

 rqstw = ot.buildRequest(OpenThermRequestType::READ,OpenThermMessageID::Tret,dataw);
 respw = ot.sendRequest(rqstw);
 responseStatus2 = ot.getLastResponseStatus();
 Serial.println(responseStatus2);
 logOTRequest(respw);

 rqstw = ot.buildRequest(OpenThermRequestType::READ,OpenThermMessageID::RelModLevel,dataw);
respw = ot.sendRequest(rqstw);
 responseStatus2 = ot.getLastResponseStatus();
 Serial.println(responseStatus2);
 logOTRequest(respw);

  rqstw = ot.buildRequest(OpenThermRequestType::READ,OpenThermMessageID::CHPressure,dataw);
respw = ot.sendRequest(rqstw);
responseStatus2 = ot.getLastResponseStatus();
 Serial.println(responseStatus2);
 logOTRequest(respw);
delay(100);
dataw=256*24;
unsigned int datap =  (unsigned int)(24 * 256);
rqstw = ot.buildRequest(OpenThermRequestType::WRITE_DATA,OpenThermMessageID::TSet,datap);
//rqstw=268524032;
respw = ot.sendRequest(rqstw);
responseStatus2 = ot.getLastResponseStatus();
 Serial.println(responseStatus2);
 logOTRequest(respw);
 // publishInitValue3("ONLINE");
 // delay(4000);
 // publishInitValue3("OFFLINE");
}