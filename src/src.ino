/*
Arduino code for Linn-Benton Community College's Rocksat-C 2015 experiment.
Author: Levi Willmeth
Source available online at https://github.com/lo9key/LBCC-RockSatC-2015/
*/
#include <SdFat.h>
#include <SdFatUtil.h>

#define buffer_max 50   // Max size of each buffer
#define DEBUGGING true  // Debugging mode on/off

// Geiger tube input pins
#define SHUTDOWN1 3
#define SHUTDOWN2 4
#define SHUTDOWN3 5
#define SHUTDOWN4 6

#define FILE_BASE_NAME "LBCC_"

// 6 Geiger tube input pins on Analog A0-A5
// These are read using the PINC block, instead of individual pins.

typedef struct gdata {
  // Each reading has a 4 byte timestamp, and 1 byte of data
  unsigned long time;// Timestamp, limited to 71! minutes
  byte tubes;        // 1 byte holds all 6, bit-sized measurements
};

// 2 buffers, with space for many readings each
gdata buffer[2][buffer_max+1];
// Variables changed inside interrupts should be declared volatile
volatile boolean active_buffer = 0;// Which buffer is ready for input
volatile byte buffer_index = 0;    // Index of active buffer
volatile byte buffer_full_index;   // Index of inactive buffer
volatile boolean buffer_full = false;// Flag for holding state of buffer

// 3600000000 is 60 min
// 3600000
// 1,000,000 microseconds per second
// 1,000 microseconds per second
unsigned long time_max = 3600000;   // Gives us a max run time, in millis

SdFat sd;
SdFile myFile;
char filename[13] = FILE_BASE_NAME "00.TXT";

// =============================================================================
void write_to_sd(){
  /*
  Takes no parameters, writes new data in the active buffer to the SD card.
  */
  active_buffer = !active_buffer;      // Switch buffers
  buffer_full_index = buffer_index-1;  // Store index of active buffer
  buffer_index = 0;                    // Reset index of active buffer
    
  if (!myFile.open(filename, O_RDWR | O_CREAT | O_AT_END)) {
    Serial.println(F("Output file failed to open."));
  } else {
    // Data from an empty, or partially filled buffer should not be written.
    for(int i=0; i<buffer_full_index; i++){
        myFile.print(buffer[!active_buffer][i].time);
        myFile.print(F("\t"));
        myFile.println( (buffer[!active_buffer][i].tubes), BIN);
        if(DEBUGGING){
          Serial.print(buffer[!active_buffer][i].time);
          Serial.print(F("\t"));
          Serial.println( (buffer[!active_buffer][i].tubes), BIN);
        }
    }
    myFile.close();
    // Buffer has been flushed, so reset the buffer_full index and flags.
    buffer_full = false;
    buffer_full_index = 0;
  }
}

// =============================================================================
void readTubes(){
  /*
  Takes no parameters, stores time and state of each tube into the buffer.
  */
  // micros() won't increment inside the ISR, but it will return previous value
  buffer[active_buffer][buffer_index].time = micros();
  // PINC returns the state of all pins on port C, which includes A0 to A5
  buffer[active_buffer][buffer_index].tubes = PINC;
  // If buffer is full, write results to SD
  if(buffer_index++ == buffer_max-1){
    // Write the SD card outside of this ISR, so we don't ignore new data.
    buffer_full = true;
  }
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
      filename[0] = 'LBCC_00.TXT';
      BASE_NAME_SIZE = sizeof("LBCC_") - 1;
    }
    
    // If the filename already exists, create a new file with larger number
    while(sd.exists(filename)){
      Serial.print(filename);
      Serial.println(" exists..");
      // If the last number is not a 9, increment it
      if(filename[BASE_NAME_SIZE + 1] != '9'){
        filename[BASE_NAME_SIZE + 1]++;
      // Otherwise, if the first number is not a 9, increment both
      } else if (filename[BASE_NAME_SIZE] != '9') {
        filename[BASE_NAME_SIZE]++;
        filename[BASE_NAME_SIZE + 1] = '0';
      // Lastly, if you're at 99, use a default
      } else {
        Serial.println(F("Error: Too many files! Using default."));
        filename[BASE_NAME_SIZE] = 'A0.TXT';
      }
    }
    if(DEBUGGING){
      Serial.print(F("Creating file: "));
      Serial.println(filename);
    }
  }
  
  // Prepare input pins and begin interrupts
  pinMode(2, INPUT);
  // Interrupt syntax is interrupt num, function name, event to listen for.
  // On the Uno, interrupt 0 is on digital pin 2. (don't ask)
  // On the due, this would be pin # instead of interrupt name
  attachInterrupt(0, readTubes, RISING);
  
  // Use onboard led as status light.
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
}

// =============================================================================
void loop(){
  // The interrupt will take over if an event occurs, so just wait until shutdown.
  // Arbitrary delay size without blocking interrupts
//  if(DEBUGGING) Serial.println(millis());
  for(unsigned int i=1; i!=0; i++){
    // Constantly check buffer flag while waiting for interrupts.
    if(buffer_full) write_to_sd();
  }
  if(millis() > time_max){
    // Assuming flight has finished, flush buffer and shut down.
    write_to_sd();                      // Write any buffer data to sd card.
    Serial.print(F("\nWriting shutdown pattern..."));
    digitalWrite(13, LOW);              // Turn off onboard LED
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
