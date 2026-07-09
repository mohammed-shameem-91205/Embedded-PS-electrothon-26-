#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

LiquidCrystal_I2C lcd(0x27,16,2);
const byte rows = 4;
const byte cols=3;
byte rowpins[rows] = {5,17,16,4};
byte colpins[cols] = {14,27,26};

char numpad [rows][cols]= { 
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
Keypad keypad = Keypad(makeKeymap(numpad),rowpins,colpins,rows,cols);
int cursor=0;
unsigned long keyTime=0;
unsigned long lockTime=0;
int wrong = 0;
boolean lock = false;
boolean correctCode = false;
int codeValue=0;

struct User {
  String name;
  int code;
};

User User1 = {"User1",1111};
User User2 = {"User2",2222};
User User3=  {"User3",3333};
User User4 = {"User4",4444};
User User5 = {"User5",5555};
void checkCode(struct User a , int codeEntry)
{
  if (a.code == codeEntry)
    {
      correctCode = true;
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("SUCCESS");
      lcd.setCursor(0,1);
      lcd.print("Welcome ");
      lcd.print(a.name);
      digitalWrite(25,HIGH);

    }
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  keypad.setDebounceTime(10);
  lcd.init();
  lcd.backlight();
  pinMode(25,OUTPUT);

}

void loop() {
  // put your main code here, to run repeatedly:
  if (correctCode == false)
    {
    if (lock == false){
      char key = keypad.getKey(); //key is inputed as a character 
      if (key!=NO_KEY) 
      {
        Serial.print(key); //to see our code in serial monitor
        keyTime=millis();
      } 
      if (key == '#' && cursor!=0){  // BACKSPACE KEY
        cursor--;
        lcd.clear();
        codeValue = codeValue/10;
        for(int i=0;i<cursor;i++){  // FOR LCD VIEW AFTER BACKSPACE
          lcd.setCursor(i,0);
          lcd.print('*');
        }
      }
      else if(key != '*' && key !=NO_KEY) {
        lcd.setCursor(cursor,0);
        lcd.print('*');
        cursor++;
        codeValue = codeValue*10 + (int(key)-48); // 48 is ASCII value of zero.
      }
      if ( cursor == 4){
        Serial.println();
        checkCode(User1,codeValue);
        checkCode(User2,codeValue);
        checkCode(User3,codeValue);
        checkCode(User4,codeValue);
        checkCode(User5,codeValue);
        
        if(correctCode==false)
        {
          lcd.clear();
          lcd.setCursor(0,0);
          wrong++;
          cursor=0;
          codeValue=0;
        }
      }
      if (wrong == 3){
        lock = true;
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("TIMEOUT");
        lockTime = millis();
        wrong=0;
      }
      if (millis() - keyTime >= 7000 && cursor!=0){
        lcd.clear();
        cursor=0;
        codeValue=0;
        Serial.println();

      }
    }
    else if (millis() - lockTime > 15000){
      lock = false;
      lcd.clear();
    }
  }


}
