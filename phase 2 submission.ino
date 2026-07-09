#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

LiquidCrystal_I2C lcd(0x27,16,2);
const byte rows = 4;
const byte cols=3;
byte rowpins[rows] = {5,17,16,4};
byte colpins[cols] = {14,27,26};

char numpad [rows][cols]= { //creating 4x3 keypad and not taking the last column with the capital letters
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
Keypad keypad = Keypad(makeKeymap(numpad),rowpins,colpins,rows,cols);
int cursor=0;
char key; //keypad press is inputed as a character 
unsigned long keyTime=0; //to keep time of last keypad press
unsigned long lockTime=0; // to kepp time of timeout phase 
int wrong = 0; // no of wrong entries
boolean lock = false; // keeping state of timeout
boolean correctCode = false; 
int codeValue=0; //to convert entered numbers into actual code for processing 
unsigned long successtime=0;//time interval to show success when correct code entered

struct User { // Struct with user values
  String name;
  int code;
  String timenow;
};

User User1 = {"User1",1111}; // Creating 5 random deafult users for this code
User User2 = {"User2",2222};
User User3=  {"User3",3333};
User User4 = {"User4",4444};
User User5 = {"User5",5555};
User storedLog[10]; // array for storing offling logs
int stored=0; // to keep track of no of stored logs in array

WiFiUDP ntpudp;
NTPClient timeClient(ntpudp); // Creating client to fetch the current time of code entry 
String timegot;
WiFiClient client;
HTTPClient http;
const char* ssid = "Wokwi-GUEST";
const char* password = "";
long wifitime=0;
const char* server = "http://httpcan.org/post"; // random mock server for http post, httpbin was busy so used this

void sendpost(struct User userLog); // declaring a function used aheah for http post

void checkCode(struct User a , int codeEntry) // Using this function to individually compare codes of all users available 
{
  if (a.code == codeEntry)
    {
      correctCode=true;
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("SUCCESS");
      lcd.setCursor(0,1);
      lcd.print("Welcome ");
      lcd.print(a.name);
      digitalWrite(25,HIGH);
      successtime=millis();
      while((millis()-successtime)<300){}//To show success for 0.3 seconds.
      digitalWrite(25,LOW);
      lcd.clear();
      wrong=0;
      a.timenow = timeClient.getFormattedTime(); // getting the time of code entry 
      sendpost(a);
      
    }
}

void sendpost(struct User userLog) // function to send http posts
{
  JsonDocument myjson;
  String jsonString; // creating Json string to upload using http post 

  if(WiFi.status() == WL_CONNECTED)
  {
    if(stored>0) // first flushing all stored logs if there are any when wifi is connected 
    {
      for(int i=0;i<stored;i++)
      {
        jsonString = ""; // To not kepping appending the json string 
        myjson["name"] = storedLog[i].name;
        myjson["code"] = storedLog[i].code;
        myjson["time"] = storedLog[i].timenow;
        serializeJson(myjson, jsonString);
        http.begin(client,server);
        http.addHeader("Content-Type","application/json");
        int httpResponseCode = http.POST(jsonString);
        Serial.print("Sent stored Post, response code: ");
        Serial.println(httpResponseCode);
        http.end(); 
      }
      stored=0;
    }
    
    myjson["name"] = userLog.name;
    myjson["code"] = userLog.code;
    myjson["time"] = userLog.timenow;
    serializeJson(myjson, jsonString);
    http.begin(client,server);
    http.addHeader("Content-Type","application/json");
    int httpResponseCode = http.POST(jsonString);

    Serial.print("Sent Post , response code: ");
    Serial.println(httpResponseCode);
    http.end();
  }
  else //If WiFi disconnects 
  {
    if(stored<10)//TO store only ten values as array is of 10 struct size only
    {
      storedLog[stored] = userLog;
      stored++; // this is the number of stroed logs currently at this step
    }
    WiFi.begin(ssid,password);//Trying to reconnect WiFi in the background
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  keypad.setDebounceTime(10); 
  lcd.init();
  lcd.backlight();
  pinMode(25,OUTPUT); // has leds which glow on correct code entry 

  Serial.print("Connecting to ");// Initial connection to WiFi 
  Serial.println(ssid);
  WiFi.begin(ssid,password);
  while(WiFi.status() != WL_CONNECTED)
  {
    if ((millis() - wifitime) > 500 )
    {
      Serial.print("."); //printing dots while connecting wifi 
      wifitime=millis(); // just to print dots hehe !!
    }
  }
  Serial.println();
  Serial.println("WiFi Connected");
  Serial.println("IP Address is: ");
  Serial.println(WiFi.localIP());

  timeClient.begin();
  timeClient.setTimeOffset(19800); // india is 5 hr 30 min aheah of gmt , thats why changing the gmt time got.
  while(!timeClient.update()) { //To forcefully convert the GMT time to IST 
    timeClient.forceUpdate();
  }
  timegot = timeClient.getFormattedTime();
  Serial.println(timegot); // only printing the starting time when the code starts
}

void loop() {
  // put your main code here, to run repeatedly:
  switch (lock){
    case false: // code executes only in the non-timeout stage
      key = keypad.getKey(); //key is inputed as a character 
      if (key!=NO_KEY) 
      {
        Serial.print(key); //to see our pressed key in serial monitor
        keyTime=millis(); // keeps time of last key press
      } 
      if (key == '#' && cursor!=0){  // BACKSPACE KEY is HASHTAG
        cursor--; //one asterisk is removed from backspace
        lcd.clear();
        codeValue = codeValue/10; //removing the units digit of entered code
        for(int i=0;i<cursor;i++){  // FOR LCD VIEW AFTER BACKSPACE
          lcd.setCursor(i,0);
          lcd.print('*');
        }
      }
      else if(key != '*' && key !=NO_KEY) { //There is no use of asterisk in the keypad in my code
        lcd.setCursor(cursor,0);
        lcd.print('*');
        cursor++;
        codeValue = codeValue*10 + (int(key)-48); // 48 is ASCII value of zero.
      }
      if ( cursor == 4){ // after 4 digit code is entered checking its value 
        Serial.println();
        checkCode(User1,codeValue);
        checkCode(User2,codeValue);
        checkCode(User3,codeValue);
        checkCode(User4,codeValue);
        checkCode(User5,codeValue);
        
        if(correctCode==false)
        {
          wrong++;
        }
        correctCode=false;//initially false for next void loop iteration 
        lcd.clear();
        lcd.setCursor(0,0);
        cursor=0;
        codeValue=0;
      }
      if (wrong == 3){ // enters timeout state for 3 wrong continuous entries
        lock = true;
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("TIMEOUT");
        lockTime = millis();
        wrong=0;
      }
      if (millis() - keyTime >= 7000 && cursor!=0){ // clearing the code when no key press for 7 seconds
        lcd.clear();
        cursor=0;
        codeValue=0;
        Serial.println();

      }
      break;

    case true:
    if ((millis() - lockTime) > 15000) // keeping timeout state for 15 sec
    {
      lock = false;
      lcd.clear();
    }
  }
  
}
