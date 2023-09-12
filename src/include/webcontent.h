
const  char  index_html[]  = R"rawliteral("
<script type="text/javascript">
  var ipAddr = "`IP_ADDR`";
  var readToken = "`READ_TOKEN`";
  var channelId = "`CHANNEL_ID`";
</script>
<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="UTF-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1.0" />
    <meta http-equiv="X-UA-Compatible" content="ie=edge" />
    <title>OpenTherm Gateway</title>

    <link href="../styles.css" rel="stylesheet" />

    <script src="https://cdn.jsdelivr.net/npm/apexcharts"></script>
    <script src="../otgw-core.js" async></script>

    <style>
      .wrapr {
        padding-top: 20px;
        padding-left: 10px;
        padding-bottom: 20px;
        padding-right: 10px;
        background: #fff;
        border: 1px solid #ddd;
        box-shadow: 0 22px 35px -16px rgba(0, 0, 0, 0.1);
        max-width: 650px;
        margin: 35px auto;
      }

      #chart-line {
        position: relative;
        margin-top: -40px;
      }

      .center {
        display: flex;
        align-items: center;
        justify-content: center;
      }

      .label-column {
        white-space: nowrap;
        width: auto;
      }

      /* switch */

      .switch {
        position: relative;
        display: inline-block;
        width: 36px;
        height: 18px;
      }

      .switch input {
        opacity: 0;
        width: 0;
        height: 0;
      }

      .slider {
        position: absolute;
        cursor: pointer;
        top: 0;
        left: 0;
        right: 0;
        bottom: 0;
        background-color: #ccc;
        -webkit-transition: 0.4s;
        transition: 0.4s;
      }

      .slider:before {
        position: absolute;
        content: "";
        height: 12px;
        width: 12px;
        left: 4px;
        bottom: 3px;
        background-color: white;
        -webkit-transition: 0.4s;
        transition: 0.4s;
      }

      input:checked + .slider {
        background-color: #2196f3;
      }

      input:focus + .slider {
        box-shadow: 0 0 1px #2196f3;
      }

      input:checked + .slider:before {
        -webkit-transform: translateX(16px);
        -ms-transform: translateX(16px);
        transform: translateX(16px);
      }

      /* Rounded sliders */
      .slider.round {
        border-radius: 24px;
      }

      .slider.round:before {
        border-radius: 50%;
      }
    </style>
  </head>

  <body>
    <div class="wrapr">
      <table style="width: 100%" id="idd">
        <tr>
          <td class="label-column">From</td>
          <td style="width: auto">
            <input type="datetime-local" id="date-from" />
          </td>
          <td style="width: 100%"></td>
        </tr>
        <tr>
          <td class="label-column">To</td>
          <td style="width: auto">
            <input type="datetime-local" id="date-to" />
          </td>
          <td style="width: 100%"></td>
        </tr>
        <tr>
          <td class="label-column">Heating target, &#x00B0;C</td>
          <td style="width: auto">
            <div id="heatingTargetLabel" style="float: right">--</div>
          </td>
          <td class="label-column">Heating current, &#x00B0;C</td>
          <td style="width: auto">
            <div id="heatingCurrentLabel">--</div>
          </td>
        </tr>
        <tr>
          <td class="label-column">Hot water target, &#x00B0;C</td>
          <td style="width: auto">
            <div id="dhwTargetLabel" style="float: right">--</div>
          </td>
          <td class="label-column">Hot water current, &#x00B0;C</td>
          <td style="width: auto">
            <div id="dhwCurrentLabel">--</div>
          </td>
        </tr>
        <tr>
          <td class="label-column">Heating enable</td>
          <td style="width: auto">
            <div style="float: right">
              <label class="switch">
                <input type="checkbox" id="heatingEnableInput" disabled />
                <span class="slider round"></span>
              </label>
            </div>
          </td>
          <td class="label-column">Hot water enable</td>
          <td style="width: auto">
            <label class="switch">
              <input type="checkbox" id="dhwEnableInput" disabled />
              <span class="slider round"></span>
            </label>
          </td>
        </tr>
      </table>
    </div>
    <div class="wrapr center" style="height: 500px" id="chart-container">
      <div id="waiting-indicator" style="text-align: center; margin: 0 auto">LOADING...</div>
      <div id="chart-flame"></div>
      <div id="chart-line2"></div>
      <div id="chart-line"></div>
    </div>

    <div class="wrapr center">
      <textarea style="height: 150px; width: 100%; min-width: 100%" id="commands-log"></textarea>
    </div>
  </body>
</html>
)rawliteral";

const char css[]  = R"rawliteral(
* {
  font-family: Arial;
}

body {
  height: 100vh;
  background: #f9f9f9;
}

#chart,
.chart-box {
  padding-top: 20px;
  padding-left: 10px;
  background: #fff;
  border: 1px solid #ddd;
  box-shadow: 0 22px 35px -16px rgba(0, 0, 0, 0.1);
}

select.flat-select {
  -moz-appearance: none;
  -webkit-appearance: none;
  appearance: none;
  background: #008ffb
    url("data:image/svg+xml;utf8,<svg xmlns='http://www.w3.org/2000/svg' width='60px' height='60px'><polyline fill='white' points='46.139,15.518 25.166,36.49 4.193,15.519'/></svg>")
    no-repeat scroll right 2px top 9px / 16px 16px;
  border: 0 none;
  border-radius: 3px;
  color: #fff;
  font-family: arial, tahoma;
  font-size: 16px;
  font-weight: bold;
  outline: 0 none;
  height: 33px;
  padding: 5px 20px 5px 10px;
  text-align: center;
  text-indent: 0.01px;
  text-overflow: "";
  text-shadow: 0 -1px 0 rgba(0, 0, 0, 0.25);
  transition: all 0.3s ease 0s;
  width: auto;
  -webkit-transition: 0.3s ease all;
  -moz-transition: 0.3s ease all;
  -ms-transition: 0.3s ease all;
  -o-transition: 0.3s ease all;
  transition: 0.3s ease all;
}
select.flat-select:focus,
select.flat-select:hover {
  border: 0;
  outline: 0;
}

.apexcharts-canvas {
  margin: 0 auto;
}

#html {
  display: none;
}
)rawliteral";

const char js[]  = R"rawliteral(
/*
    // this function will generate output in this format
    // data = [
        [timestamp, 23],
        [timestamp, 33],
        [timestamp, 12]
        ...
    ]
  */

let tempChart;
let flameChart;

init();

Date.prototype.addHours = function(h) {
  this.setTime(this.getTime() + (h*60*60*1000));
  return this;
}

function init() {
  let varsInitDone = true;
  if (ipAddr === "`IP_ADDR`" || ipAddr === "") {
    alert("Gateway ip address is unknown");
    varsInitDone = false;
  }

  if (varsInitDone && (readToken === "`READ_TOKEN`" || readToken === "")) {
    alert("Please specify READ API Token for thingspeak service in gateway firmware");
    varsInitDone = false;
  }

  if (!varsInitDone) {
    document.querySelector("#waiting-indicator").innerText = "CONFIGURATION INVALID";
    return;
  }

  window.addEventListener("load", function () {
    setTimeout(function () {
      initUi(varsInitDone);
    }, 0);
  });
}


function getData(dateFrom, dateTo) {
  // "https://api.thingspeak.com/channels/<channel-id>/feeds.json?api_key=<read-api-key>&offset=<tz-offset>&start=<start-datetime>&end=<end-datetime>&round=0"
  const tzOffset = new Date().getTimezoneOffset() / -60;
  const params = `api_key=${readToken}&offset=${tzOffset}&start=${formatDate(dateFrom, '%20')}&end=${formatDate(dateTo, '%20')}&round=0`;
  const url = `https://api.thingspeak.com/channels/${channelId}/feeds.json?${params}`;

  var xhr = new XMLHttpRequest();
  xhr.open("GET", url, false);
  xhr.send(null);

  const responseObj = JSON.parse(xhr.responseText);

  data = [];
  dataFlame = [];

  responseObj.feeds.forEach((element) => {
    const timeStamp = new Date(element.created_at).getTime();
    data.push([timeStamp, element.field1]);
    dataFlame.push([timeStamp, element.field4]);
  });

  var ret = {
    tempData: data,
    flameData: dataFlame,
  };

  return ret;
}

function updateChart(data) {
  tempChart.updateSeries([
    {
      data: data.tempData,
    },
  ]);

  flameChart.updateSeries([
    {
      data: data.flameData,
    },
  ]);
}

function reloadAndUpdate() {
  const dateFrom = Date.parse(document.querySelector("#date-from").value);
  const dateTo = Date.parse(document.querySelector("#date-to").value);

  const data = getData(new Date(dateFrom), new Date(dateTo));
  updateChart(data);
}

function initUi(initOk) {
  document.querySelector("#waiting-indicator").style.setProperty("display", "none");
  document.querySelector("#chart-container").classList.remove("center");
  
  document.querySelector("#date-from").value = formatDate(new Date().addHours(-2), 'T');
  document.querySelector("#date-to").value = formatDate(new Date(), 'T');

  document.querySelector("#date-from").addEventListener("change", (event) => {
    reloadAndUpdate();
  });

  document.querySelector("#date-to").addEventListener("change", (event) => {
    reloadAndUpdate();
  });

  document.querySelector("#heatingEnableInput").addEventListener("change", (event) => {
    try {
      const enable = document.querySelector("#heatingEnableInput").checked;
      const url = `http://${ipAddr}/heating-${enable}`;
      console.log(url);

      var xhr = new XMLHttpRequest();
      xhr.open("GET", url, false);
      xhr.send(null);
    } catch (error) {
      console.log(`Failed to send request: ${error}`);
    }
  });

  document.querySelector("#dhwEnableInput").addEventListener("change", (event) => {
    const enable = document.querySelector("#dhwEnableInput").checked;
    const url = `http://${ipAddr}/dhw-${enable}`;
    console.log(url);

    var xhr = new XMLHttpRequest();
    xhr.open("GET", url, false);
    xhr.send(null);
  });

  let options = {
    series: [
      {
        data: [],
      },
    ],
    chart: {
      id: "chart2",
      type: "area",
      height: 350,
      group: "heating",
      toolbar: {
        autoSelected: "pan",
        show: true,
      },
    },
    stroke: {
      width: 3,
    },
    dataLabels: {
      enabled: false,
    },
    fill: {
      type: "gradient",
      gradient: {
        shadeIntensity: 1,
        inverseColors: false,
        opacityFrom: 0.5,
        opacityTo: 0,
        stops: [0, 90, 100],
      },
    },
    markers: {
      size: 0,
    },
    xaxis: {
      type: "datetime",
      labels: {
        datetimeUTC: false,
      }      
    },
    yaxis: {
      labels: {
        minWidth: 40,
      },
    },
  };
  tempChart = new ApexCharts(document.querySelector("#chart-line2"), options);
  tempChart.render();

  options = {
    series: [
      {
        data: [],
      },
    ],
    chart: {
      id: "chart-flame",
      type: "area",
      height: 75,
      group: "heating",
      toolbar: {
        autoSelected: "pan",
        show: false,
      },
    },
    stroke: {
      width: 3,
    },
    dataLabels: {
      enabled: false,
    },
    fill: {
      type: "gradient",
      gradient: {
        shadeIntensity: 1,
        inverseColors: false,
        opacityFrom: 0.5,
        opacityTo: 0,
        stops: [0, 90, 100],
      },
    },
    markers: {
      size: 0,
    },
    xaxis: {
      type: "datetime",
      labels: {
        show: false,
      },
    },
    yaxis: {
      show: false,
      labels: {
        minWidth: 40,
      },
    },
    tooltip: {
      enabled: false,
    },
  };

  flameChart = new ApexCharts(document.querySelector("#chart-flame"), options);
  flameChart.render();

  reloadAndUpdate();
}

//var gateway = `ws://${window.location.hostname}/ws`;
var gateway = `ws://${ipAddr}/ws`;
var websocket;
function initWebSocket() {
  console.log("Trying to open a WebSocket connection...");
  websocket = new WebSocket(gateway);
  websocket.onopen = onOpen;
  websocket.onclose = onClose;
  websocket.onmessage = onMessage; // <-- add this line
}
function onOpen(event) {
  console.log("Connection opened");
}

function onClose(event) {
  console.log("Connection closed");
  setTimeout(initWebSocket, 2000);
}

function formatDate(date, s = ' ') {
  var d = date,
    month = "" + (d.getMonth() + 1),
    day = "" + d.getDate(),
    year = d.getFullYear(),
    hour = "" + d.getHours(),
    min = "" + d.getMinutes(),
    sec = "" + d.getSeconds();

  if (month.length < 2) month = "0" + month;
  if (day.length < 2) day = "0" + day;
  if (hour.length < 2) hour = "0" + hour;
  if (min.length < 2) min = "0" + min;
  if (sec.length < 2) sec = "0" + sec;

  return `${year}-${month}-${day}${s}${hour}:${min}:${sec}`;
}

function onMessage(event) {
  const msgData = event.data;
  console.log(`onMessage ${msgData}`);

  const msgKind = msgData.slice(0, 2);
  if (msgKind == "B:") {
    document.getElementById("heatingCurrentLabel").innerText = msgData.slice(2);
  } else if (msgKind == "D:") {
    document.getElementById("dhwCurrentLabel").innerText = msgData.slice(2);
  } else if (msgKind == "F:") {
    document.getElementById("dhwTargetLabel").innerText = msgData.slice(2);
  } else if (msgKind == "G:") {
    document.getElementById("heatingTargetLabel").innerText = msgData.slice(2);
  } else {
    const numberData = msgData.slice(1);

    var text = document.getElementById("commands-log");
    var date = new Date();

    const int = parseInt(Number(`0x${numberData}`), 10);
    const data = int & (~(1<<31));
    const msgType = data >> 28;
    const dataId = (data >> 16) & 0xff;
    const dataValue = data & 65535;

    if (msgType == 0 && dataId == 0)
    {
      document.getElementById("heatingEnableInput").checked = ((dataValue & (1<<8)) != 0);
      document.getElementById("dhwEnableInput").checked = ((dataValue & (1<<9)) != 0);
    }

    const msgTypeStr = OpenThermMessageType[msgType];
    const dataIdStr = OpenThermMessageID[dataId];

    var log = text.value;
    log = log.substring(log.length-100000);
    text.value = log + `${formatDate(date)}: ${int} [msgType: ${msgType} (${msgTypeStr}); dataId: ${dataId} (${dataIdStr}); dataValue: ${dataValue}]\r\n`;
    text.scrollTop = text.scrollHeight;
  }
}

window.addEventListener("load", onLoad);

function onLoad(event) {
  initWebSocket();
}

var OpenThermMessageType = [];
/*  Master to Slave */
OpenThermMessageType[0] = "READ_DATA";
OpenThermMessageType[1] = "WRITE_DATA";
OpenThermMessageType[2] = "INVALID_DATA";
OpenThermMessageType[3] = "RESERVED";
/* Slave to Master */
OpenThermMessageType[4] = "READ_ACK";
OpenThermMessageType[5] = "WRITE_ACK";
OpenThermMessageType[6] = "DATA_INVALID";
OpenThermMessageType[7] = "UNKNOWN_DATA_ID";

var OpenThermMessageID = [];
OpenThermMessageID[0] = "Status"; // flag8 / flag8  Master and Slave Status flags.
OpenThermMessageID[1] = "TSet"; // f8.8  Control setpoint  ie CH  water temperature setpoint (°C)
OpenThermMessageID[2] = "MConfigMMemberIDcode"; // flag8 / u8  Master Configuration Flags /  Master MemberID Code
OpenThermMessageID[3] = "SConfigSMemberIDcode"; // flag8 / u8  Slave Configuration Flags /  Slave MemberID Code
OpenThermMessageID[4] = "Command"; // u8 / u8  Remote Command
OpenThermMessageID[5] = "ASFflags"; // / OEM-fault-code  flag8 / u8  Application-specific fault flags and OEM fault code
OpenThermMessageID[6] = "RBPflags"; // flag8 / flag8  Remote boiler parameter transfer-enable & read/write flags
OpenThermMessageID[7] = "CoolingControl"; // f8.8  Cooling control signal (%)
OpenThermMessageID[8] = "TsetCH2"; // f8.8  Control setpoint for 2e CH circuit (°C)
OpenThermMessageID[9] = "TrOverride"; // f8.8  Remote override room setpoint
OpenThermMessageID[10] = "TSP"; // u8 / u8  Number of Transparent-Slave-Parameters supported by slave
OpenThermMessageID[11] = "TSPindexTSPvalue"; // u8 / u8  Index number / Value of referred-to transparent slave parameter.
OpenThermMessageID[12] = "FHBsize"; // u8 / u8  Size of Fault-History-Buffer supported by slave
OpenThermMessageID[13] = "FHBindexFHBvalue"; // u8 / u8  Index number / Value of referred-to fault-history buffer entry.
OpenThermMessageID[14] = "MaxRelModLevelSetting"; // f8.8  Maximum relative modulation level setting (%)
OpenThermMessageID[15] = "MaxCapacityMinModLevel"; // u8 / u8  Maximum boiler capacity (kW) / Minimum boiler modulation level(%)
OpenThermMessageID[16] = "TrSet"; // f8.8  Room Setpoint (°C)
OpenThermMessageID[17] = "RelModLevel"; // f8.8  Relative Modulation Level (%)
OpenThermMessageID[18] = "CHPressure"; // f8.8  Water pressure in CH circuit  (bar)
OpenThermMessageID[19] = "DHWFlowRate"; // f8.8  Water flow rate in DHW circuit. (litres/minute)
OpenThermMessageID[20] = "DayTime"; // special / u8  Day of Week and Time of Day
OpenThermMessageID[21] = "Date"; // u8 / u8  Calendar date
OpenThermMessageID[22] = "Year"; // u16  Calendar year
OpenThermMessageID[23] = "TrSetCH2"; // f8.8  Room Setpoint for 2nd CH circuit (°C)
OpenThermMessageID[24] = "Tr,"; // f8.8  Room temperature (°C)
OpenThermMessageID[25] = "Tboiler"; // f8.8  Boiler flow water temperature (°C)
OpenThermMessageID[26] = "Tdhw"; // f8.8  DHW temperature (°C)
OpenThermMessageID[27] = "Toutside"; // f8.8  Outside temperature (°C)
OpenThermMessageID[28] = "Tret"; // f8.8  Return water temperature (°C)
OpenThermMessageID[29] = "Tstorage"; // f8.8  Solar storage temperature (°C)
OpenThermMessageID[30] = "Tcollector"; // f8.8  Solar collector temperature (°C)
OpenThermMessageID[31] = "TflowCH2"; // f8.8  Flow water temperature CH2 circuit (°C)
OpenThermMessageID[32] = "Tdhw2"; // f8.8  Domestic hot water temperature 2 (°C)
OpenThermMessageID[33] = "Texhaust"; // s16  Boiler exhaust temperature (°C)
OpenThermMessageID[48] = "TdhwSetUBTdhwSetLB"; //= 48, // s8 / s8  DHW setpoint upper & lower bounds for adjustment  (°C)
OpenThermMessageID[49] = "MaxTSetUBMaxTSetLB"; // s8 / s8  Max CH water setpoint upper & lower bounds for adjustment  (°C)
OpenThermMessageID[50] = "HcratioUBHcratioLB"; // s8 / s8  OTC heat curve ratio upper & lower bounds for adjustment
OpenThermMessageID[56] = "TdhwSet"; //= 56, // f8.8  DHW setpoint (°C)    (Remote parameter 1)
OpenThermMessageID[57] = "MaxTSet"; // f8.8  Max CH water setpoint (°C)  (Remote parameters 2)
OpenThermMessageID[58] = "Hcratio"; // f8.8  OTC heat curve ratio (°C)  (Remote parameter 3)
OpenThermMessageID[100] = "RemoteOverrideFunction"; //= 100, // flag8 / -  Function of manual and program changes in master and remote room setpoint.
OpenThermMessageID[115] = "OEMDiagnosticCode"; //= 115, // u16  OEM-specific diagnostic/service code
OpenThermMessageID[116] = "BurnerStarts"; // u16  Number of starts burner
OpenThermMessageID[117] = "CHPumpStarts"; // u16  Number of starts CH pump
OpenThermMessageID[118] = "DHWPumpValveStarts"; // u16  Number of starts DHW pump/valve
OpenThermMessageID[119] = "DHWBurnerStarts"; // u16  Number of starts burner during DHW mode
OpenThermMessageID[120] = "BurnerOperationHours"; // u16  Number of hours that burner is in operation (i.e. flame on)
OpenThermMessageID[121] = "CHPumpOperationHours"; // u16  Number of hours that CH pump has been running
OpenThermMessageID[122] = "DHWPumpValveOperationHours"; // u16  Number of hours that DHW pump has been running or DHW valve has been opened
OpenThermMessageID[123] = "DHWBurnerOperationHours"; // u16  Number of hours that burner is in operation during DHW mode
OpenThermMessageID[124] = "OpenThermVersionMaster"; // f8.8  The implemented version of the OpenTherm Protocol Specification in the master.
OpenThermMessageID[125] = "OpenThermVersionSlave"; // f8.8  The implemented version of the OpenTherm Protocol Specification in the slave.
OpenThermMessageID[126] = "MasterVersion"; // u8 / u8  Master product version number and type
OpenThermMessageID[127] = "SlaveVersion"; // u8 / u8  Slave product version number and type
)rawliteral";
