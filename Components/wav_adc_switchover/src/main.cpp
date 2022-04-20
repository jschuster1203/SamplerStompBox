//#include <SPI.h>
#include <SD.h>
//#include <ADC.h>
//#include <ADC_Module.h>

File wavFile;
File dataFile;

//declare functions
void writeWavHeader(void);
void adc0_isr(void);

unsigned long dataSize; //the size of data in bytes, will be used to calculate the chunk sizes for the wav file
const int readPin = A17; //pin 41
const int chipSelect = BUILTIN_SDCARD;
const uint32_t clkPerSec= 414511690;//number of cycles per second
uint16_t adcResult;
long numSample;//overall number of samples done by the adc, (not used)
unsigned long StartTime; //used for overall sampling time done by adc
unsigned long StartTime2; //used for timing 1 second in finding the amount of samples per second
unsigned short Count; //counts the actual amount of samples per second
uint32_t cycle; //used to count number of cycles before ending the ADC
//ADC *adc = new ADC();//adc object

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
uint32_t sampleRates = 46874; //6368 on low speed
///: SampleRate * NumChannels * BitsPerSample/8
uint32_t byteRate = 93748; //samplerates*2
///: The number of byte for one frame NumChannels * BitsPerSample/8
uint16_t blockAlign = 2;
///: 8 bits = 8, 16 bits = 16
uint16_t bitsPerSample = 16;
///: Contains the letters "data"
char subChunk2ID[4] = {'d', 'a', 't', 'a'};
///: == NumSamples * NumChannels * BitsPerSample/8  i.e. number of byte in the data.
uint32_t subChunk2Size = 0; // You Don't know this until you write your data

int main(){
  
  IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_05 = 16;//set pin 41 to forced input path
 
  //Serial.begin(9600);
  //while (!Serial);

  if (!SD.begin(BUILTIN_SDCARD)){ //SD card not in slot
    //Serial.println("initialization failed!");
    return -1;}
  
  

  wavFile = SD.open("DA2.wav", FILE_WRITE);//Once done writing data to text file, need to close this then re-open as read
  //Serial.println("Starting system");
  

  //CHANGE THIS TO MAKE SLOWER, SEE SETSPEED FUNCTIONS BELOW FOR ADCK+ VALS
  ADC1_CFG = ADC_CFG_ADICLK(0) | ADC_CFG_MODE(2) | ADC_CFG_ADLSMP | ADC_CFG_ADIV(1) | ADC_CFG_ADLPC | ADC_CFG_ADSTS(3) | ADC_CFG_AVGS(3) | ADC_CFG_OVWREN;
  ADC1_GC = ADC_GC_CAL; //enable continuous conversions and set calibration bit
  while(((ADC1_GC >> 7) & 1) == 1);//wait for calibration to end before starting ADC
  if (((ADC1_GS >> 1) & 1) ==1){//check CALF bit
   //Serial.println("Calibration failed");
   return -1;
  }
  if ((ADC1_HS & 1) == 1){//check COCO0 bit
   Serial.println(ADC1_HS);
   //Serial.println("Calibration Complete");
  }
  else{
    //Serial.println("Calibration Failed");
    return -1;
  }
  adcResult = ADC1_R0; //used to clear
  ADC1_CFG = ADC_CFG_ADICLK(0) | ADC_CFG_MODE(2) | ADC_CFG_ADLSMP | ADC_CFG_ADIV(1) | ADC_CFG_ADLPC | ADC_CFG_ADSTS(3) | ADC_CFG_AVGS(3) | ADC_CFG_OVWREN;
  ADC1_GC = ADC_GC_ADCO | ADC_GC_AVGE;
  ADC1_HC0 = ADC_HC_AIEN | ADC_HC_ADCH(10); //enable conversion complete interrupt and enable external channel 10 input (pin 41, AD_B1_05)  

  attachInterruptVector(IRQ_ADC1, adc0_isr); //Attatch an inpterrupt to ADC1 to go to ADC1_ISR function when a conversion is complete
  NVIC_SET_PRIORITY(IRQ_ADC1, 255);
  NVIC_ENABLE_IRQ(IRQ_ADC1);

  /* maybe change this section to different settings to get easier data to read*/
  // adc->adc0->setAveraging(32);
  // adc->adc0->setResolution(12);
  // adc->adc0->setConversionSpeed(ADC_CONVERSION_SPEED::LOW_SPEED); // change the conversion speed
  // adc->adc0->setSamplingSpeed(ADC_SAMPLING_SPEED::LOW_SPEED); // change the sampling speed 
  // // conv and sample speed very low = 6970(lower speed used for easier analysis
  // // Both sampling and conversion speed high = 48827 sample/sec
  
  // adc->adc0->enableInterrupts(adc0_isr);
  //adc->adc0->enableCompareRange(0,255,1,1);//enable the compare function to a range insize 0 and 255 (8 bits)
  //adc->adc0->setReference(ADC_REFERENCE::REF_3V3);//set voltage reference

  //adc->adc0->startContinuous(readPin);

  //StartTime2 = millis();//used for getting sampling rate, can delete after having a final set sample rate
  
  while(1){
    cycle=cycle+1;
    if(cycle > 5 * clkPerSec){//stop after 5 seconds
      //Serial.println(cycle/5);//number of cycles per second
      //Serial.println("entered second if statement");
      //adc->adc0->stopContinuous();
      ADC1_HC0 = ADC_HC_ADCH(32); //disable conversion complete interrupt and disable conversion

      dataSize = wavFile.size(); //this gets the amount of data stored in the wav file, this will be then used to create accurate header size values
      chunkSize = dataSize+36;

      writeWavHeader();

      //Serial.println("Header completr");
     
      wavFile.close(); //close wav file to save
      Serial.println("Wave file completed, shutting down");
      return 1;

    }

  }
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


void adc0_isr(void){ //interrupt for when a adc read is available
  //adcResult = adc->adc0->analogReadContinuous();//gives most recent adc conversion
  adcResult = ADC1_R0;//will take the lower 16 bits (test to make sure)
  while((ADC1_HS & 1) == 1){//wait until flag is cleared FLAG IS NOT GETTING CLEARED
   ADC1_HC0 = ADC_HC_AIEN; 
  }

  wavFile.write((byte*)&adcResult,2);//write one byte worth of data to the wav file

  //THIS IS FOR TESTING THE SAMPLE RATE

//  if(millis() - StartTime2 < 1000){//if it has not been a second, keep counting the number of adc conversions in a second
//    ++Count;
//    }
//  else{
//    Serial.println(Count); //if it has been a minute, print the number of samples to the serial monitor
//    Count = 0;
//    StartTime2 = millis();
//    }
  //Serial.println("ADC conversion complete");
  }