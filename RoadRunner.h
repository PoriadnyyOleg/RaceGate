#ifndef RoadRunner_h
#define RoadRunner_h

#define CHILLOUT 1000

#ifdef DEBUG
#define DEBUG_PRINT(x) Serial.println(x)
#else
#define DEBUG_PRINT(x)
#endif
#ifndef Arduino_h 
#include <Arduino.h>
#endif 

#ifndef LinkedList_h 
#include "List.h"
#endif 
//#include <List.hpp>
const int maxLap = 20;

//сделать хранение кругов через связный список 


class RoadRunner {  //esp

public: enum typeLap {
  toStart,    //  отсчет
  toRace,     //  сама гонка
  toFinish,   //  lastLap
  toStop      //  stop
};

private:
  //String nameDriver;
 List<List<uint32_t>>sessions;  // добавить элемент: sessions.pashBack(ListValue); // ListValue - круги
 List<uint32_t> * currentSession; //
  int row;    //offset строки в таблице
  int lap;
  int session =0;
  int lastMillis;  
  bool isFalseStart;
  int finishLap;
  typeLap thisLap; 
public:
  String nameDriver;
  RoadRunner(String &nameDriver, int finishLap, int row ){
    this->nameDriver=nameDriver;
    lap=0;  
    lastMillis=0;
    this->finishLap=finishLap;
    this->row=row;
    thisLap=typeLap::toStart;

};



  void startSession(){
    session++;
    //currentSession=new List<uint32_t>;
    sessions.add(*(new List<uint32_t>)) ;
    currentSession=sessions.getLast();
    isFalseStart=false;
      
  }
  void makeStart(){
    thisLap=typeLap::toRace;
  }


  void falseStart(){
    thisLap=typeLap::toStop;
    isFalseStart=true;
    currentSession->add(0);
  }



 void nextLap(){   //0 - условное время старта, 1 - время первого круга, 2 второго, итд
    /*if (lap<maxLap){
    times[lap]=millis()-lastMillis;
    lastMillis=millis();
    lap++;
    };
   */ //
    currentSession->add(millis()-lastMillis);
    lastMillis=millis();
    
 };
  typeLap getTypeLap(){
    lap=currentSession->getSize();
    //DEBUG_PRINT("lap in type lap is ");
    //DEBUG_PRINT(lap);
     // if  (lap<0){  
     // thisLap=toStart;
    //} else 
    if (isFalseStart){
      thisLap= typeLap::toStop;    
    } else {
    if  (lap<finishLap){  
      thisLap=toRace;
    } else if (lap == finishLap) {    //финишный круг?
      thisLap = typeLap::toFinish;
    } else {
      thisLap= typeLap::toStop;       
    };
    };
      return thisLap;
      /*
     if (lap< finishLap*session +(session-1)) {   //основной круг?
      thisLap=toRace;
    } else if (lap == finishLap*session +(session-1) ) {    //финишный круг?
      thisLap = typeLap::toFinish;
    } else {
      thisLap= typeLap::toStop;       
    };
      return thisLap;
      */
};

int getLap(){

  return  (currentSession->getSize());
  //(lap-session+1)%finishLap ;
};


String getLaps(){
   DEBUG_PRINT("method Get Laps");
  String result;
  result+=nameDriver;
  result+="\n";
  int gettedLap=1;
 DEBUG_PRINT(sessions.getSize());
  for (int numSession=0; numSession < sessions.getSize(); numSession++){
  result+= "Сессия : ";
  result+=(numSession+1);
  result+=" \n";
  DEBUG_PRINT("size of session");
   DEBUG_PRINT(sessions[numSession].getSize());
   if (sessions[numSession].getSize()<1) {
    result+= "Нет результатов";
   } else {
   if (sessions[numSession][0]==0) {
      result+= "Фальстарт "; 
      result+=" \n";
  } else {
    // DEBUG_PRINT(sessions[numSession][0]);
  for (int numLap=1; numLap<  sessions[numSession].getSize();numLap++   ){
    
  
  result+= "Круг "; 
  result+=gettedLap;
  result+=" : ";
  uint32_t sec = sessions[numSession][numLap] / 1000u;      // полное количество секунд
  int timeMins = (sec % 3600ul) /60ul;  // минуты
  int timeSecs = (sec % 3600ul) % 60ul;  // секунды
  int timeMsecd=sessions[numSession][numLap] % 1000u;  // ul

  result+= timeMins;
  result+=":";
  result+=timeSecs;
  result+=".";
  String Msecd ="";
  if (timeMsecd<100){
    Msecd="0";
  } else if (timeMsecd<10){
    Msecd="00";
  };
  result+=Msecd;
  result+= timeMsecd;
  result+=" \n";
  DEBUG_PRINT("gettedLap");
  DEBUG_PRINT(gettedLap);
  gettedLap++;
  };
  };
  };
  };
  DEBUG_PRINT(result);
  return result;
};



String getSessionResult(){
   DEBUG_PRINT("method Get Session to POST");
   DEBUG_PRINT(sessions[session-1].getSize());

   //if (sessions[session-1].getSize()==0) return "";
  String result;
  int gettedLap=1;
//  "row=2&session=0&laps=0:1:32,476|0:1:33,742 %7C 0:1:38,039");
  result+="row=";
  result+=row;
  result+="&session=";
  result+=session-1;
  result+="&laps=";
   if (isFalseStart) {
      result+= "Фальстарт "; 
  } else {
      
  for (int numLap=1; numLap<  sessions[session-1].getSize();numLap++   ){
  result+= "0:";      //часов не наблюдаем
 
  uint32_t sec = sessions[session-1][numLap] / 1000u;      // полное количество секунд
  int timeMins = (sec % 3600ul) /60ul;  // минуты
  int timeSecs = (sec % 3600ul) % 60ul;  // секунды
  int timeMsecd=sessions[session-1][numLap] % 1000u;  // ul

  result+= timeMins;
  result+=":";
  result+=timeSecs;
  result+=",";
  String Msecd ="";
  if (timeMsecd<100){
    Msecd="0";
  } else if (timeMsecd<10){
    Msecd="00";
  };
  result+=Msecd;
  result+= timeMsecd;
    if (numLap<  sessions[session-1].getSize()-1)   result+="|";
  DEBUG_PRINT(gettedLap);
  gettedLap++;
  };
  };
   DEBUG_PRINT(result);
  return result;
};





};


class Gate{
private:
int pin;

RoadRunner* nowRunner;
bool state;
int lastTouch;
public:
Gate(int pin){
this->pin=pin;
//pinMode(pin, INPUT);
pinMode(pin, INPUT_PULLDOWN);

lastTouch = 0;
nowRunner=0;
};


void attach(RoadRunner* runner){
  nowRunner=runner;
};

RoadRunner::typeLap checkGate(){  // кто переключает светофор?????
    state = digitalRead(pin);
  if (state && (millis()- lastTouch)>CHILLOUT){
    if (nowRunner!=0){
        nowRunner->nextLap();
      };
    lastTouch=millis();
     DEBUG_PRINT("NEW LAP!");
      if (nowRunner!=0){
     DEBUG_PRINT(nowRunner->getTypeLap()); 
      };    
  };
  if (nowRunner!=0){
    return nowRunner->getTypeLap();
  } else {
   return RoadRunner::typeLap::toStop;
  };
};

void checkFalseStart(){  
    state = digitalRead(pin);
  if (state){
    if (nowRunner!=0){
        nowRunner->falseStart();
          DEBUG_PRINT("Falsestart!");
      };
    //  return state;
  };
  
};


};
/*
class DriveResult{
  public: 
  String * result;
  //DriveResult * prev;
  DriveResult * next; 
  DriveResult(String &_result, ){
    result=_result;

  }

}
*/



#endif