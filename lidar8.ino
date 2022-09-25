/*///////////////////////////////////////////////////////////////

 Using ESP32 for 2 serial ports (two different speeds)
 Now working and receiving data
 Added oled display
 Cleaned up
 Place dots every ~2* of data

 Removed - Refresh and display every three seconds and made 
            the display cumulative

 NOTE:  YDLidar library needs to have baud rate set to 128000 
        in zip file before adding to libraries

///////////////////////////////////////////////////////////////*/
 
//YD Lidar pre-setup begin -----------------------------------
#include <YDLidar.h>

// Create an driver instance 
YDLidar lidar;

bool isScanning = false;   

#define YDLIDAR_MOTOR_SCTP 25 // lidar motor speed PWM pin
#define YDLIDAR_MOTRO_EN   26 // lidar motor enable pin                  
//YD Lidar pre-setup end -------------------------------------


//OLED Display pre-setup begin -------------------------------
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#define OLED_Address 0x3c          //initialize with the I2C addr 0x3C
#define SCREEN_WIDTH 128          // OLED display width, in pixels
#define SCREEN_HEIGHT 64          // OLED display height, in pixels
#define OLED_RESET -1             //

Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
//OLED Display pre-setup end ---------------------------------


float screenX = 0;
float screenY = 0;
int screenZoom =125;

/*
int lidarDistance = 0;
int lidarDirection = 0;
float testN = 0;
bool dotColor = true;
*/

/*
// Three Second timer interrupt
hw_timer_t * My_timer = NULL;
volatile bool My_timerFlag = 0; 

void IRAM_ATTR onTimer()
{
  My_timerFlag = 1;
}
*/

float screenAngleI = 0;

void setup() {
/*
  My_timer = timerBegin(0, 80, true);
  timerAttachInterrupt(My_timer, &onTimer, true);
  timerAlarmWrite(My_timer, 3000000, true);
  timerAlarmEnable(My_timer);
*/
  //OLED Display
  Serial.begin(9600);
  display.begin(OLED_Address, true);       // Address 0x3C default
  display.display();                       // display Adafruit logo resident in memory
  //display.setTextWrap(false);
  //display.setTextSize(1);
  //display.setTextColor(SH110X_WHITE);
  //delay(2000);                              // use millis() for all other timing 
  display.clearDisplay();
  //display.display();


  // YDLIDAR on arduino hardware serial2
  lidar.begin(Serial2, 128000);
  //output mode
  pinMode(YDLIDAR_MOTOR_SCTP, OUTPUT);
  pinMode(YDLIDAR_MOTRO_EN, OUTPUT);

  //while(Serial.read() >= 0){};    // why is this here?
}

void loop()
{
  if(isScanning)
  {
    if (lidar.waitScanDot() == RESULT_OK)
    {
      float distance = lidar.getCurrentScanPoint().distance;  //distance in mm
      float angle    = lidar.getCurrentScanPoint().angle;     //angle in degrees
      byte  quality  = lidar.getCurrentScanPoint().quality;   //quality of the current measurement
	    bool  startBit = lidar.getCurrentScanPoint().startBit;
      if (distance > 120)
      {
        for (screenAngleI = 0; screenAngleI < 360; screenAngleI = screenAngleI + 2)   // adjust increment with screenZoom
        {
          float distance = lidar.getCurrentScanPoint().distance;  //distance in mm
          float angle    = lidar.getCurrentScanPoint().angle;     //angle in degrees
          byte  quality  = lidar.getCurrentScanPoint().quality;   //quality of the current measurement
	        bool  startBit = lidar.getCurrentScanPoint().startBit;
          if (angle >= screenAngleI && angle < (screenAngleI + 0.1))
          {
            // write dots to screen
            Serial.print("angle: ");
            Serial.print(angle, DEC);
            Serial.print("\t distance: ");
            Serial.println(distance, DEC);
            //display.display();
            // plot on display
            screenX = distance / screenZoom * cos(angle / 360 * 6.28) + 64;
            screenY = distance / screenZoom * sin(angle / 360 * 6.28) + 32;
            display.drawPixel(screenX, screenY, 1);
            display.display();
          }
        }

      }
      //display.display();
      //display.drawPixel(screenX, screenY, 0);

      /*
      if (My_timerFlag == 1)
      {
        My_timerFlag = 0;
        display.display();
        display.writeFillRect(0, 0, 128, 64, 0);
      }
      */
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
