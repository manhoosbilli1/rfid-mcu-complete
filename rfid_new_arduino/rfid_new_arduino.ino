#include <RTClib.h> // for the RTC
#include <SoftwareSerial.h>
#include <LCD_I2C.h>
SoftwareSerial s(6, 7); // RX, TX
LCD_I2C lcd(0x27, 16, 2); // Default address of most PCF8574 modules, change according
RTC_DS1307 rtc;
#include <SPI.h> // for the RFI and SD card module
#include <SD.h> // for the SD card
File myFile;
String uid, payload;
#define CS_SD 10

const int checkInHour = 9;
const int checkInMinute = 5;

//Variable to hold user check in
int userCheckInHour;
int userCheckInMinute;


const int redLED = A0;
const int greenLED = A2;
const int buzzer = A1;

String data;
void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  SPI.begin();
  pinMode(redLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  pinMode(buzzer, OUTPUT);
  lcd.begin(); // If you are using more I2C devices using the Wire library use lcd.begin(false)
  lcd.backlight();
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // set the data rate for the SoftwareSerial port
  s.begin(9600);

  Serial.print("Initializing SD card...");
  lcd.print("Initializing ");
  lcd.setCursor(0, 1);
  lcd.print("SD card...");
  delay(3000);
  digitalWrite(redLED, HIGH);
  lcd.clear();
  if (!SD.begin(CS_SD)) {
    Serial.println("initialization failed!");
    lcd.print("Initializing ");
    lcd.setCursor(0, 1);
    lcd.print("failed!");
    return;
  }
  Serial.println("initialization done.");
  lcd.print("Initialization ");
  lcd.setCursor(0, 1);
  lcd.print("Done...");
  delay(1000); 

  // Setup for the RTC
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    lcd.clear();
    lcd.print("Couldn't find RTC");
    while (1);
  }
  else {
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  if (!rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    lcd.clear();
    lcd.print("RTC Not Running!");
  }

  lcd.clear(); 
  lcd.print("    Show card        ");
  digitalWrite(buzzer,HIGH);
  delay(500); 
  digitalWrite(buzzer,LOW); 
  
}

void loop() { // run over and over
  if (s.available()) {
    data = s.readString();
//    Serial.println(data);
    payload = data.substring(data.indexOf('$') + 1 , data.indexOf('&'));
    uid = data.substring(data.indexOf('&') + 1, data.indexOf('%'));

//    Serial.print("Payload: ");
//    Serial.print(payload);
//    Serial.print("   uid: ");
//    Serial.print(uid);


    if (payload == "Not found!") {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(payload);
      //not found buzzer
      digitalWrite(redLED, HIGH);
      digitalWrite(greenLED, LOW);  
      digitalWrite(buzzer,HIGH);
      delay(200);
      digitalWrite(buzzer, LOW); 
      delay(200); 
      lcd.clear();
      lcd.print("Show card     ");

    }
    else {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("  Welcome     ");
      lcd.setCursor(0, 1);
      lcd.print(payload); 
      digitalWrite(buzzer, HIGH); 
      digitalWrite(greenLED, HIGH);
      digitalWrite(redLED, LOW); 
      delay(2000);
      digitalWrite(buzzer,LOW); 
      lcd.clear(); 
      lcd.print("Show card      "); 
//      digitalWrite(buzzer, LOW);
      digitalWrite(greenLED, LOW);
      digitalWrite(redLED, HIGH);
      logCard(); 
    }
  }

}

void logCard() {
  // Enables SD card chip select pin
  digitalWrite(CS_SD, LOW);

  // Open file
  myFile = SD.open("DATA.txt", FILE_WRITE);

  // If the file opened ok, write to it
  if (myFile) {
    Serial.println("File opened ok");
    delay(2000);
    myFile.print(uid);
    myFile.print(", ");

    // Save time on SD card
    DateTime now = rtc.now();
    myFile.print(now.year(), DEC);
    myFile.print('/');
    myFile.print(now.month(), DEC);
    myFile.print('/');
    myFile.print(now.day(), DEC);
    myFile.print(',');
    myFile.print(now.hour(), DEC);
    myFile.print(':');
    myFile.println(now.minute(), DEC);
    Serial.println("sucessfully written on SD card");
    delay(2000);

    myFile.close();

    // Save check in time;
    userCheckInHour = now.hour();
    userCheckInMinute = now.minute();
  }
  else {

    Serial.println("error opening data.txt");
    Serial.print("error opening data.txt");
  }
  // Disables SD card chip select pin
  digitalWrite(CS_SD, HIGH);
}
