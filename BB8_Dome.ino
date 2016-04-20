// include SPI, MP3 and SD libraries
#include <SPI.h>
#include <Adafruit_VS1053.h>
#include <SD.h>

//PIN Numbers
#define DOME_LED_LOGIC_1    9
#define DOME_LOGIC_SWITCH   8
#define DOME_LED_PSI        5
#define DOME_SPEAK_SWITCH   2
#define DOME_SPEAKER_IN     0

// These are the pins used for the music maker shield
#define SHIELD_RESET  -1      // VS1053 reset pin (unused!)
#define SHIELD_CS     7      // VS1053 chip select pin (output)
#define SHIELD_DCS    6      // VS1053 Data/command select pin (output)

// These are common pins between breakout and shield
#define CARDCS 4     // Card chip select pin
// DREQ should be an Int pin, see http://arduino.cc/en/Reference/attachInterrupt
#define DREQ 3       // VS1053 Data request, ideally an Interrupt pin

Adafruit_VS1053_FilePlayer soundPlayer = 
  Adafruit_VS1053_FilePlayer(SHIELD_RESET, SHIELD_CS, SHIELD_DCS, DREQ, CARDCS);

void setup() {
  Serial.begin(9600);
  
  pinMode(DOME_SPEAK_SWITCH, INPUT);
  pinMode(DOME_LOGIC_SWITCH, INPUT);

  // initialise the music player
  if (! soundPlayer.begin()) { 
     Serial.println(F("Couldn't find VS1053, do you have the right pins defined?"));
     while (1);
  }
  
  randomSeed(analogRead(5));
  
  // initialise the SD card
  SD.begin(CARDCS);
  
  // Set volume for left, right channels. lower numbers == louder volume!
  soundPlayer.setVolume(0,0);

  // audio playing
  soundPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT);  // DREQ int
  
  playRandomVocalic();
}

int voiceBrightness;
int pulseBrightness;
char val;
bool logicLedOn = true;
int logicSwitch = 0;
int lastLogicSwitch = 0;
int speakSwitch = 0;
int lastSpeakSwitch = 0;
int speakerVolume = 0;

void loop() {   
  // Voice light
  if (soundPlayer.playingMusic == true) {
    speakerVolume = analogRead(DOME_SPEAKER_IN);
    
    // Sets the brightness of the light based on the loudness of the voice
    voiceBrightness = map(speakerVolume, 0, 1024, 0, 255);
    
    Serial.print(speakerVolume);
    Serial.print(" > ");
    Serial.print(voiceBrightness);
    Serial.print("\n");

   analogWrite(DOME_LED_PSI, voiceBrightness);
  } else {
    analogWrite(DOME_LED_PSI, 0);
    
    speakSwitch = digitalRead(DOME_SPEAK_SWITCH);
    
    if (speakSwitch != lastSpeakSwitch) {
      if (speakSwitch == HIGH) 
        playRandomVocalic();
        
      // Delay a little bit to avoid bouncing
      delay(50);
    }
    
    logicSwitch = digitalRead(DOME_LOGIC_SWITCH);
    
    if (logicSwitch != lastLogicSwitch)  {
      if (logicSwitch == HIGH) 
        toggleLogicLed();
        
      // Delay a little bit to avoid bouncing
      delay(50);
    }
    
    lastSpeakSwitch = speakSwitch;
    lastLogicSwitch = logicSwitch;
  }
  
  // Slow pulsing light on the side of BB-8's head
  if (logicLedOn) {
    pulseBrightness = 155 + (sin(millis() / 500.00) * 100);
    analogWrite(DOME_LED_LOGIC_1, pulseBrightness);
  } else analogWrite(DOME_LED_LOGIC_1, 0);
  
  if (Serial.available()) {
    val = Serial.read();
    
    switch (val) {
      case 's': 
        playRandomVocalic();
        break;
      case 'o':
        toggleLogicLed();
        break;
    }
  }
}

void playRandomVocalic() {
  File results;
  
  char* vocalic = selectRandomFileFrom(SD.open("/"), results);
  soundPlayer.startPlayingFile(vocalic);
}

void toggleLogicLed() {
  if (logicLedOn) 
    logicLedOn = false;
  else 
    logicLedOn = true;
}

// Function to select random mp3
char* selectRandomFileFrom( File dir, File result ) {
  File entry;
  int count = 0;

  dir.rewindDirectory();

  while ( entry = dir.openNextFile() ) {
    if ( random( count ) == 0 ) {
      result = entry;
    }
    entry.close();
    count++;
  }
  
  Serial.println(result.name());
  
  return result.name();   // returns the randomly selected file name
}
