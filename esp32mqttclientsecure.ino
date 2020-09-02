/*
 * Example sketch to work with MH-Z19B CO2 and AM2302 Temperature and Humidity sensor
 */
#include <SimpleDHT.h>
#include <HardwareSerial.h>
#include <PubSubClient.h>

#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <HttpClient.h>

int pinDHT22 = 2;
SimpleDHT22 dht22(pinDHT22);

#define INTERVAL 60000

long previousMillis = 0;

// UART 2 to CO2 sensor
HardwareSerial co2Serial(2); 

// WiFi settings
#define wifi_ssid "BanzaiNet"
#define wifi_password "DasIstDasHausVomNikolaus0402"

// Certificates for SAP IoT
const char* rootCA = "-----BEGIN CERTIFICATE-----\n" \
"......" \
"-----END CERTIFICATE-----\n";
  
const char* certificate = "-----BEGIN CERTIFICATE-----\n" \
"MIIEbzCCA1egAwIBAgIQANF9PJaYepJOEAEHAJbcwzANBgkqhkiG9w0BAQsFADBW\n" \
"MQswCQYDVQQGEwJERTEjMCEGA1UEChMaU0FQIElvVCBUcnVzdCBDb21tdW5pdHkg\n" \
"SUkxIjAgBgNVBAMTGVNBUCBJbnRlcm5ldCBvZiBUaGluZ3MgQ0EwHhcNMjAwODI5\n" \
"MTA0ODM1WhcNMjEwODI5MTA0ODM1WjCBuzELMAkGA1UEBhMCREUxHDAaBgNVBAoT\n" \
"E1NBUCBUcnVzdCBDb21tdW5pdHkxFTATBgNVBAsTDElvVCBTZXJ2aWNlczF3MHUG\n" \
"A1UEAxRuZGV2aWNlQWx0ZXJuYXRlSWQ6dG9reW9FU1AzMkVudnxnYXRld2F5SWQ6\n" \
"Mnx0ZW5hbnRJZDo5NDkwNDM5MDh8aW5zdGFuY2VJZDo1N2ZjMGE4Yy03NjFkLTRm\n" \
"OWYtOGYzOS01MmZmMzQ1ZGJmZTkwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEK\n" \
"AoIBAQCBEyqJG013TBvjyQw7tj2KqRve8wJEBCoZ4mE23jM3vWK3rVCOvQFCPpRm\n" \
"sXGe3LjF9T9ZbbseMeZgcoQUFQpbUphON4ur0eRsuKFWLGHf272CWOtqliKUDu4o\n" \
"IZc/cDaPNYrKBLiAAp/DC5xf4nZB+2brQvS1V7ERrhn7UkfhM7ARfjsz91Sg9u85\n" \
"vRvaohiMiShNa7TBMUrFNRv0eWpGP5oLzNTCCj+bNNORrXxd6xfA6DKUAwteYoTO\n" \
"ybeJf+6i9Lf4M+oCPhLpQK5fdDrm3/hSwMnbd82McHOkjx0TXuXolwiWPQ0FBk4C\n" \
"IvOQzlrqsnVrnvhZ5c1R7pb1BXDPAgMBAAGjgdIwgc8wSAYDVR0fBEEwPzA9oDug\n" \
"OYY3aHR0cHM6Ly90Y3MubXlzYXAuY29tL2NybC9UcnVzdENvbW11bml0eUlJL1NB\n" \
"UElvVENBLmNybDAMBgNVHRMBAf8EAjAAMCUGA1UdEgQeMByGGmh0dHA6Ly9zZXJ2\n" \
"aWNlLnNhcC5jb20vVENTMA4GA1UdDwEB/wQEAwIGwDAdBgNVHQ4EFgQUfiWlnsiy\n" \
"vUyPissNOKFEXCx2uD4wHwYDVR0jBBgwFoAUlbez9Vje1bSzWEbg8qbJeE69LXUw\n" \
"DQYJKoZIhvcNAQELBQADggEBAKEB4ca7dXSBc/7qAas8N9uQ5bZi8GrOi5zDEg4P\n" \
"hS0FsvRD5RL+5FUG+3cq5pac6tk0asU92bduayyX23C/tCUjgI46d7kwreowxnwi\n" \
"qj2+eVthx/PhrqKBXp/EttgAzzMS5ukdGci24PwY+5GLeJ25aj2/WDSxyKRYkVJO\n" \
"8ZpzjOwy5iCYjj6CwaU5Nquoj6PxqBvsBZuWUtG0C5veiqo3W0InB3EeKv+XxCOm\n" \
"GS7RbTbAlPZp9SiyI8mRA6BPgnAkEO8AHXNv9kRpiT6NCh7+DeIzhGW9JbnIyKq6\n" \
"fa04Vlr/5ETFN0BvPVShi+OYu2aKUGWS5HixnlCcoRn+C2k=\n" \
"-----END CERTIFICATE-----\n";
  
const char* privateKey = "-----BEGIN RSA PRIVATE KEY-----\n" \
"MIIEpAIBAAKCAQEAgRMqiRtNd0wb48kMO7Y9iqkb3vMCRAQqGeJhNt4zN71it61Q\n" \
"jr0BQj6UZrFxnty4xfU/WW27HjHmYHKEFBUKW1KYTjeLq9HkbLihVixh39u9gljr\n" \
"apYilA7uKCGXP3A2jzWKygS4gAKfwwucX+J2Qftm60L0tVexEa4Z+1JH4TOwEX47\n" \
"M/dUoPbvOb0b2qIYjIkoTWu0wTFKxTUb9HlqRj+aC8zUwgo/mzTTka18XesXwOgy\n" \
"lAMLXmKEzsm3iX/uovS3+DPqAj4S6UCuX3Q65t/4UsDJ23fNjHBzpI8dE17l6JcI\n" \
"lj0NBQZOAiLzkM5a6rJ1a574WeXNUe6W9QVwzwIDAQABAoIBAHDit0/EHiMY4Ed+\n" \
"nQFYKEiuD98UP0YHMl8C+yzGdproOn8TyCGeMYyjOJz70bgzqDIRhH8NFhqZX7e4\n" \
"T8VUhYNLIENAFxqFbjab3xaDqHHlN7/ZTsMgXUuKJ0XX4vSrijwDgseeC1KBeQJX\n" \
"lELTx8N7QQ6+t1AmSomA7946nvzYPszDgr3iK1YQx0JTvpwqJo7rPg9MdPrETabM\n" \
"FlwVjnXAwym0aKPdiaHGZbMrDxzSd4MTpC0vVWJP89O/n/d7ryGg0EBO2DMvtY/5\n" \
"j2flK8IxyrxYTglTXRLWsxaJi1xRAIBwgDOS8AFyRrB+tJKAV/BbUKMW43eFM/WO\n" \
"NxrxXLkCgYEAtn0bUUu/c1qwPoyb2UTuZIPGUKJ/Y+LVuZ1f+eHBf5VfC6EXTVXc\n" \
"2jokBVKUWN0U8+15KL/IAxsmMS3Qh1v+ztm9UcLMI0j/moedkQqM+/OdTeRImvyM\n" \
"ewpRJkCc7oWREnNUpASWdgJfHUw+PC0VPKEhEAIztqea7t7bDld+2d0CgYEAtRHV\n" \
"G6pWPK69b8LmKmQIRot7+ErO/01pxdF+dARmSV9dVY2bXiktHeuGCGe/dPGlXGvr\n" \
"h2b+bODpO71K8TbvOzZzfHI7kfUiXUff3oFhqYewf8AGieErIuMCGM+flLbZN1MT\n" \
"+/T0r8LpeItikelBT1fS0wUGjhC+oZG6dR+RKJsCgYEAg7OyvW9Pkq8Gt32/FIUd\n" \
"D8nSnGtKSQpyeO1NUQhPIAbre3cdXCD1rUF8saKxkZitAIaIhJI0hZlYbPHFp4sL\n" \
"ULnwWkueDHEol5v8aasVqNhe1NJGXCScNkhx3IuRNsuKsaguk9kDxdNcVcaqu79F\n" \
"rmYTeHw+WjqIJJENw0H+OLkCgYEAi+iWgQTyE3t1lrjC/fhDSLcYeDbdkPlxBFAm\n" \
"ZGDN5F3SkXuUlII+gULuUupPTn41Jp+g4wPbDagPr4BJOu8WlOsy+vKNQu9luSwD\n" \
"ODtCiir1XxCyPLGGkjNT+HXdOZy7ZHtGEsYolvKFUgZpiJLySutyVGkqsnUPx2oB\n" \
"5+QXVocCgYBbrxOVybOn9M77YPfbfJt+YZ7P4PKmj/KSAnNDiPigsFqi3ptYqcEF\n" \
"bAqi35bHPhDJ3JuQQ1LdY3sLfWMZ9wrG3EmyIzkBBh+ofZLmD5sSzJh8eazAOtYn\n" \
"dfzRlQ0S6zRQjftJmI9bR5HmLBfAqbq1eRyJnfLS3k2ZvLuhfBB29Q==\n" \
"-----END RSA PRIVATE KEY-----\n";

// Connection settings to the MQTT Broker
const char* sapIoTServerUrl = "57fc0a8c-761d-4f9f-8f39-52ff345dbfe9.eu10.cp.iot.sap";
const int sapIoTServerPort = 8883;
// Create the MQTT Client
WiFiClientSecure httpsClient; // Create the client to use WiFi for MQTT
PubSubClient mqttClient(httpsClient);
mqttClient.setBufferSize(512); // Set message size up from 256 standard

void setup() {
  Serial.begin(115200);
  
  // Initialize the BUILTIN_LED pin as an output
  pinMode(BUILTIN_LED, OUTPUT);    
   
  // Start WiFi 
  setup_wifi();           

  // Setup MQTT client
  //httpsClient.setCACert(rootCA);
  httpsClient.setCertificate(certificate);
  httpsClient.setPrivateKey(privateKey);
  mqttClient.setServer(sapIoTServerUrl, sapIoTServerPort);
  mqttClient.setCallback(mqttCallback);

  // Start UART communication with MZ-Z19 sensor
  unsigned long previousMillis = millis();
  co2Serial.begin(9600); //Init sensor MH-Z19B
}

void loop() {

  // Here we make sure that our MQTT client stays connected
  if (!mqttClient.connected()) {
    reconnect();
  }
  // The MQTT Client call
  mqttClient.loop();

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis < INTERVAL)
    return;
 
  previousMillis = currentMillis;

  // Read sensor
  int ppm = readCO2();
  
  // Prepare output for CO2
  String sCo2Value = "CO2 concentration " + String(ppm) + "ppm.";
  Serial.println(sCo2Value);

  // Read temp/humidity sensor
  float temperature = 0;
  float humidity = 0;

  int err = SimpleDHTErrSuccess;
  char message[400];

  if ((err = dht22.read2(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess) {
    Serial.print("Read DHT22 failed, err="); Serial.println(err);
  } else { 
    String sTempHumid = "Temp: " + String((float)temperature) + "°C. Rel. Humidity: " + String((float)humidity) + "RH%";
    Serial.println(sTempHumid);

    sprintf(message, "{ \"sensorAlternateId\": \"esp32EnvData01\", \"capabilityAlternateId\": \"esp32envData\",  \"measures\":[ { \"temperature\": %5.2f, \"humidity\": %5.2f, \"CO2\": %d } ] }", temperature, humidity, ppm);
    boolean rc = mqttClient.publish("measures/tokyoESP32Env", message, true);
    Serial.println(message);
    Serial.print("Returncode: "); Serial.println(rc);
    //Serial.println(message); 
  }
}

//WiFi Setup
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connection OK ");
  Serial.print("=> IP Address: ");
  Serial.println(WiFi.localIP());
}

int readCO2() {
  // CO2 Sensor command
  byte cmd[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
  // command to ask for data
  byte response[9]; // for answer

  co2Serial.write(cmd, 9); //request PPM CO2

  // The serial stream can get out of sync. The response starts with 0xff, try to resync.
  while (co2Serial.available() > 0 && (unsigned char)co2Serial.peek() != 0xFF) {
    co2Serial.read();
  }

  memset(response, 0, 9);
  co2Serial.readBytes(response, 9);

  if (response[1] != 0x86)
  {
    Serial.println("Invalid response from co2 sensor!");
    return -1;
  }

  byte crc = 0;
  for (int i = 1; i < 8; i++) {
    crc += response[i];
  }
  crc = 255 - crc + 1;

  if (response[8] == crc) {
    int responseHigh = (int) response[2];
    int responseLow = (int) response[3];
    int ppm = (256 * responseHigh) + responseLow;
    return ppm;
  } else {
    Serial.println("CRC error!");
    return -1;
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqttClient.connect("tokyoESP32Env")) {
      Serial.println("connected");
      // Subscribe to SAP ACKs for the device
      mqttClient.subscribe("ack/tokyoESP32Env");
    } 
    else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] - ");
  char *cPayload = (char*)payload;
  Serial.print(cPayload);
}
