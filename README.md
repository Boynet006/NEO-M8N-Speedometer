# NEO-M8N-Speedometer
Arduino Speedometer using NEO-M8N that reaches 10Hz refresh rate.

The code will give you the option to select between K/m and mph.
The wiring diagram is for Arduino nano.
The CH341SER.exe is needed if you receive COM port error when uploading.
The HardwareSerial.h is a modified version of the original one.
The HardwareSerial-backup contains the original HardwareSerial, in case you need it.
The FileLocation.txt contains information about the HardwareSerial location on your computer.

Steps:
Locate where the HardwareSerial.h is for example: C:\Users\"Your username"\AppData\Local\Arduino15\packages\arduino\hardware\avr\1.8.6\cores\arduino
Make a backup of your original HardwareSerial.h.
Copy HardwareSerial.h to your file's location.
Now you can upload the sketch and you should have the butter increased to accomodate the 10Hz refresh rate from NEO-M8N.


Note: Without increasing the buffer size, the device will not work as expected.
