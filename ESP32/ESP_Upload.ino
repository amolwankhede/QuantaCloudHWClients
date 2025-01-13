#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>

// Wi-Fi credentials
const char* ssid = "Zapisco";
const char* password = "Zapisco2020@757!"; 

// API details
const char* serverUrl = "https://api.quantaflare.com/v1/Data/stream-push/5fe35eec-e18d-48cb-9604-cb7d6eb3cbb5";
const char* apiKey = "CA1F89191C86485BA5C2B615024A54B1"; 

// NTP Server settings
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 0;

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Initialize time from NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

// Get current time in milliseconds
unsigned long long getTimeInMillis() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (unsigned long long)(tv.tv_sec) * 1000LL + (tv.tv_usec / 1000LL);
}

// Generate a random State of Charge (SOC) value between 0 and 100
float generateSOC() {
  return random(0, 10001) / 100.0;  // Generates a value between 0.00 and 100.00
}

// Generate a random voltage value between 22.00 and 25.00
float generateVoltage() {
  return random(2200, 2501) / 100.0;
}

// Generate a random current value between 5.00 and 15.00
float generateCurrent() {
  return random(500, 1501) / 100.0;
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("x-api-key", apiKey);

    // Create JSON structure
    StaticJsonDocument<256> doc;
    JsonArray root = doc.to<JsonArray>();

    root.add(getTimeInMillis());  // Timestamp in milliseconds
    root.add(1);                  // Static number

    JsonArray staticArray = root.createNestedArray();
    staticArray.add(1);
    staticArray.add(2);
    staticArray.add(12);

    JsonArray valuesArray = root.createNestedArray();
    valuesArray.add(String(generateSOC(), 2));
    valuesArray.add(String(generateVoltage(), 2));
    valuesArray.add(String(generateCurrent(), 2));

    String jsonString;
    serializeJson(doc, jsonString);

    Serial.println("\nSending data: " + jsonString);

    int httpResponseCode = http.POST(jsonString);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("HTTP Response code: " + String(httpResponseCode));
      Serial.println("Response body: " + response);
    } else {
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);
      Serial.println("Error: " + http.errorToString(httpResponseCode));
    }

    http.end();
  } else {
    Serial.println("WiFi not connected");
  }

  delay(7000);  // Wait for 7 seconds before sending the next payload
}
