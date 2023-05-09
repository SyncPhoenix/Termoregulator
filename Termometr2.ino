#include <OneWire.h>
#include "TM1637.h"
#define CLK 4
#define DIO 3
#define PIN_RELAY1 8
#define PIN_RELAY2 9
#define PIN_RELAY3 10
#define PIN_RELAY4 11
TM1637 disp(CLK,DIO);



OneWire  ds(5);  // on pin 10 (a 4.7K resistor is necessary)
  boolean boiler = false;
  boolean ten1_on = false;
  boolean ten2_on = false;
  boolean ten3_on = false;
  boolean heat = false;
  int timer = 0;
  int onPin = 2;
  int offPin = 3;
  int test = 0;
const byte DEGREE_SYMBOL = 0b01100011; 


  
void setup(void) {
  Serial.begin(9600);
  disp.set(BRIGHT_TYPICAL);//BRIGHT_TYPICAL = 2,BRIGHT_DARKEST = 0,BRIGHTEST = 7;
  pinMode(PIN_RELAY1, OUTPUT); 
  digitalWrite(PIN_RELAY1, HIGH); 
  pinMode(PIN_RELAY2, OUTPUT); 
  digitalWrite(PIN_RELAY2, HIGH); 
  pinMode(PIN_RELAY3, OUTPUT); 
  digitalWrite(PIN_RELAY3, HIGH); 
  pinMode(PIN_RELAY4, OUTPUT); 
  digitalWrite(PIN_RELAY4, HIGH); 
}

void loop(void) {
  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
  float celsius, fahrenheit;
  int temp;
  
  int ten1 = 55;
  int ten2 = 50;
  int ten3 = 45;
 
  
  
  if ( !ds.search(addr)) {
  
    ds.reset_search();
    delay(250);
    return;
  }
  
  Serial.print("ROM =");
  for( i = 0; i < 8; i++) {
    Serial.write(' ');
    Serial.print(addr[i], HEX);
  }

  if (OneWire::crc8(addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return;
  }
  Serial.println();
 
  // the first ROM byte indicates which chip
  switch (addr[0]) {
    case 0x10:
      Serial.println("  Chip = DS18S20");  // or old DS1820
      type_s = 1;
      break;
    case 0x28:
      Serial.println("  Chip = DS18B20");
      type_s = 0;
      break;
    case 0x22:
      Serial.println("  Chip = DS1822");
      type_s = 0;
      break;
    default:
      Serial.println("Device is not a DS18x20 family device.");
      return;
  } 

  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);   // start conversion, with parasite power on at the end
  
  delay(1000);  
 
  
  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);         

  Serial.print("  Data = ");
  Serial.print(present, HEX);
  Serial.print(" ");
  for ( i = 0; i < 9; i++) {   // we need 9 bytes
    data[i] = ds.read();
    Serial.print(data[i], HEX);
    Serial.print(" ");
  }
  Serial.print(" CRC=");
  Serial.print(OneWire::crc8(data, 8), HEX);
  Serial.println();

  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }
  celsius = (float)raw / 16.0;
  fahrenheit = celsius * 1.8 + 32.0;
  temp = celsius/1;
  
  disp.display(0,temp/10);
  disp.display(1,temp%10);
  disp.display(2, DEGREE_SYMBOL);
  disp.display(3, {12}); 

  if (celsius > 40)
    {
      digitalWrite(PIN_RELAY1, LOW);
    }
  else
    {
      digitalWrite(PIN_RELAY1, HIGH);
    }
  
  if (celsius > 60)
    {
      boiler = true;
    }
  if (celsius < ten1 and boiler and !ten1_on)
    {
      digitalWrite(PIN_RELAY2, LOW);
      ten1_on = true;
    }
  if (celsius > ten1+5 and boiler) // and ten1_on )
    {
      digitalWrite(PIN_RELAY2, HIGH);
      ten1_on = false;
    }
   if (celsius < ten2 and boiler and ten1_on and !ten2_on)
    {
      digitalWrite(PIN_RELAY3, LOW);
      ten2_on = true;
    }
  if (celsius > ten2+5  and boiler )// and ten1_on and ten2_on)
    {
      digitalWrite(PIN_RELAY3, HIGH);
      ten2_on = false;
    }
   if (celsius < ten3 and boiler and ten1_on and ten2_on and !ten3_on)
    {
      digitalWrite(PIN_RELAY4, LOW);
      ten3_on = true;
    }

 if (celsius > ten3+5 and boiler)// and ten1_on and ten2_on and ten3_on )
    {
      digitalWrite(PIN_RELAY4, HIGH);
      ten3_on = false;
    }
 
  Serial.println("onPin");
  Serial.println(analogRead(onPin));
  Serial.println("offPin");
  Serial.println(analogRead(offPin));
 
  test++;
}
