#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

const int chipSelect = BUILTIN_SDCARD;
uint16_t countUp=0;
File datafile;
byte firstRead; //first read from wav file
byte secondRead;

uint32_t count=0;

unsigned long startTime;


void setup() {
  // put your setup code here, to run once:
  //Serial.begin(9600);
  //delay(2000);

  pinMode(0, OUTPUT);//SYNC pin, (CS)
  digitalWrite(0, HIGH);//make sure CS stays high
  delayMicroseconds(100);
  //pinMode(27, OUTPUT);//SCLK, clock
  //pinMode(26, OUTPUT); //MOSI pin 

  if (!SD.begin(BUILTIN_SDCARD)){ //SD card not in slot
    Serial.println("initialization failed!");
    exit(0);//end 
    }
  // Serial.println("Enter setup");
  datafile = SD.open("DA6.wav", FILE_READ);

  //SPI1.setCS(0);
  //SPI1.setMISO(1);
 // SPI1.setMOSI(26);
  //SPI1.setSCK(27);
 // Serial.println(SPI1.pinIsSCK(27));

  //begin initializing DAC
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

  //fair amount of time between last configuration SPI transfer and the first 
  //data transfer due to the seek and read commands

}

void loop() {
  //Serial.println("Enter loop");
  // put your main code here, to run repeatedly:
  //startTime = millis();

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
    //GOING TO NEED SOME SORT OF DELAY HERE to match sampling rate. 22.675737 for 44.1kHz
    firstRead = datafile.read();
    secondRead = datafile.read();
    delayMicroseconds(22);
    //delayMicroseconds(20);
    // 
    //countUp = countUp + 1;
    //delayMicroseconds(20);
  //   if(up){countUp = countUp + 1;}
  //   if(!up){countUp = countUp -1;}
  //   if (countUp>=65533){
      
  //     up = 0;//count down
  //     //Serial.println("Done");

  //   }
  //   if (countUp <=4){
  //     up=1;//start counting up
  //   }
  //   //count = count+1;
  //   //datafile.println(dataRec);//print the data to the SD card
  }
  //datafile.close();
  //count = count/5;
  //Serial.print("Total number of samples over 5 seconds: ");
  Serial.println("Done");
  exit(0);//end

}