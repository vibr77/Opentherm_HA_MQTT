

#include "config.h"

void connectMQTT();
void MQTT_DiscoveryMsg_Climate();
void MQTT_DiscoveryMsg_Sensor_Flaming();
void MQTT_DiscoveryMsg_Sensor_BoilerTemperature();
void MQTT_DiscoveryMsg_Sensor_WaterHeating();
void MQTT_DiscoveryMsg_Sensor_CentralHeating();
void MQTT_DiscoveryMsg_Number_Modulation_Level();
void MQTT_DiscoveryMsg_Switch_EnableWaterHeating();
void MQTT_DiscoveryMsg_Switch_EnableCentralHeating();
void MQTT_DiscoveryMsg_Switch_EnableLog();
void MQTT_DiscoveryMsg_Text_Log();


void callback(char* topic, byte* payload, unsigned int length);
void publishInitValue1();
void publishInitValue2();
void publishInitValue3(const char * val);
void publishInitValue4();
void publishInitValue5(float tp);
void publishInitValue6();
