#include <Arduino.h>
#include <Wire.h>
#include <SD.h>

uint16_t databits;//data from the ADC to be saved
uint32_t startTime;//start time for the conversions
File wavFile;
unsigned long dataSize;
uint32_t convcount = 0;//used to count the number of conversions

void writeWavHeader(void);

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
uint32_t subChunk2Size = 0; // You Don't know this until you write your data

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB
  }
  if(!SD.begin(BUILTIN_SDCARD)){
    Serial.println("SD card not in");
    exit(0);
  }

  wavFile = SD.open("ADC1.wav", FILE_WRITE);
  

  Serial.println("Serial going");
  Wire.begin();//begin wire with slave address of ADC
  Wire.setClock(1000000);
  
}

void loop() {

  Wire.beginTransmission(0b0110111);//begin transmission with ADC
  Wire.write(0x08);
  Wire.endTransmission();//may be able to not transmit this false

  Wire.requestFrom(0b0110111, 2,false);
  if(Wire.available() >= 2){ //check for all samples to be there, see if it reloads itself after running out of data, request read once?
    databits = Wire.read(); //inside while loop, check how many bytes in buffer?
    databits = databits << 8;
    databits |= Wire.read();
    wavFile.write((byte*)&databits,2);
    convcount = convcount + 1;
   }
   
  startTime = millis();//current time
   
  while(millis() - startTime < 4000){//try only requesting once, what does the 2 do, 
    Wire.requestFrom(0b0110111, 32,false);//request 32 bytes (16 samples), 
    //delayMicroseconds(6);
    if(Wire.available() >= 32){ //check for all samples to be there, see if it reloads itself after running out of data, request read once?
      for(int i =0; i<16;i++){//write 16 samples to the SD card
      databits = Wire.read(); //inside while loop, check how many bytes in buffer?
      databits = databits << 8;
      databits |= Wire.read();
      wavFile.write((byte*)&databits,2);
      convcount = convcount + 1;

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
    

;    
  Serial.println(convcount/4);
  dataSize = wavFile.size();
  chunkSize = dataSize+36;

  writeWavHeader();

   wavFile.close();
   Wire.end();
   Serial.println("Transaction ended");
   exit(0);
  
  
}

void writeWavHeader(){
  
   //Serial.println("Entered writeWavHeader");
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
}