#include <SPI.h>
#include <SD.h>
#include <ADC.h>
#include <ADC_Module.h>

int MIN_DATA_VALUE;
int MAX_DATA_VALUE;
File wavFile;
//const char* filename = "data.wav";
const int readPin = A16; //ADC0
const int chipSelect = BUILTIN_SDCARD;
int adcResult;
//String dataString = "";//used to store all adc data
char myData[]="";
long numSample;//overall number of samples done by the adc
unsigned long StartTime;
unsigned long StartTime2;
unsigned short Count;

ADC *adc = new ADC();//adc object

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
uint32_t sampleRates = 48827; //TEST THIS VALUE TO GET MORE ACCURATE SAMPLERATE
///: SampleRate * NumChannels * BitsPerSample/8
uint32_t byteRate = 48827;
///: The number of byte for one frame NumChannels * BitsPerSample/8
uint16_t blockAlign = 1;
///: 8 bits = 8, 16 bits = 16
uint16_t bitsPerSample = 8;
///: Contains the letters "data"
char subChunk2ID[4] = {'d', 'a', 't', 'a'};
///: == NumSamples * NumChannels * BitsPerSample/8  i.e. number of byte in the data.
uint32_t subChunk2Size = 0; // You Don't know this until you write your data



void setup()
{

  pinMode(readPin, INPUT);
  Serial.begin(9600);
  while (!Serial);

  if (!SD.begin(BUILTIN_SDCARD)){
    Serial.println("initialization failed!");
    return;}
  

  wavFile = SD.open("DA1.wav", FILE_WRITE);

/* maybe change this section to different settings to get easier data to read*/
  adc->adc0->setAveraging(32);
  adc->adc0->setResolution(8);
  adc->adc0->setConversionSpeed(ADC_CONVERSION_SPEED::HIGH_SPEED); // change the conversion speed
  adc->adc0->setSamplingSpeed(ADC_SAMPLING_SPEED::HIGH_SPEED); // change the sampling speed 
  adc->adc0->enableInterrupts(adc0_isr);
  //adc->adc0->enableCompareRange(0,255,1,1);//enable the compare function to a range insize 0 and 255 (8 bits)
  //adc->adc0->setReference(ADC_REFERENCE::REF_3V3);//set voltage reference
  
  writeWavHeader();

  adc->adc0->startContinuous(readPin);
  StartTime = millis();
  
  
}

char c=0;

void loop(){
  
  if(millis() - StartTime > 1000){
    Serial.println("entered second if statement");
      adc->adc0->stopContinuous();

      wavFile.close();
      Serial.println("Conversion closed, write ended");
      exit(0);
    }
    
    
}

void writeWavHeader(){
  
   Serial.println("Entered writeWavHeader");
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
   wavFile.write((byte*)&subChunk2Size,4);
}


void adc0_isr(void){ //interrupt for when a adc read is available
  adcResult = adc->adc0->analogReadContinuous();//gives most recent adc conversion

  int8_t sampleValue = map(adcResult, 0, 255, 0, 255);
  subChunk2Size += numChannels * bitsPerSample/8;
  wavFile.seek(40);
  wavFile.write((byte*)&subChunk2Size,4);

  wavFile.seek(4);
  chunkSize = 36+subChunk2Size;
  wavFile.write((byte*)&chunkSize,4);

  wavFile.seek(wavFile.size() - 1);
  wavFile.write((byte*)&sampleValue,1);


  
  //writeDataToWavFile(adcResult);//write value to adc result
//  
//  //uint8_t bitCount = sizeof(adcResult)*8;
//  //char str[bitCount + 1];
//
//  //itoa(adcResult, str, 2);
//  //Serial.println(str);
//  
//  //byte test = (byte*)&adcResult;
//  //Serial.println(adcResult);
//  //String temp = String(adcResult,BIN);
//  //wavFile.seek(wavFile.size()-1);
//  wavFile.println(adcResult);//write each adc result into wav file
////  ++numSample;
//  if(millis() - StartTime2 < 1000){
//    ++Count;
//    }
//  else{
//    Serial.println(Count);
//    Count = 0;
//    StartTime2 = millis();
//    }
//  //Serial.println("ADC conversion complete");
  }
