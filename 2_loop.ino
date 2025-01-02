
void loop() {

 //ArduinoOTA.handle();
//rtc_wdt_feed();
//esp_task_wdt_reset();
  if (onlineMode){
    unsigned long currentMillis = millis();
  // if WiFi is down, try reconnecting every CHECK_WIFI_TIME seconds
   if ( (WiFi.status() != WL_CONNECTED) && (currentMillis - previousMillis >=5000)){
    Serial.println("Reconnecting to WiFi...");
    svetofor.wifiError();
    WiFi.disconnect();
    WiFi.reconnect();
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    previousMillis = currentMillis;
     
   } else {
    //DEBUG_PRINT("WL_CONNECTED or timer test reconnect");
       if (WiFi.status() == WL_CONNECTED) {
          svetofor.setLastState();
      //    Serial.print("IP Address: ");
      //    Serial.println(WiFi.localIP());
       }
       
   }; 

   if ((WiFi.status() == WL_CONNECTED)&& (currentMillis - previousMillis >=10000)){
     googleloop();
     previousMillis = currentMillis;
      //DEBUG_PRINT("пин слалома"); 
     //DEBUG_PRINT( digitalRead(slalomGatePin));
   };
    
  };
  
 // if  (digitalRead(slalomGatePin)) DEBUG_PRINT("flstartPin enable");

  countLap=0;
   delay(4);//allow the cpu to switch to other tasks
   svetoforDriverState=gate.checkGate();
    if (nowRunner){
         countLap=nowRunner->getLap();
    };
   
     if (svetoforDriverState==RoadRunner::typeLap::toRace){
        if (svetofor.startRaceExecute(countLap))   {  //<-тут ведется отсчет светофора
          if (isSlalom) falseStartGate.checkFalseStart();      
        } else {
          if (isSlalom) svetoforDriverState= falseStartGate.checkGate(); 

      };
    };  
  if (prevSvetoforDriverState!=svetoforDriverState){
    switch (svetoforDriverState){
      case RoadRunner::typeLap::toStop: 
        DEBUG_PRINT("Стоп от ворот");
        svetofor.stopRace();
        if (nowRunner){
           DEBUG_PRINT("Записали бегуна");  
          String laps = nowRunner->getLaps();
          sessionResult=nowRunner->getSessionResult();
          if (onlineMode) webTable.addSendList(sessionResult); 
          //if (useSDcard) writeMessage(SD, dirPath.c_str(), nameDriverRun, laps  );
          nameDriverRun="";
          nowRunner=NULL;
         gate.attach(NULL);
         falseStartGate.attach(NULL);
        };
        break;
      case RoadRunner::typeLap::toFinish:  svetofor.setFinish();
        DEBUG_PRINT("Финиш от ворот");

        break;
      case RoadRunner::typeLap::toRace: //svetofor.stopRace();   //оно само позеленеет, когда досчитает до нуля. 
        DEBUG_PRINT("поехали");
        
        break;
      case RoadRunner::typeLap::toStart: //svetofor.startRaceExecute(0);
        DEBUG_PRINT("Был дан старт");
        break;
    };
    prevSvetoforDriverState=svetoforDriverState;
  };
    FastLED.show();
      delay(5);     
    
  
}