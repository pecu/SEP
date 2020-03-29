#include <Adafruit_MLX90614.h>
#include <LiquidCrystal_I2C.h>

// I2C
#include <Wire.h>

// Thermal.
Adafruit_MLX90614 therm = Adafruit_MLX90614();

// LCD
LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display

// PIR
int pirPin = 13;

// Mp3 player.
#include "SoftwareSerial.h"
#define Start_Byte 0x7E
#define Version_Byte 0xFF
#define Command_Length 0x06
#define DoRepeat 0x11
#define NORMAL_PLAY 0x0D
#define End_Byte 0xEF
#define Acknowledge 0x00 //Returns info with command 0x41 [0x01: info, 0x00: no info]
#define ACTIVATED LOW
#define PLAYER_VOL 25
#define PLAYER_PIN_BUSY 10

SoftwareSerial mySerial(8, 9);
boolean isPlaying = false;


void setup() 
{
  Serial.begin(9600);

  // Thermal
  therm.begin();

  // LCD  
  lcd.init();                      // initialize the lcd 
  lcd.init();
  lcd.backlight();

  // PIR
  pinMode(pirPin, INPUT);     

  // Player.
  mySerial.begin (9600);
  pinMode(PLAYER_PIN_BUSY, INPUT);
  playerInit();
}

void loop() 
{
  float  ambTempInDeg, tarTempInDeg;
  String ambTempDispStr, objTempDispStr;
  int pirStat = 0;

  pirStat = digitalRead(pirPin); 
  
  
  ambTempInDeg = therm.readAmbientTempC();
  tarTempInDeg = therm.readObjectTempC();
  
  ambTempDispStr = "Ambient: " + String(ambTempInDeg, 1) + "C  ";
  objTempDispStr = "Target : " + String(tarTempInDeg, 1) + "C  ";

      
  Serial.print(ambTempDispStr);    
  Serial.print(objTempDispStr);

  lcd.setCursor(0,0);
  lcd.print(ambTempDispStr);
  lcd.setCursor(0,1);
  lcd.print(objTempDispStr);

  if (pirStat == HIGH) 
  {
    Serial.print(" PIR ON");

    if(digitalRead(PLAYER_PIN_BUSY) == HIGH )
    {
      playFirst();
      //play();
    }
  }
  else
  {
  }

  Serial.println("");
     
  delay(5);
}

void playerInit()
{
  execute_CMD(0x3F, 0, 0);
  delay(500);
  setVolume(PLAYER_VOL);
  delay(500);  
}

void playFirst()
{
  // Repeats
  //execute_CMD(DoRepeat,0,1); 
  execute_CMD(NORMAL_PLAY,0,0);
  delay(500);
}

void pause()
{
  execute_CMD(0x0E,0,0);
  delay(500);
}

void play()
{
  execute_CMD(0x0D,0,1); 
  delay(500);
}

void playNext()
{
  execute_CMD(0x01,0,1);
  delay(500);
}

void playPrevious()
{
  execute_CMD(0x02,0,1);
  delay(500);
}

void setVolume(int volume)
{
  execute_CMD(0x06, 0, volume); // Set the volume (0x00~0x30)
  delay(2000);
}

void execute_CMD(byte CMD, byte Par1, byte Par2)
// Excecute the command and parameters
{
  // Calculate the checksum (2 bytes)
  word checksum = -(Version_Byte + Command_Length + CMD + Acknowledge + Par1 + Par2);
  
  // Build the command line
  byte Command_line[10] = { Start_Byte, Version_Byte, Command_Length, CMD, Acknowledge,
  Par1, Par2, highByte(checksum), lowByte(checksum), End_Byte};
  
  //Send the command line to the module
  for (byte k=0; k<10; k++)
  {
    mySerial.write( Command_line[k]);
  }
}
