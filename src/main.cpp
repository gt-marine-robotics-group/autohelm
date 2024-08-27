#include <Arduino.h>
//
//    FILE: MCP_POT_demo.ino
//  AUTHOR: Rob Tillaart
// PURPOSE: demo
//     URL: https://github.com/RobTillaart/MCP_POT


#include <MCP_POT.h>

//test file for MCP_POT below 


uint32_t start, stop;


//  select, reset, shutdown, data, clock == SOFTWARE SPI
MCP_POT pot(37, 9, 16, 11, 27);

//  select, reset, shutdown, &SPI === HW SPI UNO clock = 13, data = 11
// MCP_POT pot(5, 6, 7, &SPI);



void test_extremes()
{
  Serial.println(__FUNCTION__);
  delay(10);

  Serial.println("0");
  pot.setValue(0, 0);
  delay(2000);

  Serial.println(MCP_POT_MIDDLE_VALUE);
  pot.setValue(0, MCP_POT_MIDDLE_VALUE);
  delay(2000);

  Serial.println(MCP_POT_MAX_VALUE);
  pot.setValue(0, MCP_POT_MAX_VALUE);
  delay(2000);
}


//  connect all A GND and B 5V
//  every W will have a different signal (same freq).
void test_sinus()
{
  Serial.println(__FUNCTION__);
  delay(10);

  start = millis();
  uint32_t i = 0;
  while (millis() - start < 10000)
  {
    int8_t value = 127 * sin(i * TWO_PI / 100);
    pot.setValue(0, 128 + value);
    pot.setValue(1, 128 + value / 2);
    i++;
  }
}

//  straightforward sawtooth.
void test_sawtooth()
{
  Serial.println(__FUNCTION__);
  delay(10);

  start = millis();
  uint8_t i = 0;
  while (millis() - start < 25500)
  {
    pot.setValue(0, i++);  //  auto wrap is fast...
    delay(100);
  }
}


void test_timing()
{
  Serial.println(__FUNCTION__);
  delay(10);

  start = micros();
  for (int i = 0; i < 1000; i++)
  {
    pot.setValue(0, i++);  //  auto wrap is fast...
  }
  stop = micros();
  Serial.print("1000 x setValue():\t");
  Serial.println(stop - start);
  delay(10);

  volatile int x = 0;
  start = micros();
  for (int i = 0; i < 500; i++)
  {
    x += pot.getValue(0);
    x += pot.getValue(1);
  }
  stop = micros();
  Serial.print("1000 x getValue():\t");
  Serial.println(stop - start);
  delay(10);
}

void setup()
{
  Serial.begin(115200);
  Serial.println(__FILE__);

  SPI.begin();

  pot.begin();

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  // test_extremes();
  //  test_sinus();
  //  test_sawtooth();
  //  test_timing();

  Serial.println("\nSetup Done...");
}

//  -- END OF FILE --




void loop()
{
  digitalWrite(LED_BUILTIN, HIGH);
  test_extremes();
  Serial.println("\nTEST");
  digitalWrite(LED_BUILTIN, LOW);
  delay(2000);

}