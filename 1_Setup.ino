#define WDT_TIMEOUT 30
#include <esp_task_wdt.h>
//#include <rtc_wdt.h>

void setup() {
  Serial.begin(115200);
//rtc_wdt_protect_off();
//rtc_wdt_disable();

 // disableCore0WDT();
 // disableCore1WDT();

  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(svetofor.leds, LED_NUM).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(bright);


/*
onlineMode
switchModePin
*/
 pinMode(switchModePin, INPUT);
 onlineMode = digitalRead(switchModePin);



if (onlineMode)  { 
   Serial.println("onlineMode");
  WiFi.mode(WIFI_STA);
 // WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.setHostname("TimeGatePLC2");
  WiFi.begin(ssid, password);
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi Failed!");
    modeWiFi.error(); // яростно моргать лампой
       WiFi.disconnect();
    WiFi.reconnect();
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
   // return;
   } else {
  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  modeWiFi.online();
//RTC synchronize
  long timezone = 4;
  byte daysavetime = 0; // не требуется переход на летнее время
  configTime(3600 * timezone, daysavetime * 3600, "time.nist.gov", "0.pool.ntp.org", "1.pool.ntp.org"); //
  struct tm tmstruct ;
  delay(2000);
  tmstruct.tm_year = 0;
  getLocalTime(&tmstruct, 5000);
  String ip=WiFi.localIP().toString();
   Serial.println("ip send in web:");
  webTable.sendIP(ip);
  }
} else {
    Serial.println("OFFLine Mode");
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  modeWiFi.offline();
}
  //ArduinoOTA.begin();


  // Send web page with input fields to client
  
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){          // базовый экран 
   request->send(200, "text/html",index_html, processor); 
  });

  server.on("/runRabbit", HTTP_GET, [](AsyncWebServerRequest *request){ // нажата кнопка Начала заезда
    if (!nowRunner) if (nowWaiter){
    nowRunner=nowWaiter;
    gate.attach(nowRunner);
    if (isSlalom){
      falseStartGate.attach(nowRunner);
    }
    nowRunner->startSession();
    svetofor.startRace();
    nameDriverRun=nowRunner->nameDriver;
    nameDriverWait="";
    }
    request->redirect("/");
    //request->send_P(200, "text/html",index_html, processor); 
    DEBUG_PRINT("Судья дал отмашку старта");
  });

  server.on("/registertable", HTTP_GET, [](AsyncWebServerRequest *request){
  
    DEBUG_PRINT("Открыта страница регистрации");
    if (onlineMode)  { 
    if (finLap==0) {
       // if (webTable.getRaceType()==0) {
        finLap=webTable.getFinishLap();
         DEBUG_PRINT("Race!!");
        /*} else {
          isSlalom=true;
          finLap=1;
           DEBUG_PRINT("Slalom!!");
        }*/

    }    
    }
    request->send(200, "text/html",register_html, processor3); 
  });


  server.on("/stop", HTTP_GET, [](AsyncWebServerRequest *request){         //преждевременная остановка заезда. результаты тоже запишем 
    svetofor.stopRace();
    request->redirect("/");
   // request->send_P(200, "text/html",index_html, processor); 
    DEBUG_PRINT("Гонка остановлена судьей");
        if (nowRunner){
          DEBUG_PRINT("Записали бегуна");  
          String laps = nowRunner->getLaps();
          sessionResult=nowRunner->getSessionResult();
          if (onlineMode) webTable.addSendList(sessionResult); 
         // writeMessage(SD, dirPath.c_str(), nameDriverRun, laps  );
          nameDriverRun="";
          nowRunner=NULL;
        };
          gate.attach(NULL);
          falseStartGate.attach(NULL);
  });
  
    server.on("/howRun", HTTP_GET, [](AsyncWebServerRequest *request){    // посмотреть промежуточные результаты (чтобы было)  - вернем экран с кругами и кнопкой стоп, назад
    if (nowRunner){
     laps = nowRunner->getLaps();
    } else {
      laps = "еще нет результатов"; 
    };
    DEBUG_PRINT(laps);
    request->send(200, "text/html",result_html, processor2); 
  });

  server.on("/get", HTTP_GET, [](AsyncWebServerRequest *request){         //преждевременная остановка заезда. результаты тоже запишем 
    if (request->hasParam(DRIVER)) {
      int numDriver=0;
      numDriver = (request->getParam(DRIVER)->value().toInt());
      DEBUG_PRINT("numDriver");
      DEBUG_PRINT(numDriver);
      if (numberDrivers>=numDriver)
      nowWaiter=tableDrivers[numDriver];
    }
      nameDriverWait=nowWaiter->nameDriver;
    request->redirect("/");
    //request->send_P(200, "text/html",index_html, processor); 
    DEBUG_PRINT("Выбор пилота");
  });

  // Send a GET request to <ESP_IP>/get?input1=<inputMessage>
  server.on("/setLaps", HTTP_GET, [] (AsyncWebServerRequest *request) {
   String inputParam;
   if (request->hasParam(LAP_INPUT)) {
      finLap = (request->getParam(LAP_INPUT)->value().toInt());
      inputParam = LAP_INPUT;
      DEBUG_PRINT(finLap);
    }
      request->send(200, "text/html",register_html, processor3); 
  });

  server.on("/setBright", HTTP_GET, [] (AsyncWebServerRequest *request) {
 
   if (request->hasParam(BRIGHT_INPUT)) {
      bright = (request->getParam(BRIGHT_INPUT)->value().toInt());
      if (bright>=0 & bright<255){
      FastLED.setBrightness(bright);
      }
      DEBUG_PRINT(bright);
    }
      request->send(200, "text/html",register_html, processor3); 
  });


server.on("/set", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String name, car;
    if (finLap!=0){
    // GET input1 value on <ESP_IP>/get?input1=<inputMessage>
    if (request->hasParam(NAME_INPUT)) {
      name = request->getParam(NAME_INPUT)->value();
       DEBUG_PRINT(name);
    }
    // GET input2 value on <ESP_IP>/get?input2=<inputMessage>
    if (request->hasParam(CAR_INPUT)) {
      
      car = request->getParam(CAR_INPUT)->value();
      DEBUG_PRINT(car);
    }
    //&
    
     tableDrivers[numberDrivers]= webTable.manualReg(name, car, finLap, onlineMode );
    DEBUG_PRINT("Зарегистрирован");
       numberDrivers++;
      request->redirect("/registertable");
   // request->send_P(200, "text/html",register_html, processor3); 
    } else {
      request->send(418, "text/plain", "Please SET LAPS");
    }

    });


server.on("/regFromTable", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputParam;
    //get finLap from Table
    if (onlineMode) { webTable.setFinishLap(finLap);  

     //DEBUG_PRINT(vTaskList());
    if (finLap!=0){
    while (webTable.hasNextRunner()>0){
    tableDrivers[numberDrivers]=webTable.getNextUnit();
    DEBUG_PRINT("Зарегистрирован из таблицы");
    DEBUG_PRINT(tableDrivers[numberDrivers]->nameDriver);
       numberDrivers++;
    }
    } else request->send(418, "text/plain", "OK");
    
   //  DEBUG_PRINT(vTaskList());
      request->redirect("/registertable");
    } else {
      request->send(418, "text/plain", "OFFLINE MODE - use manual input Drivers");
    }

    });


  server.onNotFound(notFound);
  server.begin();
  /*
  if (useSDcard){
   if (!SD.begin()) {
    Serial.println("Card Mount Failed");
  //  sendTextTelegramm( "Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");
   // sendTextTelegramm( "No SD card attached");
    return;
  }
  Serial.print("SD Card Type: ");
  if (cardType == CARD_MMC) {
    Serial.println("MMC");
  } else if (cardType == CARD_SD) {
    Serial.println("SDSC");
  } else if (cardType == CARD_SDHC) {
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }/*
  xTaskCreatePinnedToCore(
    TaskCode0,   /* Task function. 
    "IDLE0",     /* name of task. 
    10000,       /* Stack size of task *
    NULL,        /* parameter of the task *
    1,           /* priority of the task *
    &Task0,      /* Task handle to keep track of created task *
    0);          /* pin task to core 0 */
//}
   
  
 
    DEBUG_PRINT("endSetup");
}

/*
void TaskCode0( void * pvParameters ) {
  while (1)
  {
   // google_loop();
    vTaskDelay(100);
  }
}
*/