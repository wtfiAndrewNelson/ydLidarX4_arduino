/*///////////////////////////////////////////////////////////////

 Using ESP32 for 2 serial ports (two different speeds)
 Place dots every ~2* of data
 Make continuous sweep

 NOTE:  YDLidar library needs to have baud rate set to 128000 
        in zip file before adding to libraries

///////////////////////////////////////////////////////////////*/
 
//YD Lidar pre-setup begin -----------------------------------
#include <YDLidar.h>

// Create an driver instance 
YDLidar lidar;

bool isScanning = false;   

#define YDLIDAR_MOTOR_SCTP 25                 // lidar motor speed PWM pin
#define YDLIDAR_MOTRO_EN   26                 // lidar motor enable pin                  
//YD Lidar pre-setup end -------------------------------------


//OLED Display pre-setup begin -------------------------------
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#define OLED_Address 0x3c                     // initialize with the I2C addr 0x3C
#define SCREEN_WIDTH 128                      // OLED display width, in pixels
#define SCREEN_HEIGHT 64                      // OLED display height, in pixels
#define OLED_RESET -1

Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
//OLED Display pre-setup end ---------------------------------


float screenX = 0;
float screenY = 0;
int screenZoom =125;
float screenAngleI = 0;

void setup()
{
  //OLED Display
  Serial.begin(9600);
  display.begin(OLED_Address, true);          // Address 0x3C default
  display.display();                          // display Adafruit logo resident in memory
  //display.setTextWrap(false);
  //display.setTextSize(1);
  //display.setTextColor(SH110X_WHITE);
  //delay(2000);                              // use millis() for all other timing 
  display.clearDisplay();
  //display.display();

  
  lidar.begin(Serial2, 128000);               // YDLIDAR on arduino hardware serial2
  pinMode(YDLIDAR_MOTOR_SCTP, OUTPUT);
  pinMode(YDLIDAR_MOTRO_EN, OUTPUT);
}

void loop()
{
  if(isScanning)
  {
    if (lidar.waitScanDot() == RESULT_OK)
    {
      if (screenAngleI >= 360) screenAngleI = 0;
      float distance = lidar.getCurrentScanPoint().distance;        //distance in mm
      float angle    = lidar.getCurrentScanPoint().angle;           //angle in degrees
      if (angle >= screenAngleI && angle < (screenAngleI + 0.1))
      {
        // write dots to screen
        Serial.print("angle: ");
        Serial.print(angle, DEC);
        Serial.print("\t distance: ");
        Serial.println(distance, DEC);
        //display.display();
        // plot on display
        if (distance < 150) distance = screenZoom * 32;
        screenX = distance / screenZoom * cos(angle / 360 * 6.28) + 64;
        screenY = distance / screenZoom * sin(angle / 360 * 6.28) + 32;
        display.drawPixel(screenX, screenY, 1);
        display.display();
        screenAngleI = screenAngleI + 2;
      }      
    }
    else
      {
        Serial.println(" YDLIDAR get Scandata failed!!!");
      }
    }
  else
    {
      //stop motor
      digitalWrite(YDLIDAR_MOTRO_EN, LOW);
      setMotorSpeed(0);
      restartScan();
    }
}


void setMotorSpeed(float vol)
{
  uint8_t PWM = (uint8_t)(51*vol);
  analogWrite(YDLIDAR_MOTOR_SCTP, PWM);
}


void restartScan()
{
  device_info deviceinfo;
  if (lidar.getDeviceInfo(deviceinfo, 100) == RESULT_OK)
  {
    int _samp_rate=4;
    String model;
    float freq = 7.0f;
    switch(deviceinfo.model)
    {
      case 1:
        model="F4";
         _samp_rate=4;
        freq = 7.0;
        break;
      case 4:
        model="S4";
        _samp_rate=4;
        freq = 7.0;
        break;
      case 5:
        model="G4";
        _samp_rate=9;
        freq = 7.0;
        break;
      case 6:
        model="X4";
        _samp_rate=5;
        freq = 7.0;
        break;
      default:
        model = "Unknown";
    }

    uint16_t maxv = (uint16_t)(deviceinfo.firmware_version>>8);
    uint16_t midv = (uint16_t)(deviceinfo.firmware_version&0xff)/10;
    uint16_t minv = (uint16_t)(deviceinfo.firmware_version&0xff)%10;
    if(midv==0)
    {
      midv = minv;
      minv = 0;
    }

    Serial.print("Firmware version:");
    Serial.print(maxv,DEC);
    Serial.print(".");
    Serial.print(midv,DEC);
    Serial.print(".");
    Serial.println(minv,DEC);

    Serial.print("Hardware version:");
    Serial.println((uint16_t)deviceinfo.hardware_version,DEC);

    Serial.print("Model:");
    Serial.println(model);

    Serial.print("Serial:");
    for (int i=0;i<16;i++)
    {
      Serial.print(deviceinfo.serialnum[i]&0xff, DEC);
    }
    Serial.println("");

    Serial.print("[YDLIDAR INFO] Current Sampling Rate:");
    Serial.print(_samp_rate,DEC);
    Serial.println("K");

    Serial.print("[YDLIDAR INFO] Current Scan Frequency:");
    Serial.print(freq,DEC);
    Serial.println("Hz");
    delay(100);
    device_health healthinfo;
    if (lidar.getHealth(healthinfo, 100) == RESULT_OK)
    {
      // detected...
      Serial.print("[YDLIDAR INFO] YDLIDAR running correctly! The health status:");
      Serial.println( healthinfo.status==0?"well":"bad");
      if(lidar.startScan() == RESULT_OK)
      {
        isScanning = true;
        //start motor in 1.8v
		    setMotorSpeed(1.8);
		    digitalWrite(YDLIDAR_MOTRO_EN, HIGH);
        Serial.println("YDLIDAR is scanning ......");
        //delay(1000);
      }
      else
      {
        Serial.println("start YDLIDAR failed!  Continue........");
      }
    }
    else
    {
      Serial.println("cannot retrieve YDLIDAR health");
    }
  }
  else
  {
    Serial.println("YDLIDAR get DeviceInfo Error!!!");
  }
}
