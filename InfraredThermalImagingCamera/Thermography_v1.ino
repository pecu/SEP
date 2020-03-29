// Thermalmeter
#include <Adafruit_AMG88xx.h>       // Adafruit AMG88xx Library - thermal camera lib.
#include <UTFTGLUE.h>               // MCUFRIEND_kbv - use GLUE display class.


#define LegendLeft            340
#define LegendTop             230
#define LegendWidth           30
#define LegendMaxValStrLeft   325
#define LegendMaxValStrTop    50
#define LegendMinValStrLeft   325
#define LegendMinValStrTop    250
#define ScreenLEDLeft         380
#define ScreenLEDTop          160
#define ScreenLEDSize         20

// TFT Display
#define TFT_BLACK             0x0000
#define TFT_BLUE              0x001F
#define TFT_RED               0xF800
#define TFT_GREEN             0x07E0
#define TFT_CYAN              0x07FF
#define TFT_MAGENTA           0xF81F
#define TFT_YELLOW            0xFFE0
#define TFT_WHITE             0xFFFF


// create the camara object.
Adafruit_AMG88xx ThermalSensor;
// create the TFT LCD object.
UTFTGLUE myGLCD(0,A2,A1,A3,A4,A0);


// start with some initial colors
uint16_t MinTemp = 15;
uint16_t MaxTemp = 40;

// variables for interpolated colors
byte red, green, blue;

// variables for row/column interpolation
byte i, j, k, row, col, incr, intpIndex;
float intPoint, val, a, b, c, d, ii;

int ThermalMapLeft = 15, ThermalMapTop = 15;
int RawThermalMapWidth = 8, RawThermalMapHeight = 8;
int ThermalMapDispScale = 35;
int TermalMapMaxValue = 0;

int x, y;
char buf[20];
uint16_t TheColor;

// array for the 8 x 8 measured pixels
float pixels[64];
uint16_t pixelsDisp[64];

// TBD:
// array for the interpolated array
// unsigned char HDTemp[80][80];


bool workingBlink = 0;

void setup()
{
  randomSeed(analogRead(0));

  Serial.begin(115200);
  
// Setup the LCD
  myGLCD.InitLCD();
  myGLCD.setFont(BigFont);
  myGLCD.fillScreen(TFT_BLACK);

  // show a cute splash screen (paint text twice to show a little shadow
  myGLCD.setTextSize(5);
  myGLCD.setTextColor(TFT_WHITE);
  myGLCD.print("Thermal", 22, 21);

  myGLCD.setTextSize(5);
  myGLCD.setTextColor(TFT_BLUE);
  myGLCD.print("Thermal", 20, 20);

  myGLCD.setTextSize(5);
  myGLCD.setTextColor(TFT_WHITE);
  myGLCD.print("Camera", 122, 71);

  myGLCD.setTextSize(5);
  myGLCD.setTextColor(TFT_RED);
  myGLCD.print("Camera", 120, 70);

  // let sensor boot up
  bool status = ThermalSensor.begin();
  delay(500);

  // read the camera for initial testing
  ThermalSensor.readPixels(pixels);

  // check status and display results
  if (pixels[0] < 0) 
  {
    while (1) 
    {
      myGLCD.setTextSize(3);
      myGLCD.setTextColor(TFT_RED);
      myGLCD.print("Sensor Test: FAIL", 20, 180);
      delay(500);
    }
  }
  else 
  {
    myGLCD.setTextSize(3);
    myGLCD.setTextColor(TFT_GREEN);
    myGLCD.print("Sensor Test: OK", 20, 180);
    delay(1000);
  }

  // get the cutoff points for the color interpolation routines
  // note this function called when the temp scale is changed
  GetJetMapABCDParam();

  // draw a cute legend with the scale that matches the sensors max and min
  DrawLegend();

  // draw a large white border for the temperature area
  myGLCD.setColor(TFT_WHITE);
  myGLCD.fillRect(ThermalMapLeft-5, ThermalMapTop-5,
                  ThermalMapLeft + RawThermalMapWidth * ThermalMapDispScale + 5,
                  ThermalMapTop + RawThermalMapHeight * ThermalMapDispScale + 5);             
}

void DrawWorkingBlink()
{
  if(workingBlink)
  {
    myGLCD.setColor(TFT_GREEN);
    myGLCD.fillRect(ScreenLEDLeft, ScreenLEDTop,
                    ScreenLEDLeft + ScreenLEDSize, ScreenLEDTop + ScreenLEDSize);
  }
  else
  {
    myGLCD.setColor(TFT_BLACK);
    myGLCD.fillRect(ScreenLEDLeft, ScreenLEDTop,
                    ScreenLEDLeft + ScreenLEDSize, ScreenLEDTop + ScreenLEDSize);
  }

  workingBlink = !workingBlink;
}

void loop()
{
  DrawWorkingBlink();
 

  // read the sensor
  ThermalSensor.readPixels(pixels);  

  for (row = 0; row < 8; row ++)
  {
    for (col = 0; col < 8; col ++)
    {
      pixelsDisp[row+col*8] = GetJetColor(pixels[row*8+col]);

      if( pixels[row*8+col] > TermalMapMaxValue )
      {
        TermalMapMaxValue = pixels[row*8+col];
      }
    }
  }

  //UpdateDispMaxMinTemp();
  
  for (row = 0; row < 8; row ++)
  {
    for (col = 0; col < 8; col ++)
    {
      myGLCD.setColor(pixelsDisp[row*8+col]);
      myGLCD.fillRect(ThermalMapLeft+col*ThermalMapDispScale, 
                      ThermalMapTop+row*ThermalMapDispScale, 
                      ThermalMapLeft+(col+1)*ThermalMapDispScale,
                      ThermalMapTop+(row+1)*ThermalMapDispScale);
    }
  }

  sprintf(buf, "Max:%dC", int(TermalMapMaxValue));
  myGLCD.print(buf, ScreenLEDLeft, (LegendMaxValStrTop+LegendMinValStrTop)/2-10);
}

uint16_t GetJetColor(float val) 
{
  /*
    pass in value and figure out R G B
    several published ways to do this I basically graphed R G B and developed simple linear equations
    again a 5-6-5 color display will not need accurate temp to R G B color calculation

    equations based on
    http://web-tech.ga-usa.com/2012/05/creating-a-custom-hot-to-cold-temperature-color-gradient-for-use-with-rrdtool/index.html
  */

  red = constrain(255.0 / (c - b) * val - ((b * 255.0) / (c - b)), 0, 255);

  if ((val > MinTemp) & (val < a)) {
    green = constrain(255.0 / (a - MinTemp) * val - (255.0 * MinTemp) / (a - MinTemp), 0, 255);
  }
  else if ((val >= a) & (val <= c)) {
    green = 255;
  }
  else if (val > c) {
    green = constrain(255.0 / (c - d) * val - (d * 255.0) / (c - d), 0, 255);
  }
  else if ((val > d) | (val < a)) {
    green = 0;
  }

  if (val <= b) {
    blue = constrain(255.0 / (a - b) * val - (255.0 * b) / (a - b), 0, 255);
  }
  else if ((val > b) & (val <= d)) {
    blue = 0;
  }
  else if (val > d) {
    blue = constrain(240.0 / (MaxTemp - d) * val - (d * 240.0) / (MaxTemp - d), 0, 240);
  }

  // use the displays color mapping function to get 5-6-5 color palet (R=5 bits, G=6 bits, B-5 bits)
  return ((red>>3) << 11) | ((green>>2) << 5) | (blue>>3);
}

void UpdateDispMaxMinTemp() 
{
  val = 0.0;
  for (i = 0; i < 64; i++) 
  {
    val = val + float(pixels[i]);
  }
  val = val / 64.0;

  MaxTemp = val + 5.0;
  MinTemp = val - 5.0;
  
  GetJetMapABCDParam();
  DrawLegend();
}

// function to get the cutoff points in the temp vs RGB graph
void GetJetMapABCDParam() 
{
  a = MinTemp + float(MaxTemp - MinTemp) * 0.2121;
  b = MinTemp + float(MaxTemp - MinTemp) * 0.3182;
  c = MinTemp + float(MaxTemp - MinTemp) * 0.4242;
  d = MinTemp + float(MaxTemp - MinTemp) * 0.8182;
}

// function to draw a cute little legend
void DrawLegend() 
{
  j = 0;

  float inc = float(MaxTemp - MinTemp) / 160.0;

  for (ii = MinTemp; ii < MaxTemp; ii += inc) 
  {
    myGLCD.setColor(GetJetColor(ii));
    myGLCD.drawLine(LegendLeft, LegendTop - j, LegendLeft + LegendWidth, LegendTop - j);
    j++;
  }

  myGLCD.setTextSize(2);
  myGLCD.setBackColor(TFT_BLACK);
  myGLCD.setColor(TFT_WHITE);
  sprintf(buf, "%2dC/%2dF", MaxTemp, (int) (MaxTemp * 1.8) + 32);
  myGLCD.print(buf, LegendMaxValStrLeft, LegendMaxValStrTop);
  
  myGLCD.setTextSize(2);
  myGLCD.setBackColor(TFT_BLACK);  
  myGLCD.setColor(TFT_WHITE);
  sprintf(buf, "%2dC/%2dF", MinTemp, (int) (MinTemp * 1.8) + 32);
  myGLCD.print(buf, LegendMinValStrLeft, LegendMinValStrTop);
}
