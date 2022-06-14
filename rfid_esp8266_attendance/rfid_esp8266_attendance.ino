//**********libraries***********
//RFID-----------------------------

#include <SPI.h>
#include <MFRC522.h>
//NodeMCU--------------------------
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#include <SoftwareSerial.h> 
SoftwareSerial s(D1, D2); 
HTTPClient http;    //Declare object of class HTTPClient
WiFiClient client;    //GET Data    HTTPClient http;    //Declare object of class HTTPClient
String data =""; 
//************************
#define SS_PIN  D4  //D2
#define RST_PIN D0  //D1
//************************
MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance.
//************************
/* Set these to your desired credentials. */
const char *ssid = "HarooN..";
const char *password = "haroonmectec";
const char* device_token  = "d84ec8c34b689b1e";
//************************
String URL = "http://192.168.0.109/rfidattendance/getdata.php"; //computer IP or the server domain
String getData, Link;
String OldCardID = "";
unsigned long previousMillis = 0;
//************************
void setup() {
  delay(1000);
  s.begin(9600);
  Serial.begin(9600);
  SPI.begin();  // Init SPI bus
  mfrc522.PCD_Init(); // Init MFRC522 card
  //---------------------------------------------
  connectToWiFi( );
  s.print('C'); //send connected to arduino 

}
//************************
void loop() {
  delay(1000);
  //check if there's a connection to Wi-Fi or not
  if (!WiFi.isConnected()) {
    connectToWiFi();    //Retry to connect to Wi-Fi
  }
  //---------------------------------------------
  if (millis() - previousMillis >= 15000) {
    previousMillis = millis();
    OldCardID = "";
  }
  delay(50);
  //---------------------------------------------
  //look for new card
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;//got to start of loop if there is no card present
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;//if read card serial(0) returns 1, the uid struct contians the ID of the read card.
  }
  String CardID = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    CardID += mfrc522.uid.uidByte[i];
  }
  //---------------------------------------------
  if ( CardID == OldCardID ) {
    return;
  }
  else {
    OldCardID = CardID;
  }
  //---------------------------------------------
  //  Serial.println(CardID);
  SendCardID(CardID);
  delay(1000);
}
//*****send the Card UID to the website****
void SendCardID( String Card_uid ) {
  Serial.println("Sending the Card ID");
  if (WiFi.isConnected()) {
    getData = "?card_uid=" + String(Card_uid) + "&device_token=" + String(device_token); // Add the Card ID to the GET array in order to send it
    //GET methode
    Link = URL + getData;
    http.begin(client, Link); //initiate HTTP request   //Specify content-type header

    int httpCode = http.GET();   //Send the request
    String payload = http.getString();    //Get the response payload

    //    Serial.println(Link);   //Print HTTP return code
    Serial.println(httpCode);   //Print HTTP return code
    Serial.println(Card_uid);     //Print Card ID
    Serial.println(payload);    //Print request response payload

    if (httpCode == 200) {
      String user_name;
      if (payload.substring(0, 5) == "login") {
        String user_name = payload.substring(5);
      }
      else if (payload.substring(0, 6) == "logout") {
        user_name = payload.substring(6);

      }
      else if (payload == "succesful") {
 
        Serial.println("Not registered  ");
      }
      else if (payload == "available") {
        Serial.print("Recognized  ");
        Serial.println(user_name);
      }
      delay(100);
      data = "$"; 
      data += payload; 
      if(payload == "available"){
        data += "$"; 
        data += "user_name@";
      }
      data += "&";
      data += Card_uid; 
      data += "%"; 
      s.println(data); 
      Serial.println(data); 
      data = ""; 
      http.end();  //Close connection
    }
  }
}
//*******connect to the WiFi*******
void connectToWiFi() {
  WiFi.mode(WIFI_OFF);        //Prevents reconnection issue (taking too long to connect)
  delay(1000);
  WiFi.mode(WIFI_STA);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Connected");

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  //IP address assigned to your ESP

  delay(1000);
}
//=======================================================================
