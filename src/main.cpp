#include <ESP8266WiFi.h>
#include <ThingsBoard.h>
#define TOKEN "THINGSBOARD_TOKEN" 

char ssid[] = "WIFI_SSID"; 
char pass[] = "WIFI_PASSWORD"; 


char thingsboardServer[] = "demo.thingsboard.io"; // URL of Thingsboard server
char location[] = "DEVICE_LOCATION"; 
char source; // To confirm source of Data (S:SD CARD or R:Real time)
char Status = ' '; //Wifi Status 
char Send_Status;
char end_line; //To transmit character telling arduino to proceed next json line from SD CARD

int status = WL_IDLE_STATUS;
int i = 0;
int j = 0;
bool stat; // To check whether data has been uploaded to tb 

WiFiClient wifiClient;
ThingsBoard tb(wifiClient);

void getStatus();
void getSource();
void InitWiFi();
void reconnect();
void getAndSendRealData();
void getAndSendCardData();


void setup()
{
  Serial.begin(9600);
  delay(10);
   WiFi.mode(WIFI_STA);
  InitWiFi();
   // delay(1000);
  tb.connect(thingsboardServer, TOKEN);
  
}


void loop()
{ 
    getStatus();
    if ( !(tb.connect(thingsboardServer, TOKEN) ) ) {
       Serial.write('F'); 
       Serial.flush();
        Status = ' ';
        reconnect();
      } 
    if( tb.connected() && Status == 'S' ){
     Serial.write('E');
     Serial.flush();
     getSource();
     if(source == 'R'){
     getAndSendRealData();
    }
    else if(source == 'C'){
      while(!((end_line=='T') || (Send_Status == 'F'))){
      //Serial.println(Send_Status);
      //Serial.println("Getting lines read");
      getAndSendCardData();
          }
    if(end_line == 'T'){
      getAndSendRealData();
          }
    Send_Status = ' ';
    end_line = ' ';
        }
    }
    Status = ' ';
}

void getStatus()  //The function get status from Arduino in order to begin read Serial Data
{
  Status = ' ';
  while(Status != 'S'){
    if(Serial.available()>0){
    Status = char(Serial.read());
    }
  }
}

void getSource() //The function get source of Data (SD CARD or Real time)
{
  source = ' ';
  while(!(source == 'C' || source == 'R')){
    if(Serial.available()>0){
      source = char(Serial.read());
    }
  }
}

void InitWiFi() //The function establish connection of esp with internet
{
  j = 0;
  while(WiFi.status() != WL_CONNECTED){
    i = 0;
    //Serial.print("\nConnecting to ");
    //Serial.print(ssids[j]);
    //WiFi.begin(ssids[j], passwords[j]);
    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED && i < 10) {
      delay(500);
      //Serial.print(".");
      i++;
    }
    delay(500);
    if(WiFi.status() == WL_CONNECTED){
      //Serial.print("\nConnected to ");Serial.println(ssids[j]);
    }
    ++j;
    //if(j > sizeof(ssids) - 1){
     // j = 0;
    //}
  }
}
 void reconnect() {
 
  // Loop until we're reconnected
  while (!tb.connected()) {
     i = 0;
    status = WiFi.status();
     Serial.println(status);
    if ( status != WL_CONNECTED) {
      //Serial.print("Connecting to ");Serial.print(ssids[j]);Serial.println("...");
      //WiFi.begin(ssids[j], passwords[j]);
      WiFi.begin(ssid, pass);
      while (WiFi.status() != WL_CONNECTED && i < 10) {  
        delay(500);
          //Serial.print(".");
        i++;
      }
      
      delay(500);
      //Serial.print("Status = ");
      //Serial.println(WiFi.status());
      delay(500);
      j++;
     // if(j > sizeof(ssids) - 1){
     //   j = 0;
     // }
    }
      
    
    else {
    if ( (tb.connect(thingsboardServer, TOKEN) )) {
      break;
    }
  }
  }
}


void getAndSendRealData() //The function get and Send Real Time Data
{
 char rec = '<';
 String str = "";
 while(rec!= '>'){
  if(Serial.available()>0){
    rec = char(Serial.read());
    if(!(rec == 'R')){
    str+=rec;
  }
 }
 }
 
 str.remove(str.length()-1,1);


 stat = tb.sendTelemetryJson(str.c_str());
 tb.sendTelemetryString("location", location);

 while(!stat){
  Serial.write('F');
  reconnect();
  stat = tb.sendTelemetryJson(str.c_str());
 // Serial.println(stat);
  }
}

void getAndSendCardData() //The Function get and Send SD  card Data Line-by-Line
{
 char rec = '<';
 String str = "";
 while(rec!= '>'){
  if(Serial.available()>0){
    rec = char(Serial.read());
    if(rec == 'T'){
      end_line = rec;
      break;
    }
    str+=rec;
    //delay(10);
  }
 }

if(end_line != 'T'){
//Serial.println("Condition True");
str.remove(str.length()-1,1);
 char jsonString[str.length()];
 str.toCharArray(jsonString,str.length());

 stat = tb.sendTelemetryJson(jsonString);
 tb.sendTelemetryString("location", location);
 if (stat){
  Serial.write('N');
 }
 else{
 while(!stat){
  Serial.write('F');
  Send_Status = 'F';
  reconnect();
  stat = tb.sendTelemetryJson(str.c_str());
  }
}
}
}
