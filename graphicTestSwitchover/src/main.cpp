#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <SD.h>

// For the Adafruit shield, these are the default.
#define TFT_DC 9
#define TFT_CS 10
#define selectPin 33

void testText(void);
void sampleSelect(void);

const int chipSelect = BUILTIN_SDCARD;

//File root = SD.open("/"); //open root of sd card
File root; //root of sd card
File curSample; //current sample
File nextSample;
  

// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
// If using the breakout, change pins as desired
//Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);
int main(){
  Serial.begin(9600);
   
  IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_07 = 5; //set pin 33 to GPIO mode
  IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_07 = IOMUXC_PAD_DSE(7) | IOMUXC_PAD_PKE | IOMUXC_PAD_PUE | IOMUXC_PAD_PUS(3) | IOMUXC_PAD_HYS; //settings for input pullup
  //drive strength 7,  pull/keeper enable, pull selected, 100k Ohm Pull up, enable hysteresis
  GPIO9_GDIR &= ~(1<<7);//GPIO9_IO7 set to input (might be wrong)
  

  if (!SD.begin(chipSelect)) {
    Serial.println("initialization failed!");
    return -1;
  }
  Serial.println("initialization done.");

  root = SD.open("/"); //open root of sd card

  curSample = root.openNextFile();
  while(curSample.isDirectory()) //loop until an actual sample is found, not a directory
  {
    curSample.close();
    curSample = root.openNextFile();
    }
  Serial.println(curSample.name());
  nextSample = root.openNextFile();
  while(nextSample.isDirectory()) //loop until an actual sample is found, not a directory
  {
    nextSample.close();
    nextSample = root.openNextFile();
    }
  Serial.println(nextSample.name());
  
  Serial.println("ILI9341 Test!");
  tft.begin();
  tft.fillScreen(ILI9341_BLACK); 
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);  // White on black
  tft.setTextWrap(false);  // Don't wrap text to next line
  tft.setTextSize(6);  // large letters
  tft.setRotation(3); // horizontal display
  testText();

//get rid of things below if never works

  //create a while loop to look for low value at pin 33 then go to sampleselect()
  uint32_t clicked; //determine is button is being pressed

  
  while(1){
    GPIO9_DR_CLEAR = (1<<7);//clears the bit in the DR register

    clicked = (GPIO9_DR >> 7) & 1; ////create bit mask for just GPIO4_IO3
    //Serial.println(clicked);
    if (clicked == 0){ //want this value to be 0 (LOW)
      sampleSelect();//if the button is being pressed, go to sample select
      //Serial.println("Clicked");
    }

  }
}



void testText() {
  tft.fillScreen(ILI9341_BLACK);//creat black background
  //unsigned long start = micros();
  tft.setCursor(0, 0);//set point to top left of screen
  tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(3);
  tft.println("Current Sample:");
//  File entry = dir.openNextFile();
//  if (entry.isDirectory()){
//    entry.close();
//    entry = dir.openNextFile();
//    }
  String sample = curSample.name();
  sample = sample.remove(sample.length() - 4, 4);//remove the file indicator (.txt, .wav, etc)
  if (sample.length() < 7){ //if sample is less than 7 characters, can fit on one line, no need to scroll
    tft.setTextSize(8);
    tft.println(sample);
    }
   if (sample.length() >=7 && sample.length() <9){
    tft.setTextSize(6);
    tft.println(sample);
    }
   if (sample.length() >=9 && sample.length()<13){
    tft.setTextSize(4);
    tft.println(sample);
    }
    if(sample.length() >12){
      tft.setTextSize(3);
      tft.println(sample);
      }

//  tft.setTextWrap(true);
//  tft.setTextSize(2);
//  tft.println("Sample Length");
//  tft.setTextSize(4);
//  tft.println("1:45"); //CALCULATE THIS BY GETTING SIZE OF FILE AND DOING CALCULATION
  tft.println("");
  tft.setTextSize(3);
  tft.println("Next Sample:");
  tft.setTextSize(4);
//  File nextEntry = dir.openNextFile();
//  if (nextEntry.isDirectory()){
//    nextEntry.close();
//    nextEntry = dir.openNextFile();
//    }
  String nextEntry = nextSample.name();
  nextEntry = nextEntry.remove(nextEntry.length()-4, 4);//remove .txt, .wav, etc
   if (nextEntry.length() < 7){ //if sample is less than 7 characters, can fit on one line, no need to scroll
    tft.setTextSize(8);
    tft.println(nextEntry);
    }
   if (nextEntry.length() >=7 && nextEntry.length() <9){
    tft.setTextSize(6);
    tft.println(nextEntry);
    }
   if (nextEntry.length() >=9 && nextEntry.length()<13){
    tft.setTextSize(4);
    tft.println(nextEntry);
    }
    if(nextEntry.length() > 12){
      tft.setTextSize(3);
      tft.println(nextEntry);
      }

}

void sampleSelect(){ //ISR for sample select button pressed
  Serial.println("Entered sampleSelect ISR");
  curSample = nextSample; //current sample becomes next sample
  Serial.println(curSample.name());
  nextSample = root.openNextFile();
  if(!nextSample){
    root.rewindDirectory(); //go back to first file in directory
    nextSample = root.openNextFile();
    }
  while(nextSample.isDirectory()){
    nextSample.close();
    nextSample = root.openNextFile();
    }
  Serial.println(nextSample.name());
  delay(500);//prevent rollover
  testText(); //run display function with updated info
  }