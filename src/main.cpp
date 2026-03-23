#include <Arduino.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define BTN_PIN 15
#define LED_PIN  14
float pitchOffset = 0;
float rollOffset  = 0;

bool displayMode = false;      
bool lastBtnState = HIGH;

Adafruit_MPU6050 mpu; //Object of the class Adafruit_MPU6050
LiquidCrystal_I2C lcd(0x27, 20, 4);  //Object of LiquidCrystal_I2C


void calibrate() 
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("  Calibrating...  ");
  lcd.setCursor(0, 1);
  lcd.print(" Keep sensor flat ");

  float pitchSum = 0, rollSum = 0;
  int samples = 200;

  for (int i = 0; i < samples; i++) 
  {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    float ax = a.acceleration.x;
    float ay = a.acceleration.y;
    float az = a.acceleration.z;

    pitchSum += atan2(ay, sqrt(ax*ax + az*az)) * 180.0 / PI;
    rollSum  += atan2(ax, sqrt(ay*ay + az*az)) * 180.0 / PI;
    delay(5);
  }

  pitchOffset = pitchSum / samples;
  rollOffset  = rollSum  / samples;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("  Calibrated!     ");
  delay(1500);
  lcd.clear();
}

String getCardinal(float heading) 
{
  if (heading >= 337.5 || heading < 22.5)  
    return "N ";
  if (heading < 67.5)                       
    return "NE";
  if (heading < 112.5)                     
     return "E ";
  if (heading < 157.5)                      
    return "SE";
  if (heading < 202.5)                      
    return "S ";
  if (heading < 247.5)                      
    return "SW";
  if (heading < 292.5)                      
    return "W ";
  return "NW";
}

String makeBar(float value, float minVal, float maxVal) 
{
  int filled = map((int)value, (int)minVal, (int)maxVal, 0, 10);
  filled = constrain(filled, 0, 10);
  String bar = "[";
  for (int i = 0; i < 10; i++) bar += (i < filled) ? "#" : "-";
  bar += "]";
  return bar;
}

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);  // SDA, SCL

  // --- Init MPU6050 ---
  if (!mpu.begin()) {
    Serial.println("MPU6050 not found! Check wiring.");
    while (1) delay(10);  // halt here until fixed
  }
  Serial.println("MPU6050 connected!");

  mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  // --- Init LCD ---
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("  Compass + Tilt  ");
  lcd.setCursor(0, 1);
  lcd.print("  Initializing... ");
  delay(2000);
  lcd.clear();
  pinMode(LED_PIN, OUTPUT);
  calibrate(); 
}

void loop() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // raw accelerometer values 
  float ax = a.acceleration.x;
  float ay = a.acceleration.y;
  float az = a.acceleration.z;


  float pitch = atan2(ay, sqrt(ax*ax + az*az)) * 180.0 / PI - pitchOffset;  //To calculate pitch angle(Tilt forward/backward)
  float roll  = atan2(ax, sqrt(ay*ay + az*az)) * 180.0 / PI - rollOffset;   //To calculate roll angle(Tilt left/right)

  float heading = atan2(-ay,ax) * 180.0/PI; // To calculate the compass angle
  if(heading < 0)
  {
    heading += 360.0 ; //To get angle in the range of 0-360deg
  }
  String cardinal = getCardinal(heading);

  bool tiltAlert = (abs(pitch) > 30.0 || abs(roll) > 30.0);
  digitalWrite(LED_PIN, tiltAlert ? HIGH : LOW);

  bool btnState = digitalRead(BTN_PIN);
  if (btnState == LOW && lastBtnState == HIGH) {
    displayMode = !displayMode;
    lcd.clear();
    delay(50);
  }
  lastBtnState = btnState; 

  Serial.print(">Pitch:"); Serial.println(pitch);
  Serial.print(">Roll:"); Serial.println(roll);
  Serial.print(">Heading:"); Serial.println(heading);

    if (!displayMode) {
        // Mode 1: numbers view
        // Line 0: Heading + cardinal
        lcd.setCursor(0, 0);
        lcd.print("HDG:");
        lcd.print((int)heading);
        lcd.print("  ");
        lcd.print(cardinal);
        lcd.print("            ");

        // Line 1: Pitch
        lcd.setCursor(0, 1);
        lcd.print("Pitch: ");
        if (pitch >= 0) lcd.print(" ");   // alignment space for positive
        lcd.print(pitch, 1);
        lcd.print(" deg     ");

        // Line 2: Roll
        lcd.setCursor(0, 2);
        lcd.print("Roll:  ");
        if (roll >= 0) lcd.print(" ");
        lcd.print(roll, 1);
        lcd.print(" deg     ");

        // Line 3: Status
        lcd.setCursor(0, 3);
        lcd.print(tiltAlert ? "!! TILT ALERT !!    " 
                            : "Status: OK          ");
    } else {
        // Mode 2: bar graph view
        lcd.setCursor(0, 0);
        lcd.print("Pitch:");
        lcd.print((int)pitch);
        lcd.print("   ");

        lcd.setCursor(0, 1);
        lcd.print(makeBar(pitch, -90, 90));

        lcd.setCursor(0, 2);
        lcd.print("Roll: ");
        lcd.print((int)roll);
        lcd.print("   ");

        lcd.setCursor(0, 3);
        lcd.print(makeBar(roll, -180, 180));
    }


  delay(150);
}