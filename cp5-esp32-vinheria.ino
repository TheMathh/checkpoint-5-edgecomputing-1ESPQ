/*
 * FIAP - Edge Computing & Computer Systems
 * Checkpoint 05 IoT - Vinheria Agnello
 * ESP32 + MQTT
 *
 * Integrantes:
 * Mateus Alves Borges - RM 571424
 * Arthur de Sá Lopes - RM 569526
 * Matheus Carvalho - RM 569454
 */

#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <string.h>

// ===== CONFIGURE AQUI =====

// Rede Wi-Fi
const char* WIFI_SSID = "NOME_DA_SUA_REDE";
const char* WIFI_SENHA = "SENHA_DA_SUA_REDE";

// Servidor MQTT
const char* BROKER_IP = "IP_DO_SEU_BROKER";
const int BROKER_PORTA = 1883; // Altere se necessário

// Identificação MQTT
const char* API_KEY = "SUA_API_KEY";
const char* DEVICE_ID = "SEU_DEVICE_ID";

// ===== PINOS =====

#define PIN_DHT 4
#define PIN_LDR 34
#define PIN_LED 2

#define DHT_TYPE DHT11
DHT dht(PIN_DHT, DHT_TYPE);

// ===== LIMITES DE REFERÊNCIA =====

#define TEMP_OK_MIN 10.0
#define TEMP_OK_MAX 15.0
#define TEMP_ALT_MIN 8.0
#define TEMP_ALT_MAX 18.0

#define HUM_OK_MIN 50.0
#define HUM_OK_MAX 70.0
#define HUM_ALT_MIN 40.0
#define HUM_ALT_MAX 80.0

#define LUZ_OK_MAX 30.0
#define LUZ_ALT_MAX 70.0

// ===== CONFIGURAÇÕES DE LEITURA =====

const int N_AMOSTRAS = 5;
const int ADC_MAX = 4095;

const unsigned long INTERVALO_MS = 5000;
unsigned long ultimoEnvio = 0;

// ===== OBJETOS E TÓPICOS MQTT =====

WiFiClient espClient;
PubSubClient mqtt(espClient);

char topicoAttrs[128];
char topicoCmd[128];
char topicoCmdExe[128];

// ===== RECEBIMENTO DE COMANDOS =====

void callback(char* topic, byte* payload, unsigned int length) {
  String msg;

  for (unsigned int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }

  msg.trim();
  msg.toLowerCase();

  Serial.print("Comando recebido: ");
  Serial.println(msg);

  const char* cmd = nullptr;

  if (msg == "on") {
    cmd = "on";
  } else if (msg == "off") {
    cmd = "off";
  }

  if (cmd == nullptr) {
    Serial.println("Comando desconhecido. Use on ou off.");
    return;
  }

  digitalWrite(PIN_LED, strcmp(cmd, "on") == 0 ? HIGH : LOW);

  char resposta[128];

  snprintf(
    resposta,
    sizeof(resposta),
    "%s@%s|Ok",
    DEVICE_ID,
    cmd
  );

  mqtt.publish(topicoCmdExe, resposta);
}

// ===== CONEXÃO WI-FI =====

void conectaWiFi() {
  Serial.print("Conectando ao Wi-Fi: ");
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_SENHA);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Wi-Fi conectado!");

  Serial.print("IP local do ESP32: ");
  Serial.println(WiFi.localIP());
}

// ===== CONEXÃO MQTT =====

void conectaMQTT() {
  while (!mqtt.connected()) {
    Serial.print("Conectando ao broker MQTT: ");
    Serial.println(BROKER_IP);

    String clientId =
      "esp32-" + String((uint32_t)ESP.getEfuseMac(), HEX);

    if (mqtt.connect(clientId.c_str())) {
      Serial.println("MQTT conectado!");

      if (mqtt.subscribe(topicoCmd)) {
        Serial.print("Inscrito no tópico: ");
        Serial.println(topicoCmd);
      }
    } else {
      Serial.print("Falha na conexão MQTT. Código: ");
      Serial.println(mqtt.state());

      Serial.println("Nova tentativa em 3 segundos.");
      delay(3000);
    }
  }
}

// ===== AVALIAÇÃO DE REFERÊNCIA =====

int avaliaStatus(
  float valor,
  float okMin,
  float okMax,
  float altMin,
  float altMax
) {
  if (valor >= okMin && valor <= okMax) {
    return 0;
  }

  if (valor >= altMin && valor <= altMax) {
    return 1;
  }

  return 2;
}

// ===== LEITURA DO LDR =====

float leLuminosidade() {
  long soma = 0;

  for (int i = 0; i < N_AMOSTRAS; i++) {
    soma += analogRead(PIN_LDR);
    delay(20);
  }

  float media = soma / (float)N_AMOSTRAS;

  float luz = map(
    (long)media,
    0,
    ADC_MAX,
    0,
    100
  );

  return constrain(luz, 0, 100);
}

// ===== CONFIGURAÇÃO INICIAL =====

void setup() {
  Serial.begin(115200);

  pinMode(PIN_LDR, INPUT);
  pinMode(PIN_LED, OUTPUT);

  digitalWrite(PIN_LED, LOW);

  dht.begin();

  // Os tópicos são montados conforme as configurações do usuário.
  snprintf(
    topicoAttrs,
    sizeof(topicoAttrs),
    "/%s/%s/attrs",
    API_KEY,
    DEVICE_ID
  );

  snprintf(
    topicoCmd,
    sizeof(topicoCmd),
    "/%s/%s/cmd",
    API_KEY,
    DEVICE_ID
  );

  snprintf(
    topicoCmdExe,
    sizeof(topicoCmdExe),
    "/%s/%s/cmdexe",
    API_KEY,
    DEVICE_ID
  );

  conectaWiFi();

  mqtt.setServer(BROKER_IP, BROKER_PORTA);
  mqtt.setCallback(callback);

  Serial.println("Sistema de monitoramento iniciado.");
}

// ===== EXECUÇÃO CONTÍNUA =====

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    conectaWiFi();
  }

  if (!mqtt.connected()) {
    conectaMQTT();
  }

  mqtt.loop();

  if (millis() - ultimoEnvio < INTERVALO_MS) {
    return;
  }

  ultimoEnvio = millis();

  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  float luz = leLuminosidade();

  if (isnan(temp) || isnan(hum)) {
    Serial.println(
      "Erro ao ler o DHT11. Verifique as conexões."
    );
    return;
  }

  // Avaliação dos valores conforme os limites definidos.
  int stTemp = avaliaStatus(
    temp,
    TEMP_OK_MIN,
    TEMP_OK_MAX,
    TEMP_ALT_MIN,
    TEMP_ALT_MAX
  );

  int stHum = avaliaStatus(
    hum,
    HUM_OK_MIN,
    HUM_OK_MAX,
    HUM_ALT_MIN,
    HUM_ALT_MAX
  );

  int stLuz =
    (luz <= LUZ_OK_MAX) ? 0 :
    (luz <= LUZ_ALT_MAX) ? 1 : 2;

  int stGeral = max(stTemp, max(stHum, stLuz));

  // Publicação dos dados em formato JSON.
  char payload[128];

  snprintf(
    payload,
    sizeof(payload),
    "{\"temperatura\":%.1f,\"umidade\":%.1f,\"luminosidade\":%d}",
    temp,
    hum,
    (int)luz
  );

  bool ok = mqtt.publish(topicoAttrs, payload);

  if (ok) {
    Serial.print("Publicado em ");
    Serial.print(topicoAttrs);
    Serial.print(": ");
    Serial.println(payload);
  } else {
    Serial.println("Falha ao publicar os dados.");
  }
}
