#include <ESPert.h>

ESPert espert;

int temp=700;

const char* mqtt_server = "mqtt.espert.io";

unsigned long loopTime = 0;

int currentSwitch = true;
String outTopic = "ESPert/" + String(espert.info.getChipId()) + "/Status";
String inTopic = "ESPert/" + String(espert.info.getChipId()) + "/Command";

int currentSwitch2 = true;
ESPert_Button button2;

const char* host = "maker.ifttt.com";
const int httpPort = 80;
String ifttt_key = "";

void callback(char* topic, byte* payload, unsigned int length) {
  String strPayload = String((char*)payload).substring(0, length);
  espert.println("Receive: " + strPayload);

  espert.oled.clear();
  espert.oled.println(espert.info.getId());
  espert.oled.println();
  espert.oled.printf("[%lu]\r\n\r\n", millis());
  espert.oled.println("PARSE OK.");


  if (espert.json.init(strPayload)) {
    if (espert.json.containsKey("DIRECTION")) {
      String value = espert.json.get("DIRECTION");

      if (value == "F") {
        digitalWrite(2, HIGH);          //Left Motor
        digitalWrite(14, HIGH);         //Right Motor
        analogWrite(12, 1023);          //Left PWM
        analogWrite(15, 1023);          //Right PWM
        String outString = "{\"WHEELED ROBOT\":\"GO FORWARD\"}";
        espert.println("WHEELED ROBOT: GO FORWARD");
        espert.mqtt.publish(outTopic, outString);
      } if (value == "B") {
        digitalWrite(2, LOW);
        digitalWrite(14, LOW);
        analogWrite(12, 900);
        analogWrite(15, 900);
        String outString = "{\"WHEELED ROBOT\":\"GO BACKWARD\"}";
        espert.println("WHEELED ROBOT: GO BACKWARD");
        espert.mqtt.publish(outTopic, outString);
      } if (value == "R") {
        digitalWrite(2, HIGH);
        digitalWrite(14, HIGH);
        analogWrite(12, 900);
        analogWrite(15, 0);
        String outString = "{\"WHEELED ROBOT\":\"TURN RIGHT\"}";
        espert.println("WHEELED ROBOT: TURN RIGHT");
        espert.mqtt.publish(outTopic, outString);
      } if (value == "L") {
        digitalWrite(2, HIGH);
        digitalWrite(14, HIGH);
        analogWrite(12, 0);
        analogWrite(15, 900);
        String outString = "{\"WHEELED ROBOT\":\"TURN LEFT\"}";
        espert.println("WHEELED ROBOT: TURN LEFT");
        espert.mqtt.publish(outTopic, outString);
      } if (value == "S") {
        analogWrite(12, 0);
        analogWrite(15, 0);
        String outString = "{\"WHEELED ROBOT\":\"STOP\"}";
        espert.println("WHEELED ROBOT: STOP");
        espert.mqtt.publish(outTopic, outString);
      }
    }

    if (espert.json.containsKey("OLED")) {
      String value = espert.json.get("OLED");
      espert.oled.clear();
      espert.oled.println(value);
    }

    if (espert.json.containsKey("IFTTT")) {
      String value = espert.json.get("IFTTT");
      ifttt_key = value;
      espert.eeprom.write(150, ifttt_key);

      String outString = "{\"IFTTT\":\"" + ifttt_key + "\", ";
      outString += "\"name\":\"" + String(espert.info.getId()) + "\"}";
      espert.println("Send...: " + outString);
      espert.mqtt.publish(outTopic, outString);
    }
  }
  else {
    espert.oled.clear();
    espert.oled.println(espert.info.getId());
    espert.oled.println();
    espert.oled.printf("[%lu]\r\n\r\n", millis());
    espert.oled.println("PARSE FAILED.");
  }
}

void setup() {
  espert.init();
  delay(100);
  espert.mqtt.init(mqtt_server, 1883, callback);

  ifttt_key = espert.eeprom.read(150, 80);

  button2.init(0);

  espert.oled.init();
  delay(2000);

  espert.println("Press USER button to switch Mode");

  espert.oled.clear();
  espert.oled.println(espert.info.getId());
  espert.oled.println();

  int mode = espert.wifi.init();

  if (mode == ESPERT_WIFI_MODE_CONNECT) {
    espert.println(">>> WiFi mode: connected.");
    espert.oled.println("WiFi: connected.");
    espert.oled.print("IP..: ");
    espert.oled.println(espert.wifi.getLocalIP());
  } else if (mode == ESPERT_WIFI_MODE_DISCONNECT) {
    espert.println(">>> WiFi mode: disconnected.");
    espert.oled.println("WiFi: not connected.");
  }

  loopTime = millis();

  pinMode (0, INPUT);
  pinMode (13, INPUT);
  pinMode(2, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(14, OUTPUT);
  pinMode(15, OUTPUT);
  digitalWrite(3, LOW);
  digitalWrite(14, LOW);
  analogWrite(12, 0);
  analogWrite(15, 0);
}

void loop() {
  espert.loop();

  if (espert.mqtt.connect()) {
    espert.println("MQTT: Connected");
    espert.println("MQTT: Out Topic " + outTopic);
    espert.mqtt.subscribe(inTopic);
    Serial.println("MQTT: Subscribed " + inTopic);
  }

  bool buttonPressed = espert.button.isOn();

  if (buttonPressed != currentSwitch) {
    String outString = "{\"button\":\"" + String(buttonPressed ? 1 : 0) + "\", ";
    outString += "\"name\":\"" + String(espert.info.getId()) + "\"}";
    espert.println("Send...: " + outString);
    espert.mqtt.publish(outTopic, outString);
    currentSwitch = buttonPressed;
  }

  buttonPressed = button2.isOn();

  if (buttonPressed != currentSwitch2) {
    espert.println("SW");

    if (buttonPressed) {
      String path = "/trigger/button/with/key/" + ifttt_key;
      espert.println(">>" + espert.wifi.postHTTP(host, path.c_str()) + "<<");
    }

    currentSwitch2 = buttonPressed;
  }

  delay(100);
}
