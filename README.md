# ydLidarX4_arduino
getting yd lidar x4 up and running on arduino with an esp32

I am finding all kinds of problems with getting data from the X4 in a useable
way that does not involve a computer attached by usb. However, I have gotten it
to work and will continue to update this fun project as I have time.

I am using an esp32 developer module; 
              oled i2c 128 x 64 display; 
              3.3 - 5 volt bidirectional data converter; 
              5v power supply; 
              yd lidar x4

The ydlidar_arduino-master.zip file is the library that you will need to
install manually for this to work. It's simple. Open the lidar*.ino in your
arduino ide. Click on SKETCH, select Include Library> Add ZIP Library..., 
navigate to where you saved the ydlidar_arduino-master.zip file and done!

I have wired the Serial2 to the yd lidar X4 as it is 3.3v control but I also
have supplied an external 5v power supply to run the lidar. I have the i2c
going to the oled display through a 3.3 to 5v bidirectional adapter and it
gets its power from the 5v adapter. I am also powerering the esp32 from 5v
so that the entire thing will run from the adapter.
