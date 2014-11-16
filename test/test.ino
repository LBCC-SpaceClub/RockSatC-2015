#define buffer_max 255  // Max size of each buffer
#define time_max 60000  // Max runtime

#define gateA 40
#define gateB 41
#define gateC 42
#define pinD 43
#define pinE 44
#define pinF 45

typedef struct gdata {
  /*
  Each reading has a 4 byte timestamp, and 1 byte of data
  Variables used by interrupts should be called volatile
  */
  volatile unsigned long time;      // Our timestamp, limited to 71! minutes of run time
  volatile byte tubes;              // 1 byte holds all 6, bit-sized measurements
};

gdata buffer[2][buffer_max+1];  // An array of 2 buffers, with space for 256 readings each
boolean active_buffer;        // determines which buffer is ready for input
byte buffer_index = 0;        // Counter to determine when the buffer is full

void setup(){
  // Debugging only, using serial is very slow.
  Serial.begin(115200);
  Serial.println("LBCC RockSat-C code warming up...");
  
  // Prepare input pins and begin interrupts
  // Pin num, function name, event to listen for
  // Coincidence gates A, B, C
  pinMode(gateA, INPUT);
  attachInterrupt(gateA, interrupt_gate_A, FALLING);
  pinMode(gateB, INPUT);
  attachInterrupt(gateB, interrupt_gate_B, FALLING);
  pinMode(gateC, INPUT);
  attachInterrupt(gateC, interrupt_gate_C, FALLING);
  // Individual tubes D, E, F
  pinMode(pinD, INPUT);
  attachInterrupt(pinD, interrupt_tube_D, FALLING);
  pinMode(pinE, INPUT);
  attachInterrupt(pinE, interrupt_tube_E, FALLING);
  pinMode(pinF, INPUT);
  attachInterrupt(pinF, interrupt_tube_F, FALLING);
}

void loop(){
  // Debugging only, fills the active buffer with sample data
  /*
  while(true){
    //process_interrupt(micros(), random(6));
  }
  */
}

void write_to_sd(){
  /*
  Takes no parameters, toggles from active to inactive buffer, writes the (now)
  inactive buffer to SD card.
  */
  active_buffer = !active_buffer; // Switch between buffers
  
  /*
  All of this code is for debugging only, and will be replaced by a
  write to the SD card.
  */
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
}

/*
  One function per interrupt, passes time and event description for storage
*/
void interrupt_gate_A(){  // events in tubes F and E
  process_interrupt(micros(), 35);
}
void interrupt_gate_B(){  // events in tubes F and D
  process_interrupt(micros(), 21);
}
void interrupt_gate_C(){  // events in tubes E and D
  process_interrupt(micros(), 14);
}
void interrupt_tube_D(){
  process_interrupt(micros(), 4);
}
void interrupt_tube_E(){
  process_interrupt(micros(), 2);
}
void interrupt_tube_F(){
  process_interrupt(micros(), 1);
}

void process_interrupt(unsigned long event_time, byte event_tube){
  /*
  Takes timestamp, and 1 byte describing which event was recorded, adds it to array
  and increments buffer_index.  If array is full, writes active buffer to SD card.
  Buffer_index overflows from 255 to 0, so it's ready for the secondary buffer.
  */
  // Should we worry about buffer flip while queued? I doubt it, but maybe.
  buffer[active_buffer][buffer_index].time = event_time;
  buffer[active_buffer][buffer_index].tubes = event_tube;
  if(buffer_index++ == buffer_max){
    write_to_sd();
  }
}
