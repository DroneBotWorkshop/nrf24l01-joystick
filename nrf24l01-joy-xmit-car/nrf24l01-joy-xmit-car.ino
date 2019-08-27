/*
  nRF24L01+ Joystick Transmitter
  nrf24l01-joy-xmit-car.ino
  nRF24L01+ Transmitter with Joystick for Robot Car
  Use with Joystick Receiver for Robot Car
  DroneBot Workshop 2018
  https://dronebotworkshop.com
*/

// Include RadioHead ReliableDatagram & NRF24 Libraries
#include <RHReliableDatagram.h>
#include <RH_NRF24.h>

// Include dependant SPI Library
#include <SPI.h>

// Define Joystick Connections
#define joyVert    A0
#define joyHorz    A1

// Define Joystick Values - Start at 512 (middle position)
int joyposVert = 512;
int joyposHorz = 512;

// Define addresses for radio channels
#define CLIENT_ADDRESS 1
#define SERVER_ADDRESS 2

// Create an instance of the radio driver
RH_NRF24 RadioDriver;

// Sets the radio driver to NRF24 and the client address to 1
RHReliableDatagram RadioManager(RadioDriver, CLIENT_ADDRESS);

// Declare unsigned 8-bit motorcontrol array
// 2 Bytes for motor speeds plus 1 byte for direction control
uint8_t motorcontrol[3];

// Define the Message Buffer
uint8_t buf[RH_NRF24_MAX_MESSAGE_LEN];

void setup()
{
  // Setup Serial Monitor
  Serial.begin(9600);

  // Initialize RadioManager with defaults - 2.402 GHz (channel 2), 2Mbps, 0dBm
  if (!RadioManager.init())
    Serial.println("init failed");

  // Set initial motor direction as forward
  motorcontrol[2] = 0;

}

void loop()
{
  // Print to Serial Monitor
  Serial.println("Reading motorcontrol values ");

  // Read the Joystick X and Y positions
  joyposVert = analogRead(joyVert);
  joyposHorz = analogRead(joyHorz);

  // Determine if this is a forward or backward motion
  // Do this by reading the Verticle Value
  // Apply results to MotorSpeed and to Direction

  if (joyposVert < 460)
  {
    // This is Backward
    // Set Motors backward
    motorcontrol[2] = 1;

    //Determine Motor Speeds
    // As we are going backwards we need to reverse readings
    motorcontrol[0] = map(joyposVert, 460, 0, 0, 255);
    motorcontrol[1] = map(joyposVert, 460, 0, 0, 255);

  }
  else if (joyposVert > 564)
  {
    // This is Forward
    // Set Motors forward
    motorcontrol[2] = 0;

    //Determine Motor Speeds
    motorcontrol[0] = map(joyposVert, 564, 1023, 0, 255);
    motorcontrol[1] = map(joyposVert, 564, 1023, 0, 255);

  }
  else
  {
    // This is Stopped
    motorcontrol[0] = 0;
    motorcontrol[1] = 0;
    motorcontrol[2] = 0;

  }

  // Now do the steering
  // The Horizontal position will "weigh" the motor speed
  // Values for each motor

  if (joyposHorz < 460)
  {
    // Move Left
    // As we are going left we need to reverse readings
    // Map the number to a value of 255 maximum
    joyposHorz = map(joyposHorz, 460, 0, 0, 255);

    motorcontrol[0] = motorcontrol[0] - joyposHorz;
    motorcontrol[1] = motorcontrol[1] + joyposHorz;

    // Don't exceed range of 0-255 for motor speeds
    if (motorcontrol[0] < 0)motorcontrol[0] = 0;
    if (motorcontrol[1] > 255)motorcontrol[1] = 255;

  }
  else if (joyposHorz > 564)
  {
    // Move Right
    // Map the number to a value of 255 maximum
    joyposHorz = map(joyposHorz, 564, 1023, 0, 255);

    motorcontrol[0] = motorcontrol[0] + joyposHorz;
    motorcontrol[1] = motorcontrol[1] - joyposHorz;

    // Don't exceed range of 0-255 for motor speeds
    if (motorcontrol[0] > 255)motorcontrol[0] = 255;
    if (motorcontrol[1] < 0)motorcontrol[1] = 0;

  }

  // Adjust to prevent "buzzing" at very low speed
  if (motorcontrol[0] < 8)motorcontrol[0] = 0;
  if (motorcontrol[1] < 8)motorcontrol[1] = 0;

  //Display the Motor Control values in the serial monitor.
  Serial.print("Motor A: ");
  Serial.print(motorcontrol[0]);
  Serial.print(" - Motor B: ");
  Serial.print(motorcontrol[1]);
  Serial.print(" - Direction: ");
  Serial.println(motorcontrol[2]);

  //Send a message containing Motor Control data to manager_server
  if (RadioManager.sendtoWait(motorcontrol, sizeof(motorcontrol), SERVER_ADDRESS))
  {
    // Now wait for a reply from the server
    uint8_t len = sizeof(buf);
    uint8_t from;
    if (RadioManager.recvfromAckTimeout(buf, &len, 2000, &from))
    {
      Serial.print("got reply from : 0x");
      Serial.print(from, HEX);
      Serial.print(": ");
      Serial.println((char*)buf);
    }
    else
    {
      Serial.println("No reply, is nrf24_reliable_datagram_server running?");
    }
  }
  else
    Serial.println("sendtoWait failed");

  delay(100);  // Wait a bit before next transmission
}
