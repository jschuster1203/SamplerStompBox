# SamplerStompBox


A foot controlled guitar sampler that allows a solo guitar player to record themselves playing. On the push of our Record button, the guitar's audio begins to be sampled by our MAX1169 Analog-to-Digital Converter(ADC). The ADC samples the guitar signal at 44.1kHz 16-bit resolution. This information is retrieved via I2C. The digital guitar signal data is then saved into a created WAV file that lives on a 32GB SD card. On the next press of our Record button, our system stops sampling the guitar data, the proper WAV file header is written based on the amount of data saved to the file, and the file is closed and created. All of the WAV files present on the SD card can be shown on the ILI9341 LCD display. The display shows the currently selected file as well as the next file in order. The user can either scroll forward or backwards through the SD card files using the Next Sample Scroll or Previous Sample Scroll buttons. When the user wants to play back a file, they scroll so that file is the "Current Sample" on the display and they press the Playback button. Once pressed, the selected WAV file has its audio data sent through the 16 bit DAC8563 Digital-to-Analog Converter(DAC) via SPI. This analog data is sent to an audio mixer which combines live guitar audio with the recorded audio giving the player the dual guitar sound they are looking for that they can't get by themselves. 
Our record button acts as a true bypass that alows the guitar player to either send their guitar audio to our system to be recorded or to bypass our system, sending the audio straight through the mixer and to the amp. All of our buttons used in our system are large 3-Pole-Double-Throw stompbox switches to allow for easy foot control. Finally, our system allows the user to remove the SD card, which has all of the audio files, from our system. This allows the user to plug the SD card into their own computer and do whatever audio trimming, processing, combmining, etc. that they are looking to do. They can then put the SD card back into our system, and the updated audio file will be displayed and ready for playback in our system. Currently, our system is unable to properly playback the audio as more noise filtering is needed to produce a clean, non-noisy audio signal. 

Sample Scroll Demo:

https://user-images.githubusercontent.com/54950513/167186175-ad26f06d-db37-448c-bc24-86445880ebb1.mov

Sample Record and Sample Playback Demo:

Sample Record Result:

https://user-images.githubusercontent.com/54950513/167186325-9005e98a-b89e-404d-8e2d-4f33d743a2e5.mov

