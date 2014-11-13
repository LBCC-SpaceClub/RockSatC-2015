#define buffer_max 256  // Max size of each buffer
#define time_max 60000  // Max runtime

#define pinA 10

typedef struct gdata {
  /*
  Each reading has a 4 byte timestamp, and 1 byte of data
  Variables used by interrupts should be considered volatile
  */
  volatile unsigned long time;         // Our timestamp
  volatile byte tubes;              // 8 bits to hold our 6 bit-sized measurements.
};

gdata buffer[2][buffer_max];  // An array of 2 buffers
boolean active_buffer;        // determines which buffer is ready for input
byte buffer_index = 0;
//unsigned long cur_time;

void setup(){
  // Debugging
  Serial.begin(115200);
  
  pinMode(pinA, INPUT);
  attachInterrupt(pinA, gateA, LOW);
}

void loop(){
  // Debugging, fill the active buffer with sample data
  while(buffer_index<buffer_max){
    buffer[active_buffer][buffer_index].time = micros();
    buffer[active_buffer][buffer_index].tubes = random(6);
    buffer_index++;
  } // end while
  
  active_buffer = !active_buffer; // Switch between buffers
  write_to_sd(); // Write old buffer to SD card
}

void write_to_sd(){
  /*
  All of this code is for debugging only, and will be replaced by a
  write to the SD card.
  */
  Serial.print("\nWriting buffer ");
  // !active_buffer refers to the inactive buffer
  Serial.print(!active_buffer); 
  Serial.print(" to the SD card.\n");
  Serial.println("First 10 lines of buffer:");
  for(int i=0; i<10; i++){
    Serial.print(buffer[!active_buffer][i].time);
    Serial.print("\t");
    Serial.println(buffer[!active_buffer][i].tubes);
  }
}

void gateA(){ // event in tube A
  buffer[active_buffer][buffer_index].time = micros();
  buffer[active_buffer][buffer_index].tubes = 32;
}
  

