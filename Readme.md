# Opentherm Thermostat implementation for Home Assistant with MQTT

## Scope of the project

The purpose of this project is to replace my Netatmo (OT Modulating & Valve) setup with custom Opentherm thermostat and zigbee valve.
My Boiler is a Saunier Duval (eBUS) with an Opentherm interface with eBUS ([link to one of many provide](https://www.cvkoopjes.nl/thermostaten/toebehoren/awb-ebus-opentherm-module-.html)).

The hardware used is: 
---
- Master interface from DIYLESS
- ESP32 MINI D1
- Opentherm <-> EBUS interface
- MOES TRV Valve

My target setup is 

BOILER <------> ESP32 MINI WITH MASTER INTERFACE <-------> HOME ASSISTANT <-----> ZIGBEE TRV VALVE







