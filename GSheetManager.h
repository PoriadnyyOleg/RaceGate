/* GOOGLE URL */
#define GSheetManager_h

#define GOOGLE_URL "https://script.google.com/macros/s/AKfycbw1XriUDOxm2mQosHF0YwbalcHOX8p3KBYEvvniRrKbkjjuHeI7wLuTDfj70i29K3ik/exec"
#define GOOGLE_TIME 30

#ifndef RoadRunner_h
#include"RoadRunner.h"
#endif 

#ifndef LinkedList_h 
#include "List.h"
#endif 

#ifndef Arduino_h 
#include <Arduino.h>
#endif 

#ifndef google_h 
#include "google.h"
#endif 

struct StringParser {
    
    void reset() {
        from = to = -1;    
    }
    bool update(String& s, char div = ',') {   //
        if (to == s.length()) return 0;
        from = to + 1;
        to = s.indexOf(div, from);
        if (to < 0) to = s.length();
        str = s.substring(from, to);
        return str.length()>0;    //пустые строки не нужны
    }
    String str;
    int from = -1;
    int to = -1;
};

class GSheetManager{
private:
  List<String> messages;    
  List<RoadRunner*> runners;
  int finishLap;
  bool _wasInit=false;
int row;


public:
  // GSheetManager();
  int getFinishLap(){
   String buffer;
     DEBUG_PRINT("Get Laps: ");
     buffer=GOOGLE_URL;
    buffer= get_google(buffer+"?howLaps=w");
   DEBUG_PRINT("buffer");
    DEBUG_PRINT(buffer);
    return   buffer.toInt();  ///
  }

  int getRaceType(){
   String buffer;
     DEBUG_PRINT("Get Race Type: ");
     buffer=GOOGLE_URL;
     StringParser  data;
    buffer= get_google(buffer+"?getType=w");
   DEBUG_PRINT("buffer");
    DEBUG_PRINT(buffer);
      data.update(buffer);
     int type =0  ;  // сделаем кодирование: 1000 - режим слалом  , 0 - обычный тайматтак
      data.update(buffer);
     String s("Слалом");
      if (s== data.str) type=1000; 
    return   type;  ///
  }


  void setFinishLap(int lap){
    finishLap=lap;
  }

  void lazyInitialization(){
    //finishLap= getFinishLap();  //проблемы с отправкой двух запросов подряд
    String rawTableString =get_google(GOOGLE_URL);
    DEBUG_PRINT(rawTableString);
    StringParser  data;
      row = 0;
    // можем обратиться к полученным строкам как data[i] или data.str[i]
     while (data.update(rawTableString)) {
        String name = data.str + " "+ (data.update(rawTableString)? data.str:"");
        
      runners.add( new RoadRunner(name, finishLap, row)) ;   //String &nameDriver, int finishLap, int row
      row++;  
  };
  _wasInit=true;
  };

  void addSendList(String& message){
    //messages.add(message);
    String str = message;
    str += post_google(GOOGLE_URL,str);
    DEBUG_PRINT(str);
  } ;

  bool hasNextRunner(){ //есть ли в полученных участниках незарегистрированные?
    DEBUG_PRINT("hasNextRunner");
      if (!_wasInit) lazyInitialization();
      DEBUG_PRINT( runners.getSize());
    return runners.getSize()>0;
  };

  RoadRunner* getNextUnit(){
      return runners.shift();
  };

  RoadRunner* manualReg(String& name, String& car, int finLap){
        String str = "newDriver=";
        str+=name;
        str+="&car=";
        str+=car;
        str+="&row=";
        str+=row;  // 
        str += post_google(GOOGLE_URL,str);
        DEBUG_PRINT(str);
        String nameDriverCar = name+" "+car;
      return new RoadRunner(nameDriverCar, finLap, row++);



  };

  void sendIP(String& message){
    //messages.add(message);
    String str = "myIP=";
    str+=message;
    str += post_google(GOOGLE_URL,str);
    DEBUG_PRINT(str);
  } ;


};

