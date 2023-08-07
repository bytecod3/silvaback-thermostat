#define uS_TO_S_FACTOR 1000000
#define TIME_TO_SLEEP 5

RTC_DATA_ATTR int bootCount = 0;

void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wake_up_source;

  wake_up_source = esp_sleep_get_wakeup_cause();

  switch(wake_up_source){
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wake-up from external signal with RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wake-up from external signal with RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wake up caused by a timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wake up caused by a touchpad"); break;
    default : Serial.printf("Wake up not caused by Deep Sleep: %d\n",wake_up_source); break;
  }
  
}

void setup() {

  Serial.begin(115200);
  ++bootCount;
  Serial.println("------------------------");
  Serial.println(String(bootCount)+ "th Boot");

  print_wakeup_reason();

  esp_sleep_enable_ext0_wakeup(GPIO_NUM_25, HIGH);

  // go to zzz!
  Serial.println("Goes into Deep Sleep mode");
  Serial.println("----------------------");
  delay(100);

  esp_deep_sleep_start();
}

void loop() {

}
