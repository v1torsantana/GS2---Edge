#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>

// Configurações de WiFi
const char *SSID = "Wokwi-GUEST";
const char *PASSWORD = "";  // Substitua pelo sua senha

// Configurações de MQTT
const char *BROKER_MQTT = "broker.hivemq.com";
const int BROKER_PORT = 1883;
const char *ID_MQTT = "Vitor_GS_mqtt";
const char *TOPIC_SUBSCRIBE_LED = "fiap/aaa";
const char *TOPIC_PUBLISH_HORMONIOS = "fiap/gs/VitorLucca";

#define PIN_LCD 0x27
const int servoPin1 = 15;
const int servoPin2 = 2;
const int servoPin3 = 4;
const int ledRED = 13;
const int ledGREEN = 12;

WiFiClient espClient;
PubSubClient MQTT(espClient);
unsigned long lastUpdateTime = 0;
const unsigned long updateInterval = 2000;

Servo servo1;
Servo servo2;
Servo servo3;

LiquidCrystal_I2C lcd(PIN_LCD, 20, 4);

int qtdEstrogenio = 7;
int qtdProgesterona = 8;
int qtdTestosterona = 4;

void initWiFi();
void initMQTT();
void callbackMQTT(char *topic, byte *payload, unsigned int length);
void reconnectMQTT();
void reconnectWiFi();
void checkWiFIAndMQTT();

void initWiFi() {
  Serial.print("Conectando com a rede: ");
  Serial.println(SSID);
  WiFi.begin(SSID, PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Conectado com sucesso: ");
  Serial.println(SSID);
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void initMQTT() {
  MQTT.setServer(BROKER_MQTT, BROKER_PORT);
  MQTT.setCallback(callbackMQTT);
}

void callbackMQTT(char *topic, byte *payload, unsigned int length) {
}

void reconnectMQTT() {
  while (!MQTT.connected()) {
    Serial.print("Tentando conectar com o Broker MQTT: ");
    Serial.println(BROKER_MQTT);

    if (MQTT.connect(ID_MQTT)) {
      Serial.println("Conectado ao broker MQTT!");
      MQTT.subscribe(TOPIC_SUBSCRIBE_LED);
    } else {
      Serial.println("Falha na conexão com MQTT. Tentando novamente em 2 segundos.");
      delay(2000);
    }
  }
}

void checkWiFIAndMQTT() {
  if (WiFi.status() != WL_CONNECTED) reconnectWiFi();
  if (!MQTT.connected()) reconnectMQTT();
}

void reconnectWiFi(void) {
  if (WiFi.status() == WL_CONNECTED)
    return;

  WiFi.begin(SSID, PASSWORD); // Conecta na rede WI-FI

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Wifi conectado com sucesso");
  Serial.print(SSID);
  Serial.println("IP: ");
  Serial.println(WiFi.localIP());
}

void enviarMensagemMQTT(const char *topico, const char *mensagem) {
  Serial.printf("Enviando mensagem via MQTT: %s para o tópico: %s\n", mensagem, topico);
  MQTT.publish(topico, mensagem);
  Serial.println("Mensagem MQTT enviada com sucesso!");
}


void setup() {
  servo1.attach(servoPin1, 500, 2400);
  servo2.attach(servoPin2, 500, 2400);
  servo3.attach(servoPin3, 500, 2400);

  pinMode(ledRED, OUTPUT);
  pinMode(ledGREEN, OUTPUT);

  initWiFi();
  initMQTT();

  lcd.init();
  lcd.backlight();
}

// ...

void loop() {
  checkWiFIAndMQTT();
  MQTT.loop();

  int tempoEstrogenio = 11;
  int tempoProgesterona = 11;
  int tempoTestosterona = 11;

  unsigned long currentMillis = millis();

  if (currentMillis - lastUpdateTime >= updateInterval) {
    lastUpdateTime = currentMillis;

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Controle Hormonal");

    lcd.setCursor(0, 1);
    lcd.print("Estrogenio: ");
    lcd.print(qtdEstrogenio);

    lcd.setCursor(0, 2);
    lcd.print("Progesterona: ");
    lcd.print(qtdProgesterona);

    lcd.setCursor(0, 3);
    lcd.print("Testosterona: ");
    lcd.print(qtdTestosterona);

    if (tempoEstrogenio > 10 && qtdEstrogenio < 10) {
      lcd.setCursor(0, 1);
      lcd.print("Liberando Estrogenio");
      digitalWrite(ledRED, HIGH);
      digitalWrite(ledGREEN, LOW);
      servo1.write(180);
      delay(2000);
      servo1.write(0);
      qtdEstrogenio++;

      if (qtdEstrogenio >= 10) {
        lcd.setCursor(0, 1);
        lcd.print("Estrogenio OK!");
        digitalWrite(ledRED, LOW);
        digitalWrite(ledGREEN, HIGH);
        enviarMensagemMQTT(TOPIC_PUBLISH_HORMONIOS, "Estrogênio OK!");
      }
    }

    if (tempoProgesterona > 10 && qtdProgesterona < 10) {
      lcd.setCursor(0, 2);
      lcd.print("Liberando Progesterona");
      digitalWrite(ledRED, HIGH);
      digitalWrite(ledGREEN, LOW);
      servo2.write(180);
      delay(2000);
      servo2.write(0);
      qtdProgesterona++;

      if (qtdProgesterona >= 10) {
        lcd.setCursor(0, 2);
        lcd.print("Progesterona OK!");
        digitalWrite(ledRED, LOW);
        digitalWrite(ledGREEN, HIGH);
        enviarMensagemMQTT(TOPIC_PUBLISH_HORMONIOS, "Progesterona OK!");
      }
    }

    if (tempoTestosterona > 10 && qtdTestosterona < 10) {
      lcd.setCursor(0, 3);
      lcd.print("Liberando Testo");
      digitalWrite(ledRED, HIGH);
      digitalWrite(ledGREEN, LOW);
      servo3.write(180);
      delay(2000);
      servo3.write(0);
      qtdTestosterona++;

      if (qtdTestosterona >= 10) {
        lcd.setCursor(0, 3);
        lcd.print("Testosterona OK!");
        digitalWrite(ledRED, LOW);
        digitalWrite(ledGREEN, HIGH);
        enviarMensagemMQTT(TOPIC_PUBLISH_HORMONIOS, "Testosterona OK!");
      }
    }
  }
}