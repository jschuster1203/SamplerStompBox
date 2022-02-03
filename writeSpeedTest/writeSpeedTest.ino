#include <SPI.h>
#include <SD.h>

File testFile;

unsigned long timer;
uint32_t value = 0xFFFFFFFF;


void setup() {
  // put your setup code here, to run once:

  Serial.begin(9600);
  while(!Serial);
  
  if (!SD.begin(BUILTIN_SDCARD)){ //SD card not in slot
    Serial.println("initialization failed!");
    exit(0);}
  
  testFile = SD.open("writeTestFile.txt", FILE_WRITE);

 
}

void loop() {
  // put your main code here, to run repeatedly:
  timer=millis(); //timer used to determine how long has passed for writing
  while(millis()-timer <5000){ //write to the sd card for 5 seconds
    testFile.write((byte*)&value,3);//write 3 bytes of data
    }
  
  
  int fileSize = testFile.size();
  Serial.println(fileSize);
  Serial.println(fileSize/5);   //THE WRITE RATE OF THIS FILE IS 
  testFile.close();
  exit(0);
}
