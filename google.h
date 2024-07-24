//----------------------------------------//
//  google.ino
//
//  created 16/12/2019
//  by Luiz H. Cassettari
//----------------------------------------//
#define google_h
#include <HTTPClient.h>
#ifndef WiFi_h
#include <WiFi.h>
#endif

#ifndef LinkedList_h 
#include "List.h"
#endif 

struct postString {
  String path;
  String body;
};
/*
class googleSheetConnector{
private:
int connection;
//List<String&> listGet;
*/
List<postString> listPost;

unsigned long previousMillis = 0;
unsigned long interval = 30000;

String postResult =""; 

    



bool testConnection(){
return (WiFi.status() == WL_CONNECTED);
}
     
void reConnect()  {
  Serial.println("Reconnecting to WiFi...");
  WiFi.disconnect();
  WiFi.reconnect();
}
    

String get_google(String path)
{
  if (path == "") return "";

  String payload = "";

  HTTPClient http;

 
  if  (!testConnection()){
    reConnect();
  }

  http.begin(path);
  http.setReuse(false);
  http.setTimeout(5000);
  //http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
  const char * headerkeys[] = {"Location"};
  size_t headerkeyssize = sizeof(headerkeys) / sizeof(char*);
  //ask server to track these headers
  http.collectHeaders(headerkeys, headerkeyssize);

  int httpCode = http.GET();

  String location = http.header((size_t) 0);

  Serial.printf("[HTTP] GET... code: %d\n", httpCode);

  if ((httpCode == HTTP_CODE_OK))
  {
    payload = http.getString();
  }

  http.end();

  if (httpCode == HTTP_CODE_FOUND)
  {
        Serial.println("GET redirect");
    payload = get_google(location);
  }

  return payload;
}

String post_google(String path, String body)
{
  if (path == "") return "";

  HTTPClient http;

  String payload = "";

  Serial.print("[HTTP] begin...\n");
  if  (!testConnection()){
     Serial.println("Posting later:");
     Serial.println(body);
      postString _poststring;
       _poststring.body=body;
      _poststring.path=path;
     listPost.add(_poststring);
 
    return "will be sent later";
  }
  http.begin(path);
  http.setTimeout(5000);
  http.setReuse(false);
  //http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
  const char * headerkeys[] = {"Location"};
  size_t headerkeyssize = sizeof(headerkeys) / sizeof(char*);
  http.collectHeaders(headerkeys, headerkeyssize);

  Serial.printf("[HTTP] POST \n");

  int httpCode = http.POST(body);

  String location = http.header((size_t) 0);

  // httpCode will be negative on error
  if (httpCode > 0)
  {
    // HTTP header has been send and Server response header has been handled
    Serial.printf("[HTTP] POST... code: %d\n", httpCode);
    // file found at server
    if ((httpCode == HTTP_CODE_OK) || (httpCode == HTTP_CODE_FOUND))
    {
      payload = http.getString();
      Serial.println(payload);
    } else {
       postString _poststring;
        _poststring.body=body;
      _poststring.path=path;
     listPost.add(_poststring);
     Serial.println("NEED Posting later:");
     Serial.println(body);
     Serial.println(httpCode);
    }
  }
  else
  {
    Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
    payload = "";
     postString _poststring;
        _poststring.body=body;
      _poststring.path=path;
     listPost.add(_poststring);
     Serial.println("NEED Posting later:");
     Serial.println(body);
     Serial.println(httpCode);
  }

  http.end();

  if (httpCode == HTTP_CODE_FOUND)
  {
     Serial.println("NO POST to get redirect");
  //  payload = get_google(location);
  }

  return payload;
};


void googleloop() {
  if ((listPost.getSize()>0) ){
    
  postString _poststring =    listPost.pop();
      postResult= post_google(_poststring.path, _poststring.body);
  }
}



