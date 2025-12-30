/* The source Code from : https://github.com/riyadhasan24
 * By Md. Riyad Hasan
 */
 
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

#define DHT_Pin 2
#define DHT_TYPE DHT11

#define IR_IN_Pin 3
#define IR_OUT_Pin 4
#define Fan_Pin 5
#define Light_Pin 6
#define Motion_Pin 7

#define Yellow_LED_Pin 8
#define Red_LED_Pin 9
#define Green_LED_Pin 10

#define Motion_LED_Pin 13

LiquidCrystal_I2C LCD(0x25, 16, 2);
DHT dht(DHT_Pin, DHT_TYPE);

int People_Count = 0;
int Temp_Threshold = 25;

int Last_In_State  = HIGH;
int Last_Out_State = HIGH;

unsigned long Last_In_Trigger_Time  = 0;
unsigned long Last_Out_Trigger_Time = 0;
unsigned long IR_Debounce_Interval  = 500;

unsigned long Last_Dht_Read_Time = 0;
unsigned long DHT_Interval       = 2000;

unsigned long Last_LCD_Update = 0;
unsigned long LCD_Interval    = 500;

float Current_Temp = 0.0;
float Current_Hum  = 0.0;

bool Pattern_Active               = false;
unsigned long Pattern_Last_Change = 0;
unsigned long Pattern_Interval    = 250;

int Pattern_Stage   = 0;
int Pattern_Repeats = 0;

int Last_Motion_State = LOW;

void setup()
{
  pinMode(IR_IN_Pin, INPUT_PULLUP);
  pinMode(IR_OUT_Pin, INPUT_PULLUP);
  pinMode(Motion_Pin, INPUT);
  pinMode(Fan_Pin, OUTPUT);
  pinMode(Light_Pin, OUTPUT);
  pinMode(Motion_LED_Pin, OUTPUT);
  pinMode(Yellow_LED_Pin, OUTPUT);
  pinMode(Red_LED_Pin, OUTPUT);
  pinMode(Green_LED_Pin, OUTPUT);

  digitalWrite(Fan_Pin, LOW);
  digitalWrite(Light_Pin, LOW);
  digitalWrite(Motion_LED_Pin, LOW);
  digitalWrite(Yellow_LED_Pin, LOW);
  digitalWrite(Red_LED_Pin, LOW);
  digitalWrite(Green_LED_Pin, LOW);

  LCD.init();
  LCD.backlight();
  dht.begin();

  LCD.clear();
  LCD.setCursor(0, 0);
  LCD.print(" Welcome to");
  LCD.setCursor(0, 1);
  LCD.print("Smart Classroom");
  delay(7000);
  LCD.clear();
}

void loop()
{
  unsigned long now = millis();

  if (now - Last_Dht_Read_Time >= DHT_Interval)
  {
    Last_Dht_Read_Time = now;
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    if (!isnan(h) && !isnan(t))
    {
      Current_Temp = t;
      Current_Hum  = h;
    }
  }

  unsigned long t = millis();
  int In_State  = digitalRead(IR_IN_Pin);
  int Out_State = digitalRead(IR_OUT_Pin);

  if (Last_In_State == HIGH && In_State == LOW)
  {
    if (t - Last_In_Trigger_Time > IR_Debounce_Interval)
    {
      People_Count++;
      if (People_Count < 0) People_Count = 0;
      Last_In_Trigger_Time = t;
    }
  }

  if (Last_Out_State == HIGH && Out_State == LOW)
  {
    if (t - Last_Out_Trigger_Time > IR_Debounce_Interval)
    {
      People_Count--;
      if (People_Count < 0) People_Count = 0;
      Last_Out_Trigger_Time = t;
    }
  }

  Last_In_State  = In_State;
  Last_Out_State = Out_State;

  if (Current_Temp >= Temp_Threshold) digitalWrite(Fan_Pin, HIGH);
  else digitalWrite(Fan_Pin, LOW);

  if (People_Count > 0) digitalWrite(Light_Pin, HIGH);
  else digitalWrite(Light_Pin, LOW);

  int Motion_State = digitalRead(Motion_Pin);
  digitalWrite(Motion_LED_Pin, Motion_State);

  if (Last_Motion_State == LOW && Motion_State == HIGH)
  {
    Pattern_Active      = true;
    Pattern_Repeats     = 0;
    Pattern_Stage       = 0;
    Pattern_Last_Change = now;
    digitalWrite(Yellow_LED_Pin, LOW);
    digitalWrite(Red_LED_Pin, LOW);
    digitalWrite(Green_LED_Pin, LOW);
  }

  Last_Motion_State = Motion_State;

  if (Pattern_Active && now - Pattern_Last_Change >= Pattern_Interval)
  {
    Pattern_Last_Change = now;

    digitalWrite(Yellow_LED_Pin, LOW);
    digitalWrite(Red_LED_Pin, LOW);
    digitalWrite(Green_LED_Pin, LOW);

    if (Pattern_Stage == 0) digitalWrite(Yellow_LED_Pin, HIGH);
    else if (Pattern_Stage == 1) digitalWrite(Red_LED_Pin, HIGH);
    else if (Pattern_Stage == 2) digitalWrite(Green_LED_Pin, HIGH);
    else if (Pattern_Stage == 3)
    {
      Pattern_Repeats++;
      if (Pattern_Repeats >= 5)
      {
        Pattern_Active = false;
        digitalWrite(Yellow_LED_Pin, LOW);
        digitalWrite(Red_LED_Pin, LOW);
        digitalWrite(Green_LED_Pin, LOW);
        Pattern_Stage = 0;
      }
    }

    if (Pattern_Active)
    {
      Pattern_Stage++;
      if (Pattern_Stage > 3) Pattern_Stage = 0;
    }
  }

  if (now - Last_LCD_Update >= LCD_Interval)
  {
    Last_LCD_Update = now;

    LCD.setCursor(0, 0);
    char Line1[17];
    snprintf(Line1, sizeof(Line1), "T:%2dC H:%2d%% ", (int)Current_Temp, (int)Current_Hum);
    LCD.print(Line1);

    LCD.setCursor(0, 1);
    char Line2[17];
    snprintf(Line2, sizeof(Line2), "People: %3d    ", People_Count);
    LCD.print(Line2);
  }
}
