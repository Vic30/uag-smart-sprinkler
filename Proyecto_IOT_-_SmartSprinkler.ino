#include <PubSubClient.h>

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "UAGEDU";
const char* password = "";
const char* mqtt_server = "m11.cloudmqtt.com";
const char* mqtt_user = "joxipsrc";
const char* mqtt_password = "5etvnhfH19sg";

//Motor
int BOMBA = D1;
//Sensor humedad
const int sensorHumedad = D3;
//Sensor lluvia
const int sensorLluvia = D0;


WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void setup_wifi() {

  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Topic arrived [");
  Serial.print(topic);
  Serial.print("] ");
  Serial.println();

  if(strcmp(topic,"bomba1")==0){
    Serial.println("[*] Bomba1");
    if ((char)payload[0] == '1') {
      digitalWrite(BOMBA, HIGH);//sets the motors speed
      Serial.println("bomba high");
    } else {
      digitalWrite(BOMBA, LOW);//sets the motors speed
      Serial.println("bomba low");
    }
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_password)) {
      Serial.println("connected");

      client.publish("sprinkler1", "Sprinkler 1 connected");
      client.subscribe("bomba1");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
  
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  pinMode(BOMBA, OUTPUT);   // where the motor is connected to
  //pinMode(sensorHumedad, OUTPUT);
  pinMode(sensorLluvia, INPUT);
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 17738);
  client.setCallback(callback);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();


  int humedad = digitalRead(sensorHumedad);
  int lluvia = digitalRead(sensorLluvia);
  int bombaStatus = digitalRead(BOMBA);

  //Seco sin lluvia -> prender
  if (humedad == HIGH && lluvia == HIGH && bombaStatus == LOW) {
    Serial.println("[*] Regando Jardinera 1 - Seco sin lluvia");
    client.publish("sprinkler1", "Regando");
    client.publish("bomba1", "1");
  }else

  //Seco con lluvia -> apagar
  if (humedad == HIGH && lluvia == LOW && bombaStatus == HIGH) {
    Serial.println("[*] Jardinera 1 Apagada - Seco con lluvia");
    client.publish("sprinkler1", "Apagada");
    client.publish("bomba1", "0");
  }else

  //Humedo con lluvia -> apagar
  if (humedad == LOW && lluvia == LOW && bombaStatus == HIGH) {
    Serial.println("[*] Jardinera 1 Apagada - Humedo con lluvia");
    client.publish("sprinkler1", "Apagada");
    client.publish("bomba1", "0");
  }else

  //Humedo sin lluvia -> apagar
  if (humedad == LOW && lluvia == HIGH && bombaStatus == HIGH) {
    Serial.println("[*] Jardinera 1 Apagada - Humedo sin lluvia");
    client.publish("sprinkler1", "Apagada");
    client.publish("bomba1", "0");
  }
  delay(500);
}
