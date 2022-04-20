#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <SD.h>
#include <SPI.h>
#include <Wire.h>

// For the Adafruit shield, these are the default.
#define TFT_DC 9
#define TFT_CS 10
#define selectPin 33

void displayText(void);
void sampleSelect(void);
void prevSampleSelect(void);
void sampleRecall(void);
void sampleRecord(void);

const int chipSelect = BUILTIN_SDCARD;

//File root = SD.open("/"); //open root of sd card
File root; //root of sd card
File curSample; //current sample
File nextSample;
int curFile;//the current number file

File startSample;//used to get the number of files on the SD card
int totFiles;//

int filesCreated = 1;//going to be used to count the number of files created by the user in the recording button
String fileNameStart = "DA";//the start of the file name
  

// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);


int main(){
  // Serial.begin(9600);

  //   while (!Serial) {
  //   ; // wait for serial port to connect. Needed for native USB
  // }

  //INITIALIZE BUTTON REGISTERS
   
  IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_07 = 5; //set pin 33 to GPIO mode (next Sample)
  IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_07 = IOMUXC_PAD_DSE(7) | IOMUXC_PAD_PKE | IOMUXC_PAD_PUE | IOMUXC_PAD_PUS(3) | IOMUXC_PAD_HYS; //settings for input pullup
  //drive strength 7,  pull/keeper enable, pull selected, 100k Ohm Pull up, enable hysteresis
  GPIO9_GDIR &= ~(1<<7);//GPIO9_IO7 set to input 

  IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_13 = 5;//set pin 34 to GPIO mode (Prev sample)
  IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_13 = IOMUXC_PAD_PUS(3) | IOMUXC_PAD_PUE | IOMUXC_PAD_PKE | IOMUXC_PAD_DSE(7) | IOMUXC_PAD_HYS;
  GPIO7_GDIR &= ~(1<<29);//set pin 34 to input

  IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_12 = 5;//set pin 35 to GPIO mode(sample recall)
  IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_12 = IOMUXC_PAD_PUS(3) | IOMUXC_PAD_PUE | IOMUXC_PAD_PKE | IOMUXC_PAD_DSE(7) | IOMUXC_PAD_HYS;
  GPIO7_GDIR &= ~(1<<28);//set pin 35 to input

  IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_02 = 5;//set pin 36 to GPIO mode(sample record)
  IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_02 = IOMUXC_PAD_PUS(3) | IOMUXC_PAD_PUE | IOMUXC_PAD_PKE | IOMUXC_PAD_DSE(7) | IOMUXC_PAD_HYS;
  GPIO7_GDIR &= ~(1<<18);//set pin 36 to input

  /////////////// END OF INITIALIZING BUTTONS ///////////////////////////////
  

  if (!SD.begin(chipSelect)) { //check if SD card is in teensy
    //Serial.println("Failed");
    tft.begin(); 
    tft.fillScreen(ILI9341_BLACK); 
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);  // White on black
    tft.setTextWrap(false);  // Don't wrap text to next line
    tft.setTextSize(4);  // large letters
    tft.setRotation(3); // horizontal display
    tft.println("SD card not\ninserted, \nplease insert \nand restart");

    return -1;
  }

  //////////////// BELOW SETS INITIAL SD CARD FILE SAMPLES///////////////////////////

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

/////////////////////////// FINISHED SETTING UP INITAL SD CARD FILES /////////////////////////////

  tft.begin(); 
  tft.fillScreen(ILI9341_BLACK); 
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);  // White on black
  tft.setTextWrap(false);  // Don't wrap text to next line
  tft.setTextSize(6);  // large letters
  tft.setRotation(3); // horizontal display
  displayText();
  
  ///////////// VALUES TO CHECK CURRENT AND PREVIOUS BUTTON STATES //////////////////////

  uint32_t nextclicked; //determine is button is being pressed, next sample button
  uint32_t nextprevClicked = (GPIO9_DR >> 7) & 1;//previously clicked state, next sample button

  uint32_t prevclicked;//previous sample scroll button pressed, current state
  uint32_t prevprevClicked = (GPIO7_DR >> 29) & 1;//previous state of previous sample scroll button

  uint32_t recallClicked;//recall sample button current state
  uint32_t recallPrevClicked = (GPIO7_DR >> 28) & 1;//previous state of recall sample button

  uint32_t recordClicked;//record sample button state
  uint32_t recordPrevClicked = (GPIO7_DR >> 18) & 1;//previous state of record button

  while(1){
    GPIO9_DR_CLEAR = (1<<7);//clears the bit in the DR register
    GPIO7_DR_CLEAR = (1<<29);
    GPIO7_DR_CLEAR = (1<<28);
    GPIO7_DR_CLEAR = (1<<18);

    nextclicked = (GPIO9_DR >> 7) & 1; //get current value of each of the buttons
    prevclicked = (GPIO7_DR >> 29) & 1;
    recallClicked = (GPIO7_DR >> 28) & 1;
    recordClicked = (GPIO7_DR >> 18) & 1;

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
    if (recallClicked != recallPrevClicked){
      recallPrevClicked = recallClicked;//set previous state to current state
      //Serial.println("Recall clicked");
      sampleRecall();
      delay(500);
      //PUT RECALL FUNCTION HERE
    }
    if (recordClicked != recordPrevClicked){
      //recordPrevClicked = recordClicked;
      //Serial.println("Record clicked");
      //delay(500);
      sampleRecord();
    }
    else{//no button pressed
      nextprevClicked = nextclicked;//remember last state of button press
      prevprevClicked = prevclicked;
      recallPrevClicked = recallClicked;
      recordPrevClicked = recordClicked;
    }

  }


}


/////////////////////////// USED FUNCTION BELOW ///////////////////////////////////

void displayText() {//function used to display current and previous sample on the tft display
  tft.fillScreen(ILI9341_BLACK);//creat black background
  //unsigned long start = micros();
  tft.setCursor(0, 0);//set point to top left of screen
  tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(3);
  tft.println("Current Sample:");

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

  tft.println("");//set up "Next Sample" text line
  tft.setTextSize(3);
  tft.println("Next Sample:");
  tft.setTextSize(4);

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

  delay(500);//prevent rollover
  displayText(); //run display function with updated info
  }


void prevSampleSelect(){//called when previous sample button is pressed
  int targetFile = curFile - 1;//get the previous file in order
  //nextSample = curSample;//the current sample becomes the next sample
  if(targetFile ==0){//trying to find previous file of first file on sd card, need to get the very last file on the sd card
    targetFile = totFiles;
  }
  curFile = targetFile;

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
  delay(500);//prevent overlap
  displayText();

}

void sampleRecall(){
  File datafile; //the file that will be read from
  byte firstRead; //first read from wav file
  byte secondRead;


  pinMode(0, OUTPUT);//SYNC pin, (CS)
  digitalWrite(0, HIGH);//make sure CS stays high
  delayMicroseconds(100);

  datafile = curSample;//make the current file, the file thats going to be read

  tft.fillScreen(ILI9341_BLACK); 
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);  // White on black
  tft.setTextWrap(true);  //  wrap text to next line
  tft.setTextSize(6);  // large letters
  tft.setRotation(3); // horizontal display

  tft.setCursor(0,0);//set point to top left 
  tft.println("Playing:");
  tft.println(curSample.name());
  //delay(5000);

//   /////////////// SET UP OF SPI AND THE DAC ///////////////////

  SPI1.begin();

  SPI1.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE1));//highest possible frequency, put 50MHz for DAC
  digitalWrite(0,LOW);
  SPI1.transfer(0x27);//power up DAC
  SPI1.transfer(0x00);
  SPI1.transfer(0x01); 
  //delayNanoseconds(100);
  digitalWrite(0,HIGH);
  SPI1.endTransaction();
 // delayMicroseconds(20);

  SPI1.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE1));
  digitalWrite(0,LOW);
  SPI1.transfer(0x30);//disable LDAC pin
  SPI1.transfer(0x00);
  SPI1.transfer(0x03); 
  //delayNanoseconds(100);
  digitalWrite(0,HIGH);
  SPI1.endTransaction();
  //delayMicroseconds(20);

  SPI1.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE1));
  digitalWrite(0,LOW);
  SPI1.transfer(0x38);//enable internal reference
  SPI1.transfer(0x00);
  SPI1.transfer(0x01); 
  digitalWrite(0,HIGH);
  SPI1.endTransaction();
  //delayMicroseconds(20);

  SPI1.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE1));
  digitalWrite(0, LOW);
  SPI1.transfer(0x02); //set gain on DAC-A to 1 (see if this minimizes noise)
  SPI1.transfer(0x00);
  SPI1.transfer(0x01);
  digitalWrite(0, HIGH);
  SPI1.endTransaction();

  SPI1.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE1));
  digitalWrite(0,LOW);
  SPI1.transfer(0x28);//reset all registers and update DAC (initalization)
  SPI1.transfer(0x00);
  SPI1.transfer(0x01); 
  digitalWrite(0,HIGH);
  SPI1.endTransaction();

  datafile.seek(44);//skip header and go to actual wav data
  firstRead = datafile.read();
  secondRead = datafile.read();

  ///////// END OF DAC INITIALIZATION, START RECALLING AUDIO ///////////

  while(datafile.available() && (firstRead != -1) && (secondRead != -1)){//run until nothing left to read from wav file
    SPI1.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE1));
    //24 bit wide register, for 8 bits are 2 dont care bits, then 3 command bits, then three address bits, the next 16 bits are the data bits
    //delayMicroseconds(2);
    digitalWrite(0,LOW);
    SPI1.transfer(0x18);//2 dont care bits, then write to input register n, then pick input register DAC-A
    SPI1.transfer((firstRead));
    SPI1.transfer(secondRead);
    //delayNanoseconds(100);
    digitalWrite(0,HIGH);
    SPI1.endTransaction();
   
    firstRead = datafile.read();
    secondRead = datafile.read();
    delayMicroseconds(26); // DELAY HERE to match sampling rate. 25.641025641026  for 39kHz

  //Serial.println("Done");
  }
  tft.fillScreen(ILI9341_BLACK); 
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);  // White on black
  tft.setTextWrap(true);  //  wrap text to next line
  tft.setTextSize(6);  // large letters
  tft.setRotation(3); // horizontal display

  tft.setCursor(0,0);//set point to top left 
  tft.println("Done    Playing:");
  tft.println(curSample.name());
  delay(2000);
  sampleSelect();//move on to next file in order
  displayText();
  
}

void sampleRecord(){
  delay(500);
  uint16_t databits;//data from the ADC to be saved
  uint32_t convcount = 0;//used to count the number of conversions
  unsigned long dataSize;

///////////////// WAV FILE HEADER INFO ////////////////////
  /// The first 4 byte of a wav file should be the characters "RIFF" */
  char chunkID[4] = {'R', 'I', 'F', 'F'};
  /// 36 + SubChunk2Size
  uint32_t chunkSize = 36; // You Don't know this until you write your data but at a minimum it is 36 for an empty file
  /// "should be characters "WAVE"
  char format[4] = {'W', 'A', 'V', 'E'};
  /// " This should be the letters "fmt ", note the space character
  char subChunk1ID[4] = {'f', 'm', 't', ' '};
  ///: For PCM == 16, since audioFormat == uint16_t
  uint32_t subChunk1Size = 16;
  ///: For PCM this is 1, other values indicate compression
  uint16_t audioFormat = 1;
  ///: Mono = 1, Stereo = 2, etc.
  uint16_t numChannels = 1;
  ///: Sample Rate of file
  uint32_t sampleRates = 38940; 
  ///: SampleRate * NumChannels * BitsPerSample/8
  uint32_t byteRate = 77880; //samplerates*2
  ///: The number of byte for one frame NumChannels * BitsPerSample/8
  uint16_t blockAlign = 2;
  ///: 8 bits = 8, 16 bits = 16
  uint16_t bitsPerSample = 16;
  ///: Contains the letters "data"
  char subChunk2ID[4] = {'d', 'a', 't', 'a'};
  ///: == NumSamples * NumChannels * BitsPerSample/8  i.e. number of byte in the data.
  //uint32_t subChunk2Size = 0; // You Don't know this until you write your data


  //tft.begin(); //start display
  tft.fillScreen(ILI9341_BLACK); 
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);  // White on black
  tft.setTextWrap(true);  //  wrap text to next line
  tft.setTextSize(5);  // large letters
  tft.setRotation(3); // horizontal display

  tft.setCursor(0,0);//set point to top left 
  tft.println("Recording in        Progress.");
  tft.println("Please    wait");

  uint32_t recordFuncClicked;//record sample button state
  uint32_t recordFuncPrevClicked = (GPIO7_DR >> 18) & 1;//previous state of record button


  String fileNameWhole = fileNameStart + filesCreated + ".wav"; //creates a string of an incrementing name that will be used as the file name
  const char* fileName= fileNameWhole.c_str();
  //Serial.println(fileName);
  filesCreated = filesCreated +1;//increment the file name number

  File wavFile = SD.open(fileName, FILE_WRITE);

  Wire.begin();//begin wire with slave address of ADC
  Wire.setClock(1000000);

  Wire.beginTransmission(0b0110111);//begin transmission with ADC
  Wire.write(0x08);
  Wire.endTransmission();//may be able to not transmit this false

  Wire.requestFrom(0b0110111, 2,false);
  if(Wire.available() >= 2){ //check for all samples to be there, see if it reloads itself after running out of data, request read once?
    databits = Wire.read(); //inside while loop, check how many bytes in buffer?
    databits = databits << 8;
    databits |= Wire.read();
    wavFile.write((byte*)&databits,2);
    
   }

   //Serial.println("Entered recording");

  GPIO7_DR_CLEAR = (1<<18);//clear record button bit
  recordFuncClicked = (GPIO7_DR >> 18) & 1;

  while(1){//check for another button press to end the recording 
  

    GPIO7_DR_CLEAR = (1<<18);//clear record button bit
    recordFuncClicked = (GPIO7_DR >> 18) & 1;
    if(recordFuncClicked != recordFuncPrevClicked){
      break;
    }
    Wire.requestFrom(0b0110111, 32,false);//request 32 bytes (16 samples), 
    if(Wire.available() >= 32){ //check for all samples to be there, see if it reloads itself after running out of data, request read once?
      for(int i =0; i<16;i++){//write 16 samples to the SD card
      databits = Wire.read(); //inside while loop, check how many bytes in buffer?
      databits = databits << 8;
      databits |= Wire.read();
      wavFile.write((byte*)&databits,2);
      convcount = convcount + 1;
       recordFuncPrevClicked = recordFuncClicked;

      }
    }
  }

  Wire.requestFrom(0b0110111, 32,true); //request 2 bytes (16 bits), if master generates a STOP condition, the ADC returns to F/S mode
  while(Wire.available() < 32); //check for all samples to be there, see if it reloads itself after running out of data, request read once?
    for(int i =0; i<16;i++){//write 16 samples to the SD card
    databits = Wire.read(); //inside while loop, check how many bytes in buffer?
    databits = databits << 8;
    databits |= Wire.read();
    wavFile.println(databits);
    wavFile.write((byte*)&databits,2);
    convcount = convcount + 1;
    }

  //Serial.println("Existed recording");

  dataSize = wavFile.size();
  chunkSize = dataSize+36;

  ////////WRITE HEADER TO WAV FILE //////////////

  wavFile.seek(0);
  wavFile.write(chunkID,4);
  wavFile.write((byte*)&chunkSize,4);
  wavFile.write(format,4);
  wavFile.write(subChunk1ID,4);
  wavFile.write((byte*)&subChunk1Size,4);
  wavFile.write((byte*)&audioFormat,2);
  wavFile.write((byte*)&numChannels,2);
  wavFile.write((byte*)&sampleRates,4);
  wavFile.write((byte*)&byteRate,4);
  wavFile.write((byte*)&blockAlign,2);
  wavFile.write((byte*)&bitsPerSample,2);
  wavFile.write(subChunk2ID,4);
  wavFile.write((byte*)&dataSize,4);

  wavFile.close();//close the wav file and end the wire
  Wire.end();

  tft.fillScreen(ILI9341_BLACK); 
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);  // White on black
  tft.setTextWrap(true);  //  wrap text to next line
  tft.setTextSize(5);  // large letters
  tft.setRotation(3); // horizontal display

  tft.setCursor(0,0);//set point to top left 
  tft.println("Recording Complete!");
  delay(2000);
  displayText();

}


