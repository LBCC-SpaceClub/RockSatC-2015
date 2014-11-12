#define buffer_max 512  // Max size of each buffer
#define time_max 60000  // Max runtime


struct gdata {
  unsigned long time;
  boolean A; // Coincidence gate 1
  boolean B; // Coincidence gate 2
  boolean C; // Coincidence gate 3
  boolean D; // Geiger tube 1
  boolean E; // Geiger tube 2
  boolean F; // Geiger tube 3
};

gdata buffer[2][buffer_max];
boolean active_buffer;

void setup(){
  Serial.begin(9600);
}

void loop(){
  // Debugging
  for(int i=0; i<buffer_max; i++){
    buffer[active_buffer][i].time = micros();
  }
  
  active_buffer = !active_buffer; // Switch between buffers
  write_to_sd(); // Write old buffer to SD card
}

void write_to_sd(){
  delay(2000);
  // always write inactive buffer to SD card by using !active_buffer
//  Serial.print("Writing to the SD card:");
  Serial.println("Writing to the SD card:"+!active_buffer);
}
  

