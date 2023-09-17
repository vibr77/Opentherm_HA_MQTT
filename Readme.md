# Opentherm Thermostat implementation for Home Assistant with MQTT

## Scope of the project

***WORK IN PROGRESS***

The purpose of this project is to replace my Netatmo (OT Modulating & Valve) setup with custom Opentherm thermostat and zigbee valve.
My Boiler is a Saunier Duval (eBUS) with an Opentherm interface with eBUS ([link to one of many provide](https://www.cvkoopjes.nl/thermostaten/toebehoren/awb-ebus-opentherm-module-.html)).

The hardware used is: 
---
* Master interface from DIYLESS
* ESP32 MINI D1
* Opentherm <-> EBUS interface
* MOES TRV Valve

The software stack:
---

My target setup is 

BOILER <------> ESP32 MINI WITH MASTER INTERFACE <-------> HOME ASSISTANT <-----> ZIGBEE TRV VALVE

The ESP32 will not have a dedicated & embedded temperature sensor. The thermostat controlling the boiler will get temperature and target temperature (setpoint) from the TRV Valve (a dedicated Nodered automation is planned). 

If a room has a current temperature that is lower to the setpoint, then the thermostat will get temperature and set point of the Valve. If multiple valve have gap, the thermostat will get the valve with the higher gap.


##  

### Home Assistant exposed entities & parameters

Control Sensors, 
---

Control
---
* Climate: Boiler Thermostat

Sensor
---
* Sensor: Boiler integral error
* Sensor: Boiler water return temperature
* Sensor: Boiler target temperature
* Sensor: Boiler current temperature
* Sensor: Boiler Flame (boolean)
* sensor: Boiler Flame level (modulation)
* Sensor: Central Heating (boolean)
* Sensor: Domestic Water Heating (boolean)
* Sensor: Domestic Water temeprature

Configuration:
---
* Switch: Enable Central Heating
* Switch: Enable Domestic Water Heating
* Number: Domestic Water Temperature
* Number: Max Modulation Level
* Number: High bound temperature
* Number: Low bound temperature
* Number: No setpoint temperature override


Diagnostic:
---
* Switch: Enable OT Log
* Text: Openther log
* Text: WiFi IP Addr
* Text: WiFi Mac Addr
* Text: WiFi SSID
* Text: WiFi RSSI
* Text: Ping Alive

#### Sensor: Boiler integral error  - PID 
([Wikipedia link to Proportional-Integral-Derivative]https://en.wikipedia.org/wiki/Proportional–integral–derivative_controller)
Proportional-Integral-Derivative (PID) control algorithm is implemented to determine the best efficient boiler temperature to reach the set-point temperature (sp) from a giving thermostat temprature (p).
The integral in a PID controller is the sum of the instantaneous error over time and gives the accumulated offset that should have been corrected previously.

#### Sensor: Boiler water return temperature
Represent of the temperature of water returning in the circuit of heating the house.

#### Sensor: Boiler target temperature








