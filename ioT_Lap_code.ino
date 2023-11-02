
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include "DHT.h"
#include "ThingSpeak.h"

#include <Servo.h>
Servo servo;

#define SECRET_CH_ID 2253800
#define SECRET_WRITE_APIKEY "GMFJUAZRPSRAKVSY"
WiFiClient  client;
unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;

#define DHTPIN D2
#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE);

#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>
#define WIFI_SSID "Do An 2"
#define WIFI_PASSWORD "NgManhLoc"

#define API_KEY "AIzaSyAdd1ziU6jZmzYuFFX3DmnMND8lBkbXZhw"
#define USER_EMAIL "nguyenhieuvan0703@gmail.com"
#define USER_PASSWORD "Hieu12345"
#define DATABASE_URL "iot-lap1-default-rtdb.firebaseio.com"
#define DATABASE_SECRET "CAfOISEioXX2097y9VR3ohvEQOAmbO7v8lU7U1eB"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
/*rain sensor pin*/
#define rain_pin D1
/*gas sensor pin*/
#define Gas_Pin A0

/*khai bao bien*/


int door; int door_value;
int lamp; int lamp_value;
int fan; int fan_value;
int buzzer; int buzzer_value;

unsigned long gas=0; unsigned long gas_status;
float t ; float t_status ;
uint8_t h;uint8_t h_status;

int rain_value = 1;
int rain_status = 1;
int i=0;
unsigned long dataMillis = 0;

int x=0;int y =0;int z =0;
uint8_t tt1=0;
void Read_Sensor(){
  gas_status = analogRead(Gas_Pin);
  h_status = dht.readHumidity();
  t_status = dht.readTemperature();
  Serial.print("nhiet do:");
  Serial.println(gas_status);
}
void setup()
{

    Serial.begin(9600);
    dht.begin();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

    config.api_key = API_KEY;
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;
   
    config.database_url = DATABASE_URL;
   
    Firebase.reconnectNetwork(true);
    fbdo.setBSSLBufferSize(4096,1024);
    fbdo.setResponseSize(5096);
    String base_path = "/UsersData/";
    
    config.token_status_callback = tokenStatusCallback; 
  
    Firebase.begin(&config, &auth);

    String var = "$userId";
    String val = "($userId === auth.uid && auth.token.premium_account === true && auth.token.admin === true)";
    Firebase.setReadWriteRules(fbdo, base_path, var, val, val, DATABASE_SECRET);
    ThingSpeak.begin(client);
    pinMode (rain_pin, INPUT);
    pinMode (D0, OUTPUT);
    pinMode (D5, OUTPUT);
    pinMode (D6, OUTPUT);
    servo.attach(D7);
    servo.write(0);
    
    delay(200);
}

void loop()
{
    
    Read_Sensor();
    rain_status = digitalRead(rain_pin);
    if (millis() - dataMillis > 300 && Firebase.ready())
    { 
        dataMillis = millis();
        
          
            Serial.printf("Get int door %s\n", Firebase.getInt(fbdo, F("/Control/Door")) ? String(door_value=fbdo.to<int>()).c_str() : fbdo.errorReason().c_str());delay(10);Serial.println(); 
                    ThingSpeak.setField(1,t_status);
                    ThingSpeak.setField(2,int(h_status));
                    ThingSpeak.setField(3,int(gas_status));
                    x = ThingSpeak.writeFields(myChannelNumber,myWriteAPIKey);
                    if( x == 200){
                      Serial.println("field 1 OK");
                    }
                    else Serial.println("Problem updating channel. HTTP error code " + String(x));//Serial.printf("field 1 False: %d\n",x);
          Serial.printf("Get int lamp %s\n", Firebase.getInt(fbdo, F("/Control/Lamp")) ? String(lamp_value=fbdo.to<int>()).c_str() : fbdo.errorReason().c_str());delay(10);Serial.println();
          Serial.printf("Get int fan %s\n", Firebase.getInt(fbdo, F("/Control/Fan")) ? String(fan_value=fbdo.to<int>()).c_str() : fbdo.errorReason().c_str());delay(10);Serial.println();
          Serial.printf("Get int buzzer %s\n", Firebase.getInt(fbdo, F("/Control/Buzzer")) ? String(buzzer_value=fbdo.to<int>()).c_str() : fbdo.errorReason().c_str());delay(10);Serial.println(); 
        
          Set_Firebase();
    }
    Control();
}
void Set_Firebase(){
    if(t!=t_status){
     t = t_status;
      Serial.printf("Set int Temp %s\n", Firebase.setFloat(fbdo, "/Sensor/Temp", t) ? "ok" : fbdo.errorReason().c_str());delay(10);
    }
    if(h!=h_status){
     h = h_status;
     Serial.printf("Set int Humid %s\n", Firebase.setInt(fbdo, "/Sensor/Humid", h) ? "ok" : fbdo.errorReason().c_str());delay(10);
    }
    if(gas!=gas_status){
    gas = gas_status;
     Serial.printf("Set int Gas %s\n", Firebase.setInt(fbdo, "/Sensor/Gas", gas) ? "ok" : fbdo.errorReason().c_str());delay(10);
    }
    if(rain_value!=rain_status) {
    rain_value = rain_status;
    Serial.printf("Set int rain %s\n", Firebase.setFloat(fbdo, "/Sensor/Rain", rain_status) ? "ok" : fbdo.errorReason().c_str());delay(3);
  }
}
void Control(){
    if(door!=door_value){
        door = door_value;
        switch(door){
          case 0: servo.write(0);break;
          case 1: servo.write(120);break;
      }
    }
    if(fan!=fan_value){
      fan = fan_value;
      digitalWrite (D0,fan );
    }
    if(lamp!=lamp_value){
      lamp=lamp_value;
      digitalWrite (D5,lamp);
    }
    if(buzzer!=buzzer_value){
      buzzer=buzzer_value;
      digitalWrite (D6,buzzer);
    }
}
