
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <SD.h>

//For converting to the latch button, have one side of switch set to low, read the input coming from that switch,
// remember the previous state, if the new state is different then the old state, a button was pressed, use internal pullup

//For creating the backwards button, get the number of files on the sd card, remember the number file that are currently on, 
//restart from begining of sd card and openNextFile() currentfile-1 times to get previous.



// For the Adafruit shield, these are the default.
#define TFT_DC 9
#define TFT_CS 10
#define selectPin 33

void testText(void);
void sampleSelect(void);
void prevSampleSelect(void);

const int chipSelect = BUILTIN_SDCARD;

//File root = SD.open("/"); //open root of sd card
File root; //root of sd card
File curSample; //current sample
File nextSample;
int curFile;//the current number file

File startSample;//used to get the number of files on the SD card
int totFiles;//
  

// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
// If using the breakout, change pins as desired
//Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);
int main(){
  Serial.begin(9600);
   
  IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_07 = 5; //set pin 33 to GPIO mode (next Sample)
  IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_07 = IOMUXC_PAD_DSE(7) | IOMUXC_PAD_PKE | IOMUXC_PAD_PUE | IOMUXC_PAD_PUS(3) | IOMUXC_PAD_HYS; //settings for input pullup
  //drive strength 7,  pull/keeper enable, pull selected, 100k Ohm Pull up, enable hysteresis
  GPIO9_GDIR &= ~(1<<7);//GPIO9_IO7 set to input 

  IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_13 = 5;//set pin 34 to GPIO mode (Prev sample)
  IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_13 = IOMUXC_PAD_PUS(3) | IOMUXC_PAD_PUE | IOMUXC_PAD_PKE | IOMUXC_PAD_DSE(7) | IOMUXC_PAD_HYS;
  GPIO7_GDIR &= ~(1<<29);//set pin 34 to input

  if (!SD.begin(chipSelect)) {
    Serial.println("Failed");
    return -1;
  }
  
  //delay(2000);
  root = SD.open("/"); //open root of sd card

//USED TO GET NUMBER OF FILES ON AN SD CARD
  startSample=root.openNextFile();
  while(startSample.isDirectory()) //loop until an actual sample is found, not a directory
  {
    startSample.close();
    startSample = root.openNextFile();
    }
  totFiles=1;//found first file on the SD card
  while(1){
    //Serial.println("Entered loop");
    if(!startSample){//probably wont even enter this part
      break;//no more files found, reached end of 
    }
    if(startSample.isDirectory()){//not an actual file, just a directory
      startSample.close();
      startSample = root.openNextFile();
    }
    else{//actual file found, increment the count
      //Serial.println(startSample.name());
      startSample.close();
      startSample = root.openNextFile();
      totFiles=totFiles+1;
    }
  }
  totFiles = totFiles - 1;
  // Serial.print("total number of files is:");//check to make sure counting number of files correctly
  // Serial.println(totFiles);
  root.rewindDirectory();//start back at begining
  curSample = root.openNextFile();
  while(curSample.isDirectory()) //loop until an actual sample is found, not a directory
  {
    curSample.close();
    curSample = root.openNextFile();
    }
  curFile = 1;//first file

  nextSample = root.openNextFile();
  while(nextSample.isDirectory()) //loop until an actual sample is found, not a directory
  {
    nextSample.close();
    nextSample = root.openNextFile();
    }

  tft.begin();
  tft.fillScreen(ILI9341_BLACK); 
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);  // White on black
  tft.setTextWrap(false);  // Don't wrap text to next line
  tft.setTextSize(6);  // large letters
  tft.setRotation(3); // horizontal display
  testText();


  //create a while loop to look for low value at pin 33 then go to sampleselect()
  uint32_t nextclicked; //determine is button is being pressed
  uint32_t nextprevClicked = (GPIO9_DR >> 7) & 1;//previously clicked state
  uint32_t prevclicked;//previous sample scroll button pressed, current state
  uint32_t prevprevClicked = (GPIO7_DR >> 29) & 1;//previous state of previous sample scroll button

  while(1){
    GPIO9_DR_CLEAR = (1<<7);//clears the bit in the DR register
    GPIO7_DR_CLEAR = (1<<29);

    nextclicked = (GPIO9_DR >> 7) & 1; //create bit mask for just GPIO4_IO3
    prevclicked = (GPIO7_DR >> 29) & 1;
    //Serial.print(clicked);
    //Serial.println(prevClicked);
    //delay(100);
    if (nextclicked != nextprevClicked){ //transition from high to low or vise versa(button pressed)
      //Serial.println("clicked");
      nextprevClicked = nextclicked;//remember last state of button press
      //Serial.println("Next Clicked");
      sampleSelect();//if the button is being pressed, go to sample select
      
    }
    if (prevclicked != prevprevClicked){
      prevprevClicked = prevclicked;
      //Serial.println("Previous clicked");
      prevSampleSelect();
    }
    else{
      nextprevClicked = nextclicked;//remember last state of button press
      prevprevClicked = prevclicked;
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
  
  curSample = nextSample; //current sample becomes next sample
  curFile = curFile + 1;//increment the number of file
  if(curFile > totFiles){
    curFile = 1;
  }
  nextSample = root.openNextFile();
  if(!nextSample){
    root.rewindDirectory(); //go back to first file in directory
    nextSample = root.openNextFile();
    //curFile = 1;//reset the current file number back to begining
    }
  while(nextSample.isDirectory()){
    nextSample.close();
    nextSample = root.openNextFile();
    }
  // Serial.print("The new current file number is:");
  // Serial.println(curFile);
  delay(500);//prevent rollover
  testText(); //run display function with updated info
  }

void prevSampleSelect(){
 
  int targetFile = curFile - 1;//get the previous file in order
  //nextSample = curSample;//the current sample becomes the next sample
  
  if(targetFile ==0){//trying to find previous file of first file on sd card, need to get the very last file on the sd card
    targetFile = totFiles;
  }
  curFile = targetFile;
  // Serial.print("The new current file number should be: ");
  // Serial.println(targetFile);
  // Serial.print("The current sample name before iterating is:");
  // Serial.println(curSample.name());

  //curSample.close();//close any current file
  root.rewindDirectory();//start from begining of sd card
  curSample = root.openNextFile();
  while(curSample.isDirectory()){//loop until its not a directory
    curSample.close();
    curSample = root.openNextFile();
    }

 // int countFile = 1;
  int i;
  for(i=2; i<=targetFile;i++){//try <= next time if < doesnt work
    if(curSample.isDirectory()){
      while(curSample.isDirectory()){//loop until not a directory
        curSample.close();
        curSample = root.openNextFile();
      }
    }
    else{//not a directory so just open next file
      curSample.close();
      curSample = root.openNextFile();
    }
  }
  nextSample = root.openNextFile();
  if(!nextSample){//end of the sd card, go back to begining
    root.rewindDirectory();
    nextSample = root.openNextFile();
  }
  if(nextSample.isDirectory()){//loop until not directory
    while(nextSample.isDirectory()){
      nextSample.close();
      nextSample = root.openNextFile();
    }
    
  }

 // Serial.print("The final current sample name is");
 // Serial.println(curSample.name());

  delay(500);//prevent overlap
  testText();

}