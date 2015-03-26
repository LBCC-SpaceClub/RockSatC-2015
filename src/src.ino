/*
Arduino code for Linn-Benton Community College's Rocksat-C 2015 experiment.
Author: Levi Willmeth
*/
#include <SdFat.h>
#include <SdFatUtil.h>

#define buffer_max 10   // Max size of each buffer
#define DEBUGGING true  // Debugging mode on/off

// Geiger tube input pins
#define SHUTDOWN1 3
#define SHUTDOWN2 4
#define SHUTDOWN3 5
#define SHUTDOWN4 6

// 6 Geiger tube input pins on Analog A0-A5
// These are read using the PINC block, instead of individual pins.

typedef struct gdata {
  // Each reading has a 4 byte timestamp, and 1 byte of data
  unsigned long time;// Timestamp, limited to 71! minutes
  byte tubes;        // 1 byte holds all 6, bit-sized measurements
};

// 2 buffers, with space for many readings each
gdata buffer[2][buffer_max+1];
// Variables used inside interrupts should be declared volatile
volatile boolean active_buffer = 0;// determines which buffer is ready for input
volatile byte buffer_index = 0;    // Counter to determine when the buffer is full

unsigned long time_max = 60*5*1000;   // Gives us a max run time, in ms.

SdFat sd;
SdFile myFile;
#define FILE_BASE_NAME "LBCC_"
char filename[13] = FILE_BASE_NAME "00.CSV";

// =============================================================================
void write_to_sd(){
  /*
  Takes no parameters, writes active buffer to SD card and toggles from
  active to inactive buffer.
  */
  // Switch between buffers
  active_buffer = !active_buffer;
  buffer_index = 0;
    
  if (!myFile.open("output.txt", O_RDWR | O_CREAT | O_AT_END)) {
    Serial.println(F("Output file failed to open."));
  } else {
    for(int i=0; i<buffer_max; i++){
        myFile.print(buffer[!active_buffer][i].time);
        myFile.print(F("\t"));
        myFile.println( (buffer[!active_buffer][i].tubes), BIN);
        // Debugging
        if(DEBUGGING){
          Serial.print(buffer[!active_buffer][i].time);
          Serial.print(F("\t"));
          Serial.println( (buffer[!active_buffer][i].tubes), BIN);
        }
    }
    myFile.close();
  }
}

// =============================================================================
void readTubes(){
  /*
  Takes no parameters, stores time and state of each tube into the buffer.
  */
  buffer[active_buffer][buffer_index].time = micros();
  // PINC returns the state of all pins on port C, which includes A0 to A5
  buffer[active_buffer][buffer_index].tubes = PINC;
  // If buffer is full, write results to SD
  if(buffer_index++ == buffer_max){
    write_to_sd();
  }
}

unsigned long safeMicros() {
  extern volatile unsigned long timer0_overflow_count;
  return((timer0_overflow_count << 8) + TCNT0)*(64/16);
}

// =============================================================================
void setup(){
  Serial.begin(115200);
  Serial.println(F("LBCC RockSat-C code warming up..."));
  
  // Prepare shutdown pins as outputs, allowing them to pull down voltage
  pinMode(SHUTDOWN1, OUTPUT);
  pinMode(SHUTDOWN2, OUTPUT);
  pinMode(SHUTDOWN3, OUTPUT);
  pinMode(SHUTDOWN4, OUTPUT);
  
  // Power on pattern is 0110
  digitalWrite(SHUTDOWN1, HIGH);
  digitalWrite(SHUTDOWN2, LOW);
  digitalWrite(SHUTDOWN3, LOW);
  digitalWrite(SHUTDOWN4, HIGH);
  
  // Prepare SD card
  pinMode(10, OUTPUT);
  if(!sd.begin(10, SPI_FULL_SPEED)){
    // Card failed to initialize, attempt to format or fix.
    Serial.println(F("SD Card failed."));
  } else {
    // Card initialized, create a new file to write into.
    uint8_t BASE_NAME_SIZE = sizeof(FILE_BASE_NAME) - 1;    
    // Check if filename too long, if so, use a default.
    if(BASE_NAME_SIZE > 6){
      Serial.print(F("Error: FILE_BASE_NAME too long!  Using default."));
      filename[0] = 'LBCC_00.CSV';
      BASE_NAME_SIZE = sizeof("LBCC_") - 1;
    }
    
    // If the filename already exists, create a new file with larger number
    while(sd.exists(filename)){
      // If the last number is not a 9, increment it
      if(filename[BASE_NAME_SIZE + 1] != '9'){
        filename[BASE_NAME_SIZE + 1]++;
      // Otherwise, if the first number is not a 9, increment it
      } else if (filename[BASE_NAME_SIZE] != '9') {
        filename[BASE_NAME_SIZE]++;
        filename[BASE_NAME_SIZE + 1] = 0;
      // Lastly, if you're at 99, use a default
      } else {
        Serial.println(F("Error: Too many files! Using default."));
        filename[BASE_NAME_SIZE] = 'A0.CSV';
      }      
      Serial.print(F("Opening file: "));
      Serial.println(filename);
    }
  }
  
  // Prepare input pins and begin interrupts
  pinMode(2, INPUT);
  // Interrupt syntax is interrupt num, function name, event to listen for.
  // On the Uno, interrupt 0 is on digital pin 2. (don't ask)
  attachInterrupt(0, readTubes, RISING);
  
  if(DEBUGGING){
    // If debugging, use onboard led as status light.
    pinMode(13, OUTPUT);
    digitalWrite(13, HIGH);
  }
}

// =============================================================================
void loop(){
  // The interrupt will take over if an event occurs, so just wait until shutdown.
  if(millis() > time_max){
    write_to_sd();
    Serial.print(F("\nWriting shutdown pattern..."));
    digitalWrite(13, LOW);
    while(true){
      // Power off pattern is 0110
      digitalWrite(SHUTDOWN1, LOW);
      digitalWrite(SHUTDOWN2, HIGH);
      digitalWrite(SHUTDOWN3, HIGH);
      digitalWrite(SHUTDOWN4, LOW);
      // Arduino should now be powered off.
    }
  }
}
