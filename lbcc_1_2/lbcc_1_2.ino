#include <SdFat.h>

#define buffer_max 255  // Max size of each buffer
#define time_max 3600000  // Max runtime (60 min)

#define tube_interrupt 32
#define tubeA 34
#define tubeB 35
#define tubeC 36
#define tubeD 37
#define tubeE 38
#define tubeF 39

#define SDPin 4  // CS pin for SD card socket, pin 4 on ethernet shield

typedef struct gdata {
  /*
  Each reading has a 4 byte timestamp, and 1 byte of data
  Variables used by interrupts should be called volatile
  */
  volatile unsigned long time;      // Timestamp, limited to 71 minutes of run time
  volatile byte tubes;              // 1 byte holds all 6, bit-sized measurements
};

gdata buffer[2][buffer_max+1];  // An array of 2 buffers, with space for 256 readings each
boolean active_buffer;        // Determines which buffer is ready for input
byte buffer_index = 0;        // Counter to hold current place in the buffer
SdFat sd;
SdFile dataFile;

void setup(){
  // Debugging only, using serial is very slow.
  Serial.begin(9600); // 115200 or 9600?
  Serial.println("LBCC RockSat-C warming up...");
  // delay(400);  // Solves Due reset problem
  
  // Prepare port C by setting input and output pins.  
   DDRC |= B0000001;  // Changes pins 2-7 on port C to inputs

  // Pin num, function name, event to listen for
  pinMode(tube_interrupt, INPUT);
  attachInterrupt(tube_interrupt, record_event, FALLING);
  
  // Set up the SD card
  pinMode(SDPin, OUTPUT);
  if(!sd.begin(SDPin, SPI_HALF_SPEED)){
    Serial.println("SD Card initialization failed!");
    while(true){}    // Should attempt to fix/format the SD card.
  } else {
    if(!sd.mkdir("/lbcc/rocksatc/")){
      Serial.println("create folder \"/lbcc/rocksatc/\" failed.");
    } else {
      Serial.println("SD Card ready.");
    }
  }
}

void loop(){
  // Debugging only, fills the active buffer with sample data
  while(true){
    process_interrupt(micros(), random(6));
  }
}

void write_to_sd(){
  /*
  Takes no parameters, toggles from active to inactive buffer, writes the (now)
  inactive buffer to SD card.
  */
  active_buffer = !active_buffer; // Switch between buffers
  
  if(dataFile.open("/lbcc/rocksatc/rscdata.txt", O_RDWR | O_CREAT | O_AT_END)){
    Serial.print("Writing buffer ");
    Serial.print(!active_buffer);
    Serial.print(" to the SD card...");
    dataFile.println("New flush beginning here:");
    for(int i=0; i<buffer_max; i++){
      dataFile.print(buffer[!active_buffer][i].time);
      dataFile.print("\t");
      dataFile.println(buffer[!active_buffer][i].tubes);
    }
    Serial.println(" Write complete!");
    dataFile.close();
  } else {
    Serial.println("Error opening file...");
  }
  /*
  All of this code is for debugging only, and will be replaced by a
  write to the SD card.
  */
  /*
  Serial.print("\nWriting buffer ");
  // !active_buffer refers to the inactive buffer
  Serial.print(!active_buffer); 
  Serial.print(" to the SD card.\n");
  Serial.println("First 10 lines of buffer:");
  for(int i=0; i<buffer_max; i++){
    Serial.print(buffer[!active_buffer][i].time);
    Serial.print("\t");
    Serial.println(buffer[!active_buffer][i].tubes);
  }
  */
}

void record_event(){
  /*
  Creates a new entry in the active buffer using the current time and tube status.
  The status of all tubes is recorded simultaneously via port manipulation.
  When buffer is full, swaps to secondary buffer and calls write_to_sd().
  Remember, interrupt functions cannot take parameters.
  */
  buffer[active_buffer][buffer_index].time = micros();
  buffer[active_buffer][buffer_index].tubes = PINC; // Read all input pins on port C.
  if(buffer_index++ == buffer_max){
    write_to_sd();
  }
}
