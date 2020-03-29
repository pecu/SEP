// Declaration and initialization of the input pin
#include <WS2812.h>

int MIC_ANALOG_INPUT = A0;
int MIC_ANALOG_BASE_LEVEL = 258;
int MIC_SOUND_LEVEL1 = 20;
int BG_SOUND_LEVEL = 0; // Target: 600DN
int BG_SOUND_SAMPLING_NUM = 10000;

int LED_COUNT = 100;
WS2812 LEDStrip(LED_COUNT); // 100 LEDs
cRGB value;

int LED_R_VAL[4] = { 100, 0, 0, 100 };
int LED_G_VAL[4] = { 100, 100, 0, 0 };
int LED_B_VAL[4] = { 100, 0, 100, 0 };

int LED_COLOR_TYPE = 0;



const int numReadings = 10;
int readings[numReadings];      // the readings from the analog input
int readIndex = 0;              // the index of the current reading
int total = 0;                  // the running total
int average = 0;                // the average
  
void setup ()
{
  delay(5000);
  
  pinMode (MIC_ANALOG_INPUT, INPUT);
  pinMode (LED_BUILTIN, OUTPUT);

  pinMode (A1, OUTPUT);
       
  Serial.begin (19200); // Serial output with 9600 bps

  digitalWrite( A1, HIGH );

  for (int thisReading = 0; thisReading < numReadings; thisReading++) 
  {
    readings[thisReading] = 0;
  }

  LEDStrip.setOutput(9);

  EstBackgroundSoundLevel();
}

void BlinkLEDStrip()
{
  value.b = LED_R_VAL[LED_COLOR_TYPE]; value.g = LED_G_VAL[LED_COLOR_TYPE]; value.r = LED_B_VAL[LED_COLOR_TYPE]; // RGB Value -> Blue

  for( int LEDIdx = 0; LEDIdx < LED_COUNT; LEDIdx ++ )
  {
    LEDStrip.set_crgb_at(LEDIdx, value);  
  }  
  LEDStrip.sync(); // Sends the value to the LED
  
  delay(10); // Wait 500 ms

  value.b = 0; value.g = 0; value.r = 0; // RGB Value -> Blue

  for( int LEDIdx = 0; LEDIdx < LED_COUNT; LEDIdx ++ )
  {
    LEDStrip.set_crgb_at(LEDIdx, value);  
  }  
  LEDStrip.sync(); // Sends the value to the LED

  LED_COLOR_TYPE = ( LED_COLOR_TYPE + 1 ) % 4;
}

void EstBackgroundSoundLevel()
{  
  Serial.print( "Sampling BG sound level.\n" );

  digitalWrite( LED_BUILTIN, HIGH );

  float SoundSumVal = 0;

  for( int SamplingIdx = 0; SamplingIdx < BG_SOUND_SAMPLING_NUM; SamplingIdx ++ )
  {
    SoundSumVal += float(analogRead (MIC_ANALOG_INPUT));
  }

  BG_SOUND_LEVEL = SoundSumVal / float(BG_SOUND_SAMPLING_NUM);
  
  digitalWrite( LED_BUILTIN, LOW );

  Serial.print( "BG_SOUND_LEVEL :" );
  Serial.println( BG_SOUND_LEVEL );
  Serial.print( "Sampling BG sound level - end.\n" );

}

int CalMovingAverage(int curVal )
{
  // subtract the last reading:
  total = total - readings[readIndex];
  // read from the sensor:
  readings[readIndex] = curVal;
  // add the reading to the total:
  total = total + readings[readIndex];
  // advance to the next position in the array:
  readIndex = readIndex + 1;

  // if we're at the end of the array...
  if (readIndex >= numReadings) {
    // ...wrap around to the beginning:
    readIndex = 0;
  }

  // calculate the average:
  average = total / numReadings;
  // send it to the computer as ASCII digits
  return (average);
}
  
// The program reads the current value of the input pins
// and outputs it via serial out
void loop ()
{
  int micAnalogValue;
    
  // Current value will be read and converted to voltage 
  micAnalogValue = analogRead (MIC_ANALOG_INPUT); 
      
  //... and outputted here
  //Serial.print ("Analog voltage value:"); 
  //Serial.print (micAnalogValue, 1);  Serial.print ("DN\n");
  //Serial.print ("Extreme value:");

  // Plotter. Calibration.
  // Serial.println(CalMovingAverage(micAnalogValue));
  //Plotter. Current value.
  //Serial.println(micAnalogValue);
  
  if(abs( micAnalogValue - BG_SOUND_LEVEL ) >= MIC_SOUND_LEVEL1)
  {
      //Serial.println (" L1");
      digitalWrite( LED_BUILTIN, HIGH );
      delay( 10 );
      digitalWrite( LED_BUILTIN, LOW );
      delay( 10 );      

      BlinkLEDStrip();
  }
  else
  {
      //Serial.println (" L0");
  }
  //Serial.println ("----------------------------------------------------------------");
  //delay (10);
}
