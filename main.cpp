#include <SoftwareSerial.h>

// Define the pins for SoftwareSerial
const int RX_PIN = 10; // Connect to TX of SIM808
const int TX_PIN = 11; // Connect to RX of SIM808

SoftwareSerial sim808(RX_PIN, TX_PIN); // RX, TX

bool sendATCommand(const char* command, const char* expectedResponse, String& response, unsigned long timeout = 1000);
bool initializeSIM808();
void parseGPSData(const String& response);

void setup() {
  Serial.begin(9600);
  sim808.begin(9600);

  Serial.println("Starting communication test...");

  // Wait for the module to power up
  delay(5000);

  // Initialize the module
  if (initializeSIM808()) {
    Serial.println("SIM808 initialized successfully.");
  } else {
    Serial.println("SIM808 init error.");
  }

  // Attempt to power on GPS
  String response;
  if (sendATCommand("AT+CGNSPWR=1", "OK", response)) {
    Serial.println("Open the GPS power success");
  } else {
    Serial.println("Open the GPS power failure");
  }
}

void loop() {
  String response;
  // Request GPS data
  if (sendATCommand("AT+CGNSINF", "OK", response)) {
    parseGPSData(response);
  }

  // Add a delay to avoid spamming requests
  delay(5000); // Request GPS data every 5 seconds
}

bool initializeSIM808() {
  String response;
  // Check if the SIM808 module initializes properly
  return sendATCommand("AT", "OK", response) &&
         sendATCommand("AT+CGNSPWR=1", "OK", response) && // Power on GPS
         sendATCommand("AT+CGNSSEQ=RMC", "OK", response); // Set NMEA sentence type
}

bool sendATCommand(const char* command, const char* expectedResponse, String& response, unsigned long timeout) {
  sim808.println(command);
  unsigned long startTime = millis();

  response = "";
  while (millis() - startTime < timeout) {
    while (sim808.available()) {
      char c = sim808.read();
      response += c;
    }

    if (response.indexOf(expectedResponse) != -1) {
      // Print the response for debugging
      Serial.println("Response: " + response);
      return true;
    }
  }

  // Print the response for debugging
  Serial.println("Response: " + response);
  return false;
}

void parseGPSData(const String& response) {
  Serial.println("Raw GPS Data: " + response);

  // Extract data from the +CGNSINF response
  int startIndex = response.indexOf("+CGNSINF: ");
  if (startIndex != -1) {
    startIndex += 10; // Move past "+CGNSINF: "
    int endIndex = response.indexOf('\n', startIndex);
    if (endIndex == -1) endIndex = response.length();

    String gpsData = response.substring(startIndex, endIndex);
    Serial.println("Parsed GPS Data: " + gpsData);

    // Split the data by commas
    int commaIndex = gpsData.indexOf(',');
    String fixStatus = gpsData.substring(0, commaIndex);
    gpsData = gpsData.substring(commaIndex + 1);

    commaIndex = gpsData.indexOf(',');
    String fixType = gpsData.substring(0, commaIndex);
    gpsData = gpsData.substring(commaIndex + 1);

    commaIndex = gpsData.indexOf(',');
    String dateTime = gpsData.substring(0, commaIndex);
    gpsData = gpsData.substring(commaIndex + 1);

    commaIndex = gpsData.indexOf(',');
    String latitude = gpsData.substring(0, commaIndex);
    gpsData = gpsData.substring(commaIndex + 1);

    commaIndex = gpsData.indexOf(',');
    String longitude = gpsData.substring(0, commaIndex);
    gpsData = gpsData.substring(commaIndex + 1);

    commaIndex = gpsData.indexOf(',');
    String speed = gpsData.substring(0, commaIndex);
    gpsData = gpsData.substring(commaIndex + 1);

    commaIndex = gpsData.indexOf(',');
    String heading = gpsData.substring(0, commaIndex);
    gpsData = gpsData.substring(commaIndex + 1);

    // Print parsed GPS data
    Serial.println("GPS Fix Status: " + fixStatus);
    Serial.println("Fix Type: " + fixType);
    Serial.println("Date/Time: " + dateTime);
    Serial.println("Latitude: " + latitude);
    Serial.println("Longitude: " + longitude);
    Serial.println("Speed (km/h): " + speed);
    Serial.println("Heading: " + heading);
  } else {
    Serial.println("Empty GPS data");
  }
}
