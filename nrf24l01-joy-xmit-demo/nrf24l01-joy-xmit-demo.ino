/*
  nRF24L01+ Joystick Transmitter
  nrf24l01-joy-xmit-demo.ino
  nRF24L01+ Transmitter with Joystick
  Use with Joystick Receiver Demo
  DroneBot Workshop 2018
  https://dronebotworkshop.com
*/

// Include RadioHead ReliableDatagram & NRF24 Libraries
#include <RHReliableDatagram.h>
#include <RH_NRF24.h>

// Include dependant SPI Library
#include <SPI.h>

// Define Joystick Connections
#define JoyStick_X_PIN     A0
#define JoyStick_Y_PIN     A1

// Define addresses for radio channels
#define CLIENT_ADDRESS 1
#define SERVER_ADDRESS 2

// Create an instance of the radio driver
RH_NRF24 RadioDriver;

// Sets the radio driver to NRF24 and the client address to 1
RHReliableDatagram RadioManager(RadioDriver, CLIENT_ADDRESS);

// Declare unsigned 8-bit joystick array
uint8_t joystick[3];

// Define the Message Buffer
uint8_t buf[RH_NRF24_MAX_MESSAGE_LEN];

void setup()
{
  // Setup Serial Monitor
  Serial.begin(9600);

  // Initialize RadioManager with defaults - 2.402 GHz (channel 2), 2Mbps, 0dBm
  if (!RadioManager.init())
    Serial.println("init failed");
}

void loop()
{
  // Print to Serial Monitor
  Serial.println("Reading joystick values ");

  // Read Joystick values and map to values of 0 - 255
  joystick[0] = map(analogRead(JoyStick_X_PIN), 0, 1023, 0, 255);
  joystick[1] = map(analogRead(JoyStick_Y_PIN), 0, 1023, 0, 255);
  joystick[2] = 100;

  //Display the joystick values in the serial monitor.
  Serial.println("-----------");
  Serial.print("x:");
  Serial.println(joystick[0]);
  Serial.print("y:");
  Serial.println(joystick[1]);

  Serial.println("Sending Joystick data to nrf24_reliable_datagram_server");

  //Send a message containing Joystick data to manager_server
  if (RadioManager.sendtoWait(joystick, sizeof(joystick), SERVER_ADDRESS))
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
