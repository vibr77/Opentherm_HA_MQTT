# Opentherm Thermostat implementation for Home Assistant with MQTT

## Scope of the project

***WORK IN PROGRESS***

The purpose of this project is to replace my Netatmo (OT Modulating & Valve) setup with custom Opentherm thermostat and zigbee valve.
My Boiler is a Saunier Duval (eBUS) with an Opentherm interface with eBUS ([link to one of many provide](https://www.cvkoopjes.nl/thermostaten/toebehoren/awb-ebus-opentherm-module-.html)).

The hardware used is: 
---
- Master interface from DIYLESS
- ESP32 MINI D1
- Opentherm <-> EBUS interface
- MOES TRV Valve

The software stack:
---

My target setup is 

BOILER <------> ESP32 MINI WITH MASTER INTERFACE <-------> HOME ASSISTANT <-----> ZIGBEE TRV VALVE

The ESP32 will not have a dedicated & embedded temperature sensor. The thermostat controlling the boiler will get temperature and target temperature (setpoint) from the TRV Valve (a dedicated Nodered automation is planned). 

If a room has a current temperature that is lower to the setpoint, then the thermostat will get temperature and set point of the Valve. If multiple valve have gap, the thermostat will get the valve with the higher gap.


##  

### Parameters





