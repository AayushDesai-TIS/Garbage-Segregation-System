// CODE FOR DATA TRANMITTING BETWEEN RIPI AND esp32 : ESP32 CODE (Receiver)

#include <WiFi.h>
#include <WiFiUDP.h>

const char* ssid = "TIS";
const char* pass = "Wecanwewill";

WiFiUDP udp;

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  udp.begin(8080);
}

void loop() {
  int packetSize = udp.parsePacket();
  if (packetSize) {
    char message[packetSize + 1];
    udp.read(message, packetSize);
    message[packetSize] = '\0';
    Serial.print("Received message: ");
    Serial.println(message);

    size_t messageLength = strlen(message);
    uint8_t messageArray[messageLength];
    for (int i = 0; i < messageLength; i++) {
      messageArray[i] = (uint8_t)message[i];
    }

    udp.beginPacket(udp.remoteIP(), udp.remotePort());
    udp.write(messageArray, messageLength);
    udp.endPacket();
  }
}
