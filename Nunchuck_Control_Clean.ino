#include <Wire.h>
#include <Adafruit_NeoPixel.h>

#define NUMPIXELS 27
#define LED_PIN D6
Adafruit_NeoPixel strip(NUMPIXELS, LED_PIN, NEO_GRB +NEO_KHZ800);

static uint8_t nunchuck_buf[6];   // array to store nunchuck data
int lightPos = 1;
int lightHue = 15;
int lightSize = 1;
int lightBrightness = 15;
int lightSat = 0;
boolean ONOFF = 0;

void setup()
{
  Serial.begin(115200);

  strip.begin();
  strip.show();
  strip.setBrightness(50);

  strip.fill(strip.Color(255, 0, 255));
  strip.show();
  
  nunchuck_init(); // send the initilization handshake
  Serial.print ("Finished setup\n");
}

void loop()
{
  nunchuck_get_data(); 
  //nunchuck_print_data();  
  nunchuck_parse_data();
  nunchuck_print_parsed_data();
  updateLights();    
  delay(100);    
}

void updateLights(){
  strip.clear();
  int indexStart = lightPos - (lightSize / 2);
  int stripLength = lightSize;
  if(indexStart < 0){
    indexStart = 0;
  }
  if((indexStart + stripLength) > NUMPIXELS){
    stripLength = 0;
  }
  strip.fill(strip.ColorHSV((lightHue * 2621), lightSat, (lightBrightness * 10)), indexStart, stripLength);
  if(!ONOFF){
    strip.clear();
  }
  strip.show();
}

void nunchuck_init()
{ 
  Wire.begin();                      // join i2c bus as master
  Wire.beginTransmission(0x52);     // transmit to device 0x52
  Wire.write(0x40);            // sends memory address
  Wire.write(0x00);            // sends sent a zero.  
  Wire.endTransmission();     // stop transmitting
}

void nunchuck_send_request()
{
  Wire.beginTransmission(0x52);     // transmit to device 0x52
  Wire.write(0x00);            // sends one byte
  Wire.endTransmission();     // stop transmitting
}

int nunchuck_get_data()
{
    int cnt=0;
    Wire.requestFrom (0x52, 6);
    while (Wire.available ()) {
      // receive byte as an integer
      nunchuck_buf[cnt] = nunchuk_decode_byte(Wire.read());
      cnt++;
    }
    nunchuck_send_request();  // send request for next data payload
    // print once 6 bytes are recieved
    if (cnt >= 5) {
     return 1;   // success
    }
    return 0; //failure
}

void nunchuck_parse_data(){

  if((nunchuck_buf[5] >> 1) & 1){
    if(nunchuck_buf[0] < 100 && lightPos > 1){
      lightPos--;
    }else if(nunchuck_buf[0] > 150 && lightPos < NUMPIXELS){
      lightPos++;
    }
    if(nunchuck_buf[1] < 100 && lightSize > 1){
      lightSize--;
    }else if(nunchuck_buf[1] > 150 && lightSize < NUMPIXELS){
      lightSize++;
    }

    if((nunchuck_buf[5] >> 0) & 1){
    //z not pressed
    }else{
      if(ONOFF){
        ONOFF = false;
      }else if(!ONOFF){
        ONOFF = true;
      }
      delay(150);
      Serial.print("-------------------------------------------------");

    }
  
  }else{
    if(nunchuck_buf[0] < 100 && lightHue > 1){
      lightSat = 255;
      lightHue--;
    }else if(nunchuck_buf[0] > 150 && lightHue < 25){
      lightSat = 255;
      lightHue++;
    }
    if(nunchuck_buf[1] < 100 && lightBrightness > 1){
      lightBrightness--;
    }else if(nunchuck_buf[1] > 150 && lightBrightness < 25){
      lightBrightness++;
    }

    if((nunchuck_buf[5] >> 0) & 1){
      //z not pressed
    }else{
      lightSat = 0;
      lightSize = NUMPIXELS;
      lightPos = NUMPIXELS / 2;
      lightBrightness = 250;     
      
      delay(150);
    }
  }  
}

void nunchuck_print_parsed_data(){
  Serial.print("Light Position: ");
  Serial.print(lightPos);
  Serial.print("  Light Size: ");
  Serial.print(lightSize);

  Serial.print("  Hue: ");
  Serial.print(lightHue);
  Serial.print("  Brightness: ");
  Serial.print(lightBrightness);
  Serial.print("\r\n");  // newline
}

void nunchuck_print_data()
{ 
  int joy_x_axis = nunchuck_buf[0];
  int joy_y_axis = nunchuck_buf[1];

  int accel_x_axis = nunchuck_buf[2]; // * 2 * 2; 
  int accel_y_axis = nunchuck_buf[3]; // * 2 * 2;
  int accel_z_axis = nunchuck_buf[4]; // * 2 * 2;


  int z_button = 0;
  int c_button = 0;

  // byte nunchuck_buf[5] contains bits for z and c buttons
  // it also contains the least significant bits for the accelerometer data
  // so we have to check each bit of byte outbuf[5]
  if ((nunchuck_buf[5] >> 0) & 1) 
    z_button = 1;
  if ((nunchuck_buf[5] >> 1) & 1)
    c_button = 1;

  if ((nunchuck_buf[5] >> 2) & 1) 
    accel_x_axis += 2;
  if ((nunchuck_buf[5] >> 3) & 1)
    accel_x_axis += 1;

  if ((nunchuck_buf[5] >> 4) & 1)
    accel_y_axis += 2;
  if ((nunchuck_buf[5] >> 5) & 1)
    accel_y_axis += 1;

  if ((nunchuck_buf[5] >> 6) & 1)
    accel_z_axis += 2;
  if ((nunchuck_buf[5] >> 7) & 1)
    accel_z_axis += 1;
  
  Serial.print("joy:");
  Serial.print(joy_x_axis,DEC);
  Serial.print(",");
  Serial.print(joy_y_axis, DEC);
  Serial.print("  \t");

  Serial.print("acc:");
  Serial.print(accel_x_axis, DEC);
  Serial.print(",");
  Serial.print(accel_y_axis, DEC);
  Serial.print(",");
  Serial.print(accel_z_axis, DEC);
  Serial.print("\t");

  Serial.print("but:");
  Serial.print(z_button, DEC);
  Serial.print(",");
  Serial.print(c_button, DEC);

  Serial.print("\r\n");  // newline
}

// Encode data to format that most wiimote drivers except
// only needed if you use one of the regular wiimote drivers
char nunchuk_decode_byte (char x)
{
  x = (x ^ 0x17) + 0x17;
  return x;
}
