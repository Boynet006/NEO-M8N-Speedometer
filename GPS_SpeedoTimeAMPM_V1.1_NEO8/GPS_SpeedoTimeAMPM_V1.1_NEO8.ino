// Developed by 3DElectroTech
// https://www.youtube.com/@3DElectroTech
// Inspired by iforce2d YouTube Channel

/*
 Connections:
 GPS TX -> Arduino 0 (disconnect to upload this sketch)
 GPS RX -> Arduino 1
 Screen SDA -> Arduino A4
 Screen SCL -> Arduino A5
*/

#define myTime +4 // <<< adjust you time accourding to UTC time

// For BaudRate Adjustment
#define GPS_BAUD1 9600
#define GPS_BAUD2 115200

// Change State button k/m mph
#define button 3
int state = 0;
int old = 0;
int buttonPoll = 0;

#include <Adafruit_SH1106.h> 
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 

//On an arduino UNO: A4(SDA), A5(SCL)
#define OLED_RESET -1 //Reset pin # (or -1 if sharing Arduino reset pin) 
#define SCREEN_ADDRESS 0x3C //See datasheet for Address   
Adafruit_SH1106 display(OLED_RESET); 

#define GPS Serial

const unsigned char UBLOX_INIT[] PROGMEM = {
  // Disable NMEA for NEO M8N, M9N
  0xB5,0x62,0x06,0x01,0x08,0x00,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x23, // GxGGA off
  0xB5,0x62,0x06,0x01,0x08,0x00,0xF0,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x2A, // GxGLL off
  0xB5,0x62,0x06,0x01,0x08,0x00,0xF0,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x31, // GxGSA off
  0xB5,0x62,0x06,0x01,0x08,0x00,0xF0,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x38, // GxGSV off
  0xB5,0x62,0x06,0x01,0x08,0x00,0xF0,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x3F, // GxRMC off
  0xB5,0x62,0x06,0x01,0x08,0x00,0xF0,0x05,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x46, // GxVTG off

//   // Disable UBX
  0xB5,0x62,0x06,0x01,0x08,0x00,0x01,0x07,0x00,0x00,0x00,0x00,0x00,0x00,0x17,0xDC, //NAV-PVT off
//   0xB5,0x62,0x06,0x01,0x08,0x00,0x01,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x12,0xB9, //NAV-POSLLH off
//   0xB5,0x62,0x06,0x01,0x08,0x00,0x01,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x13,0xC0, //NAV-STATUS off
  // 0xB5,0x62,0x06,0x01,0x08,0x00,0x01,0x12,0x00,0x00,0x00,0x00,0x00,0x00,0x22,0x29, //NAV-VELNED off

//   // Enable UBX
  0xB5,0x62,0x06,0x01,0x08,0x00,0x01,0x07,0x00,0x01,0x00,0x00,0x00,0x00,0x18,0xE1, //NAV-PVT on

//   // Rate
  // 0xB5,0x62,0x06,0x08,0x06,0x00,0x28,0x00,0x01,0x00,0x01,0x00,0x3E,0xAA, //(25Hz)
  // 0xB5,0x62,0x06,0x08,0x06,0x00,0x32,0x00,0x01,0x00,0x01,0x00,0x48,0xE6, //(20Hz)
  // 0xB5,0x62,0x06,0x08,0x06,0x00,0x42,0x00,0x01,0x00,0x01,0x00,0x58,0x46, //(15.15Hz)
  0xB5,0x62,0x06,0x08,0x06,0x00,0x64,0x00,0x01,0x00,0x01,0x00,0x7A,0x12, //(10Hz)
  // 0xB5,0x62,0x06,0x08,0x06,0x00,0x78,0x00,0x01,0x00,0x01,0x00,0x8E,0x8A, //(8.33Hz)
  // 0xB5,0x62,0x06,0x08,0x06,0x00,0xC8,0x00,0x01,0x00,0x01,0x00,0xDE,0x6A, //(5Hz)
  // 0xB5,0x62,0x06,0x08,0x06,0x00,0xE8,0x03,0x01,0x00,0x01,0x00,0x01,0x39, //(1Hz)
};

// Add the UBX command for setting the baud rate to 115200
const unsigned char SET_BAUD_RATE_115200[] PROGMEM = {
  // Baud Rate 115200
  0xB5,0x62,0x06,0x00,0x14,0x00,0x01,0x00,0x00,0x00,0xD0,0x08,0x00,0x00, // Baud Rate 115200
  0x00,0xC2,0x01,0x00,0x23,0x00,0x03,0x00,0x00,0x00,0x00,0x00,0xDC,0x5E  // Baud Rate 115200
};

const unsigned char UBX_HEADER[] = { 0xB5, 0x62 };

struct NAV_PVT {
  unsigned char cls;
  unsigned char id;
  unsigned short len;
  unsigned long iTOW;          // GPS time of week of the navigation epoch (ms)

  unsigned short year;         // Year (UTC) 
  unsigned char month;         // Month, range 1..12 (UTC)
  unsigned char day;           // Day of month, range 1..31 (UTC)
  unsigned char hour;          // Hour of day, range 0..23 (UTC)
  unsigned char minute;        // Minute of hour, range 0..59 (UTC)
  unsigned char second;        // Seconds of minute, range 0..60 (UTC)
  char valid;                  // Validity Flags
  unsigned long tAcc;          // Time accuracy estimate (ns)
  long nano;                   // Fraction of second (ns)
  unsigned char fixType;       // GNSSfix Type, range 0..5
  char flags;                  // Fix Status Flags
  unsigned char reserved1;     // reserved
  unsigned char numSV;         // Number of satellites used in Nav Solution

  long lon;                    // Longitude (deg)
  long lat;                    // Latitude (deg)
  long height;                 // Height above Ellipsoid (mm)
  long hMSL;                   // Height above mean sea level (mm)
  unsigned long hAcc;          // Horizontal Accuracy Estimate (mm)
  unsigned long vAcc;          // Vertical Accuracy Estimate (mm)

  long velN;                   // NED north velocity (mm/s)
  long velE;                   // NED east velocity (mm/s)
  long velD;                   // NED down velocity (mm/s)
  long gSpeed;                 // Ground Speed (2-D) (mm/s)
  long heading;                // Heading of motion 2-D (deg)
  unsigned long sAcc;          // Speed Accuracy Estimate
  unsigned long headingAcc;    // Heading Accuracy Estimate
  unsigned short pDOP;         // Position dilution of precision
  short reserved2;             // Reserved
  unsigned long reserved3;     // Reserved

  unsigned long buffer1;     // To Pass Checksum check
  unsigned long buffer2;     // To Pass Checksum check
};

NAV_PVT pvt;

void calcChecksum(unsigned char* CK) {
  memset(CK, 0, 2);
  for (int i = 0; i < (int)sizeof(NAV_PVT); i++) {
    CK[0] += ((unsigned char*)(&pvt))[i];
    CK[1] += CK[0];
  }
}

long numGPSMessagesReceived = 0;

bool processGPS() {
  static int fpos = 0;
  static unsigned char checksum[2];
  const int payloadSize = sizeof(NAV_PVT);

  while ( GPS.available() ) {
    byte c = GPS.read();
    if ( fpos < 2 ) {
      if ( c == UBX_HEADER[fpos] )
        fpos++;
      else
        fpos = 0;
    }
    else {      
      if ( (fpos-2) < payloadSize )
        ((unsigned char*)(&pvt))[fpos-2] = c;

      fpos++;

      if ( fpos == (payloadSize+2) ) {
        calcChecksum(checksum);
      }
      else if ( fpos == (payloadSize+3) ) {
        if ( c != checksum[0] )
          fpos = 0;
      }
      else if ( fpos == (payloadSize+4) ) {
        fpos = 0;
        if ( c == checksum[1] ) {
          return true;
        }
      }
      else if ( fpos > (payloadSize+4) ) {
        fpos = 0;
      }
    }
  }
  return false;
}

void setup() 
{

  GPS.begin(GPS_BAUD1);

  pinMode(button, INPUT);
  digitalWrite(LED_BUILTIN, LOW);

  // Initialize OLED display
  display.begin(SH1106_SWITCHCAPVCC, SCREEN_ADDRESS); 
  display.clearDisplay();
  display.display();
  delay(2000);

  // send configuration data in UBX protocol
  for(unsigned int i = 0; i < sizeof(UBLOX_INIT); i++) {                        
    GPS.write( pgm_read_byte(UBLOX_INIT+i) );
    delay(5); 
  }

    // Optional: Change baud rate to 115200
  for(unsigned int i = 0; i < sizeof(SET_BAUD_RATE_115200); i++) {                        
    GPS.write(pgm_read_byte(SET_BAUD_RATE_115200 + i));
    delay(5); 
  }

  // // Reinitialize Serial2 at 115200 baud
  GPS.end();              // End the 9600 baud communication
  GPS.begin(GPS_BAUD2);  // Start communication at 115200 baud

}

bool isPM = false;

unsigned char hour = 0; 
unsigned char minute = 0;
unsigned char second = 0;

long gSpeed = 0;
int numSV = 0;
unsigned long lastScreenUpdate = 0;
// char speedBuf[16];
// char satsBuf[16];
float speedCalc;
String speedoText = "N/A";

char* spinner = "/-\\|";
byte screenRefreshSpinnerPos = 0;
byte gpsUpdateSpinnerPos = 0;

void loop() {
  buttonPoll = digitalRead(button);
  if(buttonPoll == 1){
    delay(200);
    buttonPoll = digitalRead(button);
    if(buttonPoll == 0){
      state = old + 1;
    }
  }

  if ( processGPS() ) {
    hour = (pvt.hour myTime) % 24;  // Add 4 hours and ensure it wraps around after 23
    minute = pvt.minute;
    second = pvt.second;

    // Convert to 12-hour format
    if (hour >= 12) {
        isPM = true;
    } else {
        isPM = false;
    }

    if (hour > 12) {
        hour -= 12;
    } else if (hour == 0) {
        hour = 12;  // Midnight case (12 AM)
    }

    numSV = pvt.numSV;
    gSpeed = pvt.gSpeed;    
    gpsUpdateSpinnerPos = (gpsUpdateSpinnerPos + 1) % 4;
  }

  unsigned long now = millis();
  if ( now - lastScreenUpdate > 100 ) {
    updateScreen();
    lastScreenUpdate = now;
    screenRefreshSpinnerPos = (screenRefreshSpinnerPos + 1) % 4;
  }
}

void updateScreen(){

  switch (state){
    case 1:
      digitalWrite(LED_BUILTIN, HIGH);
      old = state;
      speedoText = "mph";
      speedCalc = (gSpeed / 445.114); // mph 
      break;

    default:
      digitalWrite(LED_BUILTIN, LOW);
      old = 0;
      speedoText = "Km/h";
      speedCalc = (gSpeed / 276.581); // kmh
      break;
  }

  display.clearDisplay();
  display.setTextColor(WHITE);

  display.setTextSize(2);
  display.setCursor(0, 0);
  display.print(spinner[screenRefreshSpinnerPos]);
  
  display.setTextSize(2);
  display.setCursor(15, 0);
  display.print(spinner[gpsUpdateSpinnerPos]);

  display.setTextSize(2);
  display.setCursor(30, 0);
  display.print(numSV);

if (pvt.valid & 0x04) {  // Checking if UTC time is valid (based on UBX protocol)
    display.setTextSize(1);
    display.setCursor(70, 15);
    display.print(hour);   // Print Hour
    display.print(":");
    if (minute < 10) display.print("0");  // Add leading zero if needed
    display.print(minute); // Print Minute
    display.print(":");
    if (second < 10) display.print("0");  // Add leading zero if needed
    display.print(second); // Print Second

    // Display AM/PM
    display.setCursor(115, 15);
    if (isPM) {
        display.print("PM");
    } else {
        display.print("AM");
    }
} else {
    // Display "No Fix" if UTC time is not valid
    display.setTextSize(1);
    display.setCursor(81, 15);
    display.print("NO DATA");
}


  display.setTextSize(4);
  display.setCursor(5, 28);
  display.print(speedCalc, 1); // Km/h or mph, calculation

  display.setTextSize(1);
  display.setCursor(100, 2);
  display.print(speedoText); // Km/h or mph

  display.display(); 
}
