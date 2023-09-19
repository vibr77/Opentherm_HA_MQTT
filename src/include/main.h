
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
float flameLevel=0,boilerTemp=0,boilerReturnTemp=0,dwhTemp=0,dwhTarget=45;
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
float pingAliveCount=0;


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