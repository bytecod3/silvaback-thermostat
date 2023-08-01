#define pina 2
#define pinb 3

volatile int counter = 0;

void setup(){
  pinMode(pina, INPUT_PULLUP);
  pinMode(pinb, INPUT_PULLUP);
  
  // attach interrupts
  attachInterrupt(digitalPinToInterrupt(pina), read_encoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(pinb), read_encoder, CHANGE);

  // start the serial monitor
  Serial.begin(9600);
  delay(500); // wait for serial to start
  Serial.println("[+]Start...");
}

void loop(){
  static int last_counter = 0;

  // if counter has changed, print the new value to serial
  if(counter != last_counter){
    Serial.println(counter);
    last_counter = counter;
  }
}

void read_encoder(){
  static uint8_t old_ab = 3;
  static int8_t encoder_value = 0; // encoder value
  static const int8_t enc_states[] = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};
  
  old_ab <<= 2; // remember previous state
  
  if(digitalRead(pina)) old_ab |= 0x02; // add current state of pin A
  
  if(digitalRead(pinb)) old_ab |= 0x01; // add current state of pin B
 
  encoder_value += enc_states[(old_ab & 0x0f)];

  if(encoder_value > 2){ // four steps forward
    Serial.println("CW");
    counter++; //increase counter
    encoder_value = 0;  
      
  } else if(encoder_value < -2){ // four steps backwards
    Serial.println("CCW");
    counter--;
    encoder_value = 0;
  }

  delay(20);
}
