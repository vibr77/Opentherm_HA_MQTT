

#include "config.h"


void LogMasterParam();
void connectMQTT();
void MQTT_DiscoveryMsg_Climate();
void MQTT_DiscoveryMsg_Sensor_Flaming();
void MQTT_DiscoveryMsg_Sensor_FlameLevel();

void MQTT_DiscoveryMsg_Sensor_BoilerTemperature();
void MQTT_DiscoveryMsg_Sensor_BoilerReturnTemperature();
void MQTT_DiscoveryMsg_Sensor_BoilerTargetTemperature();
void MQTT_DiscoveryMsg_Sensor_IntegralError();
void MQTT_DiscoveryMsg_Sensor_dwhTemperature();
void MQTT_DiscoveryMsg_Sensor_WaterHeating();
void MQTT_DiscoveryMsg_Sensor_CentralHeating();
void MQTT_DiscoveryMsg_Sensor_LeadingDevice();
void MQTT_DiscoveryMsg_Number_MaxModulationLevel();
void MQTT_DiscoveryMsg_Number_LowBandTemperature();
void MQTT_DiscoveryMsg_Number_HighBandTemperature();
void MQTT_DiscoveryMsg_Number_NospTempOverride();
void MQTT_DiscoveryMsg_Number_dwh_temp();


void MQTT_DiscoveryMsg_Switch_EnableWaterHeating();
void MQTT_DiscoveryMsg_Switch_EnableCentralHeating();
void MQTT_DiscoveryMsg_Switch_EnableLog();

void MQTT_DiscoveryMsg_Text_Log();

void MQTT_DiscoveryMsg_Text_WIFI_SSID();
void MQTT_DiscoveryMsg_Text_WIFI_RSSI();
void MQTT_DiscoveryMsg_Text_IpAddr();
void MQTT_DiscoveryMsg_Text_MacAddr();
void MQTT_DiscoveryMsg_Text_PingAlive();

void MQTT_DiscoveryMsg_Button_InitDefValues();



void callback(char* topic, byte* payload, unsigned int length);
bool isValidNumber(char  payload[]);

float getPayloadFloatValue(const char * key,char * payload);
char * getPayloadCharValue(const char * key, char * payload);

bool publishToTopicFloat(float value,const char *topic,const char * key,bool retain=false);
bool publishToTopicStr(char * value,const char *topic,const char * key,bool retain=false);
void publishAvailable();

void publishInitializationValues();
