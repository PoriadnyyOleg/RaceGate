//#define SD_CS 5   // Define CS pin for the SD card module
/*microSD картридер подключён на HSPI:
D2 – SD_MISO  	12
D12 – SD_CS	15
D14 – SD_SCLK	14
D15 – SD_MOSI	13
*/
#include <SPI.h>
#include "FS.h"
#include "SD.h"
#include <time.h>

//bool isMoreDataAvailable();
//byte getNextByte();
/*
bool isMoreDataAvailable()
{
  return myFile.available();
};

byte getNextByte()
{
  return myFile.read();
};

 */



 void writeFile(fs::FS &fs, const char * path, const char * message) {
  #ifdef DEBUG
  Serial.printf("Writing file: %s\n", path);
  #endif
  
  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    DEBUG_PRINT("Failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    DEBUG_PRINT("File written");
  } else {
    DEBUG_PRINT("Write failed");
  }
  file.close();
}
// Append data to the SD card (DON'T MODIFY THIS FUNCTION)
void appendFile(fs::FS &fs, const char * path, String  &message) {
  #ifdef DEBUG
  Serial.printf("Appending to file: %s\n", path);
  #endif
  
  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    DEBUG_PRINT("Failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    DEBUG_PRINT("Message appended");
  } else {
    DEBUG_PRINT("Append failed");
  }
  file.close();
}



bool checkFile (fs::FS &fs, const char * filePath) {
  bool check = fs.exists(filePath);

  if (!check) {
    DEBUG_PRINT("File doens't exist");
    DEBUG_PRINT("Creating file...");
    writeFile(fs, filePath, " \r\n");
  }
  else {
    DEBUG_PRINT("File already exists :" + (String)filePath);
  }
  return check;
}
void writeMessage(fs::FS &fs, const char * path, String & drivername, String & lapsResult) {
String filePath=path+drivername+".txt";
  DEBUG_PRINT("Save data: ");
  DEBUG_PRINT(lapsResult);
  checkFile (fs, filePath.c_str());
  appendFile(fs, filePath.c_str(), lapsResult);
}

