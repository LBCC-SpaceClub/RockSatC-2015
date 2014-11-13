#define buffer_max 512  // Max size of each buffer
#define time_max 60000  // Max runtime


typedef struct gdata {
  unsigned long time;         // Our timestamp
  byte tubes;              // 8 bits to hold our 6 bit-sized measurements.
};

gdata buffer[2][buffer_max];  // An array of 2 buffers
boolean active_buffer;        // determines which buffer is ready for input

void setup(){
  // Debugging
  Serial.begin(115200);
}

void loop(){
  // Debugging, load the buffers with sample data
  for(int i=0; i<buffer_max; i++){
    buffer[active_buffer][i].time = micros();
    
    // First, clear any data in the buffer
    buffer[active_buffer][i].tubes = false;
    // Set a random tube to True
    // In the final project, a read on tube 1,2 or 3 would mean a coincidence event.
    bitSet(buffer[active_buffer][i].tubes, random(6));
//    buffer[active_buffer][i].counters[random(6)+1] = true; // set a random tube (1-6 inc) to True
  }
  
  active_buffer = !active_buffer; // Switch between buffers
  write_to_sd(); // Write old buffer to SD card
}

void write_to_sd(){
  delay(500);
  // always write inactive buffer to SD card by using !active_buffer
  Serial.print("\nWriting buffer ");
  Serial.print(!active_buffer);
  Serial.print(" to the SD card.\n");
  Serial.println("First 10 lines of buffer:");
  for(int i=0; i<10; i++){
    Serial.print(buffer[!active_buffer][i].time);
    Serial.print("\t");
    Serial.println(buffer[!active_buffer][i].tubes);
  }
}
  

