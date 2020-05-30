#include <lorawan.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>

#define BME_ADDR 0x76 //Address BME280
#define SEALEVELPRESSURE_HPA (1013.25)
#define PIN 33 // Pin NeoPixel di ThinkBoard LoRa Node
#define NUMPIXELS 1 // Jumlah led RGB


//ABP Credentials 
const char *devAddr = "260415A0"; //Sesuaikan dengan Device Address di TTN
const char *nwkSKey = "9D582639715DD1093B5A18A3BBCB0F14"; //Sesuaikan dengan Network Session Key di TTN
const char *appSKey = "54276A8182DA0ADD0209724FE5A8BEF0"; //Sesuaikan dengan Apps Key di TTN

// Delay
const unsigned long interval = 30000;    // Sesuaikan dengan uplink fair usage di TTN
unsigned long previousMillis = 0;  

// Variabel untuk BME280
int suhu;     
int lembab;
int tekanan;

char myStr[255];
char outStr[255];
byte recvStatus = 0;

// Pin LoRa RFM95W di ThinkBoard LoRa Node
const sRFM_pins RFM_pins = {
  .CS = 5, //NSS
  .RST = 27, //RST
  .DIO0 = 2, //DIO0
  .DIO1 = 4 //DIO1
};


Adafruit_BME280 bme;
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  // Setup loraid access
  Serial.begin(115200);
  pixels.begin();
  pixels.show();
  pixels.setBrightness(50);
  
  Serial.println("Set BME");
  bool status;
  status = bme.begin(BME_ADDR);
  if (!status) {
    Serial.println("BME280 tidak ditemukan!");
    while (1);
  }
  Serial.println("BME OK!");
  colorWipe(pixels.Color(209,0,0), 500);
  delay(2000);
  
  Serial.println("Set LoRa!");
  if(!lora.init()){
    Serial.println("RFM95 tidak terdeteksi, periksa sambungan!");
    delay(5000);
    return;
  }
  colorWipe(pixels.Color(16,6,209), 500); 
  Serial.println("LoRa OK!");
  delay(2000);
  // Set LoRaWAN Class change CLASS_A or CLASS_C
  lora.setDeviceClass(CLASS_A);

  // Set Data Rate
  lora.setDataRate(SF7BW125);

  // set channel to random
  lora.setChannel(MULTI);
  
  // Put ABP Key and DevAddress here
  lora.setNwkSKey(nwkSKey);
  lora.setAppSKey(appSKey);
  lora.setDevAddr(devAddr);
}

void loop() {
  theaterChase(pixels.Color(127, 127, 127), 250); // 
  measure(); 
  if(millis() - previousMillis > interval) {
    previousMillis = millis();
    colorWipe(pixels.Color(48,   240,   0), 500);
    encodeJson(); 
    sendLora();
  }
}

void measure(){
  suhu = bme.readTemperature();
  tekanan = (bme.readPressure() / 100.0F);
  lembab = bme.readHumidity();
}

void encodeJson(){
  StaticJsonDocument<255> doc;
  doc["suhu"] = suhu;
  doc["kelembaban"] = lembab;
  doc["tekanan"] = tekanan;
  serializeJson(doc,myStr);
}

void sendLora(){
  Serial.print("Sending: ");
  Serial.println(myStr);   
  lora.sendUplink(myStr, strlen(myStr), 0);

  recvStatus = lora.readData(outStr);
  if(recvStatus) {
    Serial.println(outStr);
  }
  
  // Check Lora RX
  lora.update();  
}

void colorWipe(uint32_t color, int wait) {
  for(int i=0; i<pixels.numPixels(); i++) { 
    pixels.setPixelColor(i, color);         
    pixels.show();                         
    delay(wait);                           
  }
}

void theaterChase(uint32_t color, int wait) {
  for(int a=0; a<10; a++) {  // Repeat 10 times...
    for(int b=0; b<3; b++) { //  'b' counts from 0 to 2...
      pixels.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in steps of 3...
      for(int c=b; c<pixels.numPixels(); c += 3) {
        pixels.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      pixels.show(); // Update strip with new contents
      delay(wait);  // Pause for a moment
    }
  }
}
