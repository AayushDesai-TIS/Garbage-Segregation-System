#include <WiFi.h>
#include <WiFiUDP.h>

const char* ssid = "TIS";
const char* pass = "Wecanwewill";


WiFiUDP udp;
String old = "a";
String c = "a";
String m;
int flag2 = 0;
int start = 0;
int stop = 0;


// Define the pins for the TCS3200 sensor
#define S0 13
#define S1 26
#define S2 14
#define S3 19
#define OUT_PIN 5

#define con_dir 25
#define con_pwm 27
#define dir 33
#define pwm 32
#define pot 34

int flag = 0;
String color; 

// Min and max raw values from the sensor (these need to be calibrated)
int minRaw = 0;  // Adjust based on your setup
int maxRaw = 250; // Adjust based on your setup

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  udp.begin(8080);

   // Set the frequency scaling to 20%
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  digitalWrite(S0, HIGH);
  digitalWrite(S1, LOW);
  pinMode(dir,OUTPUT);
  pinMode(pwm,OUTPUT);
  pinMode(con_dir,OUTPUT);
  pinMode(con_pwm,OUTPUT);
  pinMode(pot,INPUT);

  // Set the other control pins
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  
  // Set the output pin as input
  pinMode(OUT_PIN, INPUT);
}

void loop() {

  // ---------------------------------------------------------------------------------------
  // data collection from Ripi
  if(c == "a")
  {
    analogWrite(con_pwm,0);
    analogWrite(32,0);

  }
  int packetSize = udp.parsePacket();
  if (packetSize) {
    char message[packetSize + 1];
    udp.read(message, packetSize);
    message[packetSize] = '\0';
    //Serial.print("Received message: ");
    m = String(message);

    
    

    if(old != message)
    {
      
      

      if(m == "W")
      {
        c = "g";
      }
      if(m == "D")
      {
        c = "b";
      }
      if(m == "E")
      {
        c = "r";
      }
      if(m == "N")
      {
        c = "N";

      }
      
      old = message;
      Serial.println(c);
      

    }



    size_t messageLength = strlen(message);
    uint8_t messageArray[messageLength];
    for (int i = 0; i < messageLength; i++) {
      messageArray[i] = (uint8_t)message[i];
    }

    udp.beginPacket(udp.remoteIP(), udp.remotePort());
    udp.write(messageArray, messageLength);
    udp.endPacket();
  }

  // --------------------------------------------------------------------------------------------


  int x = analogRead(pot);
  int y = map(x,0,4095,0,255);
  
 
  int rawRed = readColor(LOW, LOW);    // Read red color
  int rawGreen = readColor(HIGH, HIGH); // Read green color
  int rawBlue = readColor(LOW, HIGH);   // Read blue color

  // Map the raw values to the RGB range (0-255)
  int red = map(rawRed, minRaw, maxRaw, 255, 0);
  int green = map(rawGreen, minRaw, maxRaw, 255, 0);
  int blue = map(rawBlue, minRaw, maxRaw, 255, 0);

  // Ensure the values are within the 0-255 range
  red = constrain(red, 0, 255);
  green = constrain(green, 0, 255);
  blue = constrain(blue, 0, 255);

  // Detect and print the color
  String color = detectColor(red, green, blue);
  //Serial.print("Detected Color: ");
  //Serial.println(color);

  if(flag == 0)
  {

    if(c == "N")
    {
      analogWrite(32,0);
     digitalWrite(33,HIGH);
     

    }
    else
    {
      analogWrite(32,100);
      digitalWrite(33,HIGH);
      

    }


    if(flag2 == 0)
    {
      analogWrite(con_pwm,0);
    }

  
    
 
  

  }
  else  
  {
  //analogWrite(con_pwm,y);
  analogWrite(32,0);
  digitalWrite(33,HIGH);
  


  }

   

   if(color == c)
  {
    // dustbin is detected
    flag = 1;
    flag2 = 1;

  }
  else  
  {
    flag = 0; 
  }

  if(flag2 == 1)
  {
    start = millis();
    if(start - stop>=5000)
    {
      
      
      stop = start;
      flag2 = 0;
    }
    else  
    {
      analogWrite(con_pwm,150);
    }
  }



  
}



int readColor(bool s2, bool s3) {
  digitalWrite(S2, s2);
  digitalWrite(S3, s3);
  return pulseIn(OUT_PIN, LOW);
}

String detectColor(int red, int green, int blue) {

  if (red>blue && red>green && red>210) {
    return "r";
  } 
  else if (green>200 && green >red && green>blue) {
    return "g";
  } 
  else if (blue>220 && blue>green && blue>red) {
    return "b";
  } 
  else {
    return "Unknown";
  }

  

}
