// FIAP - Edge Computing & Computer Systems
// Checkpoint 05 IoT - Vinheria Agnello (ESP32 + FIWARE)
//
// Integrantes:
//   Mateus Alves Borges   RM 571424
//   Arthur de Sá Lopes    RM 569526
//   Matheus Carvalho      RM 569454

#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// ── CONFIGURE AQUI ──────────────────────────────
const char* WIFI_SSID  = "FIAP_IOT2";
const char* WIFI_SENHA = "F!@p25.IOT";
const char* BROKER_IP  = "3.135.234.101";    
const int   BROKER_PORTA = 1883;

const char* API_KEY   = "TEF";
const char* DEVICE_ID = "vinheria571424";


#define PIN_DHT 4     
#define PIN_LDR 34    
#define PIN_LED 2     

#define DHT_TYPE DHT11
DHT dht(PIN_DHT, DHT_TYPE);


#define TEMP_OK_MIN   10.0
#define TEMP_OK_MAX   15.0
#define TEMP_ALT_MIN   8.0
#define TEMP_ALT_MAX  18.0

#define HUM_OK_MIN   50.0
#define HUM_OK_MAX   70.0
#define HUM_ALT_MIN  40.0
#define HUM_ALT_MAX  80.0

#define LUZ_OK_MAX   30.0
#define LUZ_ALT_MAX  70.0

const int N_AMOSTRAS = 5;
const int ADC_MAX = 4095;

const unsigned long INTERVALO_MS = 5000;
unsigned long ultimoEnvio = 0;

WiFiClient espClient;
PubSubClient mqtt(espClient);

char topicoAttrs[64];
char topicoCmd[64];
char topicoCmdExe[64];


void callback(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];
  msg.trim();
  Serial.print("Comando recebido: ");
  Serial.println(msg);

  const char* cmd = nullptr;
  if (msg.indexOf("off") >= 0)     cmd = "off";
  else if (msg.indexOf("on") >= 0) cmd = "on";

  if (cmd == nullptr) {
    Serial.println("Comando desconhecido (use on ou off).");
    return;
  }

  digitalWrite(PIN_LED, strcmp(cmd, "on") == 0 ? HIGH : LOW);

  
  char resp[64];
  snprintf(resp, sizeof(resp), "%s@%s|Ok", DEVICE_ID, cmd);
  mqtt.publish(topicoCmdExe, resp);
}

void conectaWiFi() {
  Serial.print("Conectando ao Wi-Fi ");
  Serial.println(WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_SENHA);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Wi-Fi conectado. IP do ESP32: ");
  Serial.println(WiFi.localIP());
}

void conectaMQTT() {
  while (!mqtt.connected()) {
    Serial.print("Conectando ao broker MQTT ");
    Serial.print(BROKER_IP);
    Serial.print("... ");
    String clientId = "vinheria-esp32-" + String((uint32_t)ESP.getEfuseMac(), HEX);
    if (mqtt.connect(clientId.c_str())) {
      Serial.println("conectado!");
      mqtt.subscribe(topicoCmd);
    } else {
      Serial.print("falhou, rc=");
      Serial.print(mqtt.state());
      Serial.println(" - nova tentativa em 3 s");
      delay(3000);
    }
  }
}

int avaliaStatus(float val, float okMin, float okMax, float altMin, float altMax) {
  if (val >= okMin  && val <= okMax)  return 0;
  if (val >= altMin && val <= altMax) return 1;
  return 2;
}

float leLuminosidade() {
  long soma = 0;
  for (int i = 0; i < N_AMOSTRAS; i++) {
    soma += analogRead(PIN_LDR);
    delay(20);
  }
  float media = soma / (float)N_AMOSTRAS;
  float luz = map((long)media, 0, ADC_MAX, 0, 100);
  return constrain(luz, 0, 100);
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_LDR, INPUT);
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);
  dht.begin();

  snprintf(topicoAttrs,  sizeof(topicoAttrs),  "/%s/%s/attrs",  API_KEY, DEVICE_ID);
  snprintf(topicoCmd,    sizeof(topicoCmd),    "/%s/%s/cmd",    API_KEY, DEVICE_ID);
  snprintf(topicoCmdExe, sizeof(topicoCmdExe), "/%s/%s/cmdexe", API_KEY, DEVICE_ID);

  conectaWiFi();
  mqtt.setServer(BROKER_IP, BROKER_PORTA);
  mqtt.setCallback(callback);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) conectaWiFi();
  if (!mqtt.connected()) conectaMQTT();
  mqtt.loop();

  if (millis() - ultimoEnvio < INTERVALO_MS) return;
  ultimoEnvio = millis();

  float temp = dht.readTemperature();
  float hum  = dht.readHumidity();
  float luz  = leLuminosidade();

  if (isnan(temp) || isnan(hum)) {
    Serial.println("Erro ao ler o DHT11. Verifique a ligação.");
    return;
  }

  int stTemp  = avaliaStatus(temp, TEMP_OK_MIN, TEMP_OK_MAX, TEMP_ALT_MIN, TEMP_ALT_MAX);
  int stHum   = avaliaStatus(hum,  HUM_OK_MIN,  HUM_OK_MAX,  HUM_ALT_MIN,  HUM_ALT_MAX);
  int stLuz   = (luz <= LUZ_OK_MAX) ? 0 : (luz <= LUZ_ALT_MAX) ? 1 : 2;
  int stGeral = max(stTemp, max(stHum, stLuz));

  char payload[128];
  snprintf(payload, sizeof(payload),
           "{\"temperatura\":%.1f,\"umidade\":%.1f,\"luminosidade\":%d}",
           temp, hum, (int)luz);

  bool ok = mqtt.publish(topicoAttrs, payload);

  if (ok) {
    Serial.print("Publicado: ");
    Serial.println(payload);
  } else {
    Serial.println("Falha ao publicar.");
  }

}