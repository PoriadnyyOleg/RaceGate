/* TODO
-) список участников
-) контороль фальстарта?
-) Результаты всех участников -> в табличку ?+  
в массив
//  GoogleSheet                     //OR RTC and OFFLINE?
   +распределенно                         +минимальная аппаратная привязка  
    открыто                               -завязано на одну конкретную таблицу
                                          -не совместимо с  ESPAsyncWebServer?
GET :                                           
POST: row=0&session=0&laps= 1:32.476 | 1:33.742 | 1:38.039   
-) добавить webSocket состояния заезда?
правильный формат - с запятой     0:1:32,342

добавить вход Online-Offline    //переключатель
добавить выход WiFiIsOK  // при включенни в онлайн режиме
сделать TableManager c интерфейсом: 
        send (message) // полжить в список отправки сообщение (интернет может пропадать) 
        getDrivers ()  // получить


*/
//#include <ArduinoOTA.h>
#include <Arduino.h>
#ifdef ESP32
  #include <WiFi.h>
  #include <AsyncTCP.h>
#else
  #include <ESP8266WiFi.h>
  #include <ESPAsyncTCP.h>
#endif

#include <ESPAsyncWebServer.h>
#define CONFIG_ASYNC_TCP_USE_WDT 0

#define DEBUG
#ifdef DEBUG
#define DEBUG_PRINT(x) Serial.println(x)
#else
#define DEBUG_PRINT(x)
#endif


//#include "FileSaveResult.h"
#include "RoadRunner.h"
#include "svetofor.h"
#include "WiFimodeLight.h"
#include  "GSheetManager.h"

//TaskHandle_t Task0;

WiFimodeLight modeWiFi(33); //для индикации режима WiFi
bool onlineMode=true; 
bool useSDcard=false;

GSheetManager webTable ;
String sessionResult ="";

// REPLACE WITH YOUR NETWORK CREDENTIALS
/*
const char* ssid = "D37";
const char* password = "20111760";
*/
/*
const char* ssid = "Gate";
const char* password = "12345678";
*/
const char* offlineAPid = "OfflineGatePoint";
const char* ssid = "GatePoint";
const char* password = "12345679";


 /*
struct tm tmstruct;
if(!getLocalTime(&tmstruct)){
  DEBUG_PRINT("Failed to obtain time");
  return;
};
char timeOfDay[10];
strftime(timeOfDay,10, "%R", &tmstruct);
  year = tmstruct.tm_year-100;
  month = tmstruct.tm_mon+1;
  _newDay = tmstruct.tm_mday;
String formattedDate;
int month, year, day;
char uptime[100];
*/
//String dirPath = "/LapResults/";
//SPIClass * hspi = NULL;
//File myFile;


AsyncWebServer server(80);
uint8_t slalomGatePin = 35;
uint8_t gatePin = 25;
const int switchModePin = 34 ;  


bool isSlalom=false;

Gate gate(gatePin);
Gate falseStartGate(slalomGatePin);

Svetofor svetofor(0);  // для адресной ленты пин указан в .Н, используется в сетапе/ цыфра - костыль
RoadRunner* nowRunner;
RoadRunner* nowWaiter; 
//RoadRunner  fakeRunner("f", 0);
int countLap=0;
int finLap=0;
String nameDriverWait, nameDriverRun;
String laps; //результаты 
RoadRunner::typeLap svetoforDriverState, prevSvetoforDriverState;
RoadRunner * tableDrivers[50];
int numberDrivers =0; 
  int bright = 20;


String getSelectTable(){
  DEBUG_PRINT("Формируем SelectTable");
  String result="";
  for (int i=0 ; i< numberDrivers;i++){
     result+="<option value=";
     result+=String(i);
     result+=">";
     result+=tableDrivers[i]->nameDriver;
     result+="</option>";
  }
  DEBUG_PRINT(result);
  return result;
}
String getDriverTable(){
  String result="";
  DEBUG_PRINT("Формируем DriverTable");
  for (int i=0 ; i< numberDrivers;i++){
    
     result+=String(i+1);
     result+=" : ";
     result+=tableDrivers[i]->nameDriver;
     result+=" \n ";
  }
  DEBUG_PRINT(result);
  return result;
}

String getResultTable(){
  String result="";
  DEBUG_PRINT("Формируем ResultTable");
  for (int i=0 ; i< numberDrivers;i++){
     result+=String(i+1);
     result+=" : ";
     result+=tableDrivers[i]->getLaps();
     //result+=" \n ";
  }
  DEBUG_PRINT(result);
  return result;
}




String processor(const String& var)
{
  if(var == "TEMPLATE_WAITER_DRIVER"){
    return F (String(nameDriverWait).c_str());
  } else if ((var == "TEMPLATE_NOW_DRIVER") && (nowRunner)){
    return F(String(nameDriverRun).c_str());
  } else if (var == "TEMPLATE_LAPS"){
    return F(String(finLap).c_str());
  } else if(var == "SELECT_DRIVER"){
    return F (String(getSelectTable()).c_str());
  } 
  return String();
}

String processor2(const String& var)
{
  if(var == "TEMPLATE_RESULTS"){
    return F (getResultTable().c_str());
  } 
  return String();
}

String processor3(const String& var)
{
  if(var == "DRIVER_TABLE"){
    return F (getDriverTable().c_str());
  } else if (var == "TEMPLATE_LAPS"){
    return F(String(finLap).c_str());
  } else if (var == "TEMPLATE_BRIGHT"){
    return F(String(bright).c_str());
  } 
  return String();
}
//ошибки персонала
String processorErrors(const String& var)
{
  if(var == "ERROR_NUL_LAP"){
    return F (getDriverTable().c_str());
  } else if (var == "TEMPLATE_LAPS"){
    return F(String(finLap).c_str());
  }
  return String();
}


const char* NAME_INPUT = "name";
const char* CAR_INPUT = "car";
const char* LAP_INPUT = "laps";
const char* DRIVER = "driver";
const char* BRIGHT_INPUT = "bright";


const char index_html[] PROGMEM = R"rawliteral(
<html><head><meta charset="utf-8" name="viewport" content="width=device-width, initial-scale=1">
<link rel="icon" href="data:,">
<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}
.button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;
text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}
.button2 {background-color: #555555;}
.button3 {background-color: #FF0000;font-size: 10px;}</style></head>
<body><h1>Гоночные Врата</h1>
 <form action="/get">
    <p><select size="10" name="driver" required>
    <option disabled>Кто следующий?</option>
    %SELECT_DRIVER%   
   </select></p>
   <p><input type="submit" value="Готовится"></p>
  </form>
<p>Ожидает: %TEMPLATE_WAITER_DRIVER%</p>
<p>Едет   : %TEMPLATE_NOW_DRIVER%</p>
 
<p><a href="/runRabbit"><button class="button">СТАРТ!</button></a></p>   
<br>
<p><a href="/stop"><button class="button button3">СТОП!</button></a></p>
<br>
<p><a href="/howRun"><button class="button button2">Результаты</button></a></p>
<p><a href="/registertable"><button class="button button2">Регистрация</button></a></p>

<script>
function runButton() {
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/runRabbit", true);
  xhr.send();
  }
 function stopButton() {
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/stop", true);
  xhr.send();
  }
 function readyButton() {
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/stop", true);
  xhr.send();
  }

</script>

</body></html>)rawliteral";


const char result_html[] PROGMEM = R"rawliteral(
<html><head><meta charset="utf-8" name="viewport" content="width=device-width, initial-scale=1">
<link rel="icon" href="data:,">
<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}
.button { background-color: #555555; border: none; color: white; padding: 16px 60px;
text-decoration: none; font-size: 20px; margin: 2px; cursor: pointer;}
</style></head>
<body><h1>Гоночные Врата</h1>
 <pre>%TEMPLATE_RESULTS%</pre>
<br>
<p><a href="/"><button class="button button">На главный</button></a></p>
</body></html>)rawliteral";

const char error_html[] PROGMEM = R"rawliteral(
<html><head><meta charset="utf-8" name="viewport" content="width=device-width, initial-scale=1">
<link rel="icon" href="data:,">
<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}
.button { background-color: #555555; border: none; color: white; padding: 16px 60px;
text-decoration: none; font-size: 20px; margin: 2px; cursor: pointer;}
</style></head>
<body><h1>Гоночные Врата</h1>
 <pre>%ERROR_NUL_LAP%</pre>
<br>
<p><a href="/"><button class="button button">На главный</button></a></p>
</body></html>)rawliteral";



///РЕГИСТРАЦИЯ
const char register_html[] PROGMEM = R"rawliteral(
<html><head><meta charset="utf-8" name="viewport" content="width=device-width, initial-scale=1">
<link rel="icon" href="data:,">
<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}
.button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;
text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}
.button2 {background-color: #555555;}
.button3 {background-color: #FF0000;font-size: 20px;}</style></head>
<body><h1>Регистрация</h1>
<h5>Сначала установите количество кругов </h5>
<p>Кругов : %TEMPLATE_LAPS%</p>
   <form action="/setLaps">
    Кругов: <input type="text" name="laps">
    <input type="submit" value="Установить">
  </form>
  <h5>Потом прочитать таблицу регистрации </h5>
<p><a href="/regFromTable"><button class="button button3">Из таблицы</button></a></p>

 <h5>Кто не прошел предварительную регистрацию: </h5>
 <form action="/set">
    ФИО: <input type="text" name="name"><br><br>
    Авто: <input type="text" name="car"><br><br>
    <input type="submit" value="Зарегистрировать">
  </form>
<br>
 <pre> %DRIVER_TABLE%</pre>
<br>


<p><a href="/"><button class="button button2">На главную</button></a></p>
<br>

<br>
<p>Яркость 0..255 : %TEMPLATE_BRIGHT%</p>
   <form action="/setBright">
     <input type="text" name="bright">
    <input type="submit" value="Задать">
  </form>
</body></html>)rawliteral";


void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}
