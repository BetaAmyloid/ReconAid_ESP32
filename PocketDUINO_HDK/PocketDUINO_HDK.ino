#include <SoftwareSerial.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>

#define LED_01 27
#define GMS_RX 12
#define GMS_TX 14

TaskHandle_t Task1;
TaskHandle_t Task2;

SoftwareSerial GMS_Serial(GMS_TX, GMS_RX);

String phoneNumber;
String phoneNumberCompare;
String Name;
String rescueType;
String rescueTypeCompare;
double longitude;
double latitude;
String command = "";
String postData = "";
String payload = "";
String senderNumber = "";
String message = "";

HTTPClient http;  //--> Declare object of class HTTPClient.
int httpCode;     //--> Variables for HTTP return code.

// const char* ssid = "GlobeAtHome_D63DE_2.4";
// const char* password = "5cEMVxRk";
const char* ssid = "PLDTHOMEFIBR80180";
const char* password = "Skai_Chubby_07";

WiFiServer wifiServer(80);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  GMS_Serial.begin(9600);

  Serial.println("Initializing...");
  delay(1000);

  GMS_Serial.println("AT"); // Handshaking with SIM800L
  updateSerial();

  GMS_Serial.println("AT+CMGF=1"); // Configuring TEXT mode
  updateSerial();

  GMS_Serial.println("AT+CNMI=1,2,0,0,0"); // Set SIM800L to send SMS data to serial port when received
  updateSerial();

  pinMode(LED_01, OUTPUT);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");
  Serial.println(WiFi.localIP());
  wifiServer.begin();

  //create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
                    Task1code,   /* Task function. */
                    "Task1",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task1,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */                  
  delay(500); 

  //create a task that will be executed in the Task2code() function, with priority 1 and executed on core 1
  xTaskCreatePinnedToCore(
                    Task2code,   /* Task function. */
                    "Task2",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task2,      /* Task handle to keep track of created task */
                    1);          /* pin task to core 1 */
    delay(500); 

}

//Task1code: blinks an LED every 1000 ms
void Task1code( void * pvParameters ){
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());

  for(;;){
    // put your main code here, to run repeatedly:
    WiFiClient client = wifiServer.available();
    if (client) {
      Serial.println("New Client Connected");
      while (client.connected()) {
        if (client.available()) {
          //do things here
          receivePhoneData(client);
            if (command == "verifyAndRequest") {
              receiveFromDatabase(client);
              sendToDatabase();
              client.stop();
            } else if (command == "updateDatabase") {
              updateDatabase();
              client.stop();
            }
        }
        delay(10);
      }
      client.stop();
      Serial.println("Client Disconnected");
    }
    delay(10);
  } 
}

//Task2code: blinks an LED every 700 ms
void Task2code( void * pvParameters ){
  Serial.print("Task2 running on core ");
  Serial.println(xPortGetCoreID());

  for(;;){
    updateSerial();
  }
}

void loop() {

}

double readDouble(WiFiClient& client) {
    uint8_t buffer[8]; // Allocate 8 bytes for double
    client.read(buffer, 8); // Read 8 bytes

    double value;
    memcpy(&value, buffer, sizeof(value));
    return value;
}

String urlEncode(const String& str) {
  String encodedString = "";
  char c;
  char code0;
  char code1;
  char code2;
  for (int i = 0; i < str.length(); i++) {
    c = str.charAt(i);
    if (c == ' ') {
      encodedString += '+';
    } else if (isalnum(c)) {
      encodedString += c;
    } else {
      code1 = (c & 0xf) + '0';
      if ((c & 0xf) > 9) {
        code1 = (c & 0xf) - 10 + 'A';
      }
      code0 = ((c >> 4) & 0xf) + '0';
      if (((c >> 4) & 0xf) > 9) {
        code0 = ((c >> 4) & 0xf) - 10 + 'A';
      }
      code2 = '\0';
      encodedString += '%';
      encodedString += code0;
      encodedString += code1;
    }
  }
  return encodedString;
}

void receivePhoneData(WiFiClient& client) {
  command = client.readStringUntil('\n');
  Serial.println("Command: " + command);

  phoneNumber = client.readStringUntil('\n');
  Serial.println("Received: " + phoneNumber);

  Name = client.readStringUntil('\n');
  Serial.println("Received: " + Name);
  
  rescueType = client.readStringUntil('\n');
  Serial.println("Received: " + rescueType);

  longitude = readDouble(client);
  Serial.print("Received: ");
  Serial.println(longitude, 8);

  latitude = readDouble(client);
  Serial.print("Received: ");
  Serial.println(latitude, 8);
}

void sendToDatabase() {
    Serial.println("-------------Sending Data");
    Serial.print("Phone Number to send is: ");
    Serial.println(phoneNumber);
    Serial.print("Name to send is: ");
    Serial.println(Name);
    Serial.print("Rescue Type to send is: ");
    Serial.println(rescueType);
    Serial.print("longitude to send is: ");
    Serial.println(longitude, 8);
    Serial.print("latitude to send is: ");
    Serial.println(latitude, 8);
    

    postData = "phoneNumber=" + urlEncode(phoneNumber);
    postData += "&Name=" + urlEncode(Name);
    postData += "&rescueType=" + urlEncode(rescueType);
    postData += "&Longitude=" + String(longitude, 8);
    postData += "&Latitude=" + String(latitude, 8);

    payload + "";

    http.begin("http://192.168.254.102/ReconAidDatabase/insertdb.php");  //--> Specify request destination
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");  //--> Specify content-type header

    httpCode = http.POST(postData);
    payload = http.getString();

    Serial.print("httpCode : ");
    Serial.println(httpCode); //--> Print HTTP return code
    Serial.print("payload  : ");
    Serial.println(payload);  //--> Print request response payload
    
    http.end();
    Serial.println("-------------End Sending Data");
}

void receiveFromDatabase(WiFiClient& client) {
  Serial.println("---------------START receive from database");

  Serial.print("Sending: ");
  Serial.println(phoneNumber);
  Serial.print("Sending: ");
  Serial.println(rescueType);

  postData = "phoneNumber=" + urlEncode(phoneNumber);
  postData += "&rescueType=" + urlEncode(rescueType);

  payload = "";

  http.begin("http://192.168.254.102/ReconAidDatabase/getdata.php");  
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");  

  httpCode = http.POST(postData); //--> Send the request
  payload = http.getString();     //--> Get the response payload

  Serial.print("httpCode : ");
  Serial.println(httpCode); //--> Print HTTP return code
  Serial.print("payload  : ");
  Serial.println(payload);  //--> Print request response payload

  http.end();

  JSONVar myObject = JSON.parse(payload);

  if (JSON.typeof(myObject) == "undefined") {
    Serial.println("Parsing input failed!");
    Serial.println("---------------");
    return;
  }

  phoneNumberCompare = (const char*) myObject["phoneNumber"];
  rescueTypeCompare = (const char*) myObject["rescueType"];
  
  Serial.print("Received Phone Number: ");
  Serial.println(phoneNumberCompare);
  Serial.print("Received Rescue Type: ");
  Serial.println(rescueTypeCompare);

  if (phoneNumber == phoneNumberCompare) {
    client.println("Data already exists");
  } else {
    client.println("Data does not exist");
  }

  Serial.println("-------------End Receiving Data");

}

void updateDatabase() {
  Serial.println("-------------Sending Data");
    Serial.print("Phone Number to send is: ");
    Serial.println(phoneNumber);
    Serial.print("Name to send is: ");
    Serial.println(Name);
    Serial.print("Rescue Type to send is: ");
    Serial.println(rescueType);
    Serial.print("longitude to send is: ");
    Serial.println(longitude, 8);
    Serial.print("latitude to send is: ");
    Serial.println(latitude, 8);
    

    postData = "phoneNumber=" + urlEncode(phoneNumber);
    postData += "&Name=" + urlEncode(Name);
    postData += "&rescueType=" + urlEncode(rescueType);
    postData += "&Longitude=" + String(longitude, 8);
    postData += "&Latitude=" + String(latitude, 8);

    payload + "";

    http.begin("http://192.168.254.102/ReconAidDatabase/updatedb.php");  //--> Specify request destination
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");  //--> Specify content-type header

    httpCode = http.POST(postData);
    payload = http.getString();

    Serial.print("httpCode : ");
    Serial.println(httpCode); //--> Print HTTP return code
    Serial.print("payload  : ");
    Serial.println(payload);  //--> Print request response payload
    
    http.end();
    Serial.println("-------------End Sending Data");
}

void updateSerial()
{
  delay(500);
  while (Serial.available()) 
  {
    GMS_Serial.write(Serial.read()); // Forward what Serial received to Software Serial Port
  }
  while(GMS_Serial.available()) 
  {
    char c = GMS_Serial.read();
    Serial.write(c); // Forward what Software Serial received to Serial Port
    parseMessage(c);
  }
}

void parseMessage(char c)
{
  static String buffer = "";
  buffer += c;

  if (buffer.indexOf("\r\n+CMT: ") != -1) 
  {
    int start = buffer.indexOf("+CMT: ") + 6;
    int end = buffer.indexOf(",", start);
    senderNumber = buffer.substring(start, end);
    Serial.println("Sender Number: " + senderNumber);
    buffer = buffer.substring(end + 1);
  }

  if (buffer.indexOf("\r\n") != -1) 
  {
    int start = buffer.indexOf("\r\n") + 2;
    int end = buffer.indexOf("\r\n", start);
    if (end != -1)
    {
      message = buffer.substring(start, end);
      Serial.println("Message: " + message);
      buffer = "";
    }
  }
}
