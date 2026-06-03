
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define NOME_REDE   "Alexandre"
#define SENHA_REDE  "xawc8090"
const String urlServidor = "http://172.19.243.39/projeto_sesi/salvar_dados.php";

#define PINO_SENSOR_HALL 4

volatile unsigned long tempoUltimoPulso  = 0;
volatile unsigned long intervaloPulsos   = 0;
volatile bool          primeiroPulso     = true;

const float diametroRoda = 0.60;
float circunferenciaRoda;

float velocidadeKmh = 0;
float rpm           = 0;

unsigned long tempoUltimoEnvio          = 0;
const unsigned long intervaloEnvio      = 2000;
void IRAM_ATTR detectarIma() {
    unsigned long tempoAtual = micros();
    if (tempoAtual - tempoUltimoPulso > 5000) {
        if (!primeiroPulso) {
            intervaloPulsos = tempoAtual - tempoUltimoPulso;
        } else {
            primeiroPulso = false;
        }
        tempoUltimoPulso = tempoAtual;
    }
}
void setup() {
    Serial.begin(115200);
    Serial.println("\n--- Velocímetro IoT ESP32 ---");
    WiFi.begin(NOME_REDE, SENHA_REDE);
    Serial.print("Conectando ao Wi-Fi");

    int tentativas = 0;
    while (WiFi.status() != WL_CONNECTED && tentativas < 30) {
        delay(500);
        Serial.print(".");
        tentativas++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWi-Fi conectado! IP: " + WiFi.localIP().toString());
    } else {
        Serial.println("\nAVISO: Wi-Fi não disponível. Operando offline.");
    }

    pinMode(PINO_SENSOR_HALL, INPUT_PULLUP);

    circunferenciaRoda = diametroRoda * 3.141592;

    attachInterrupt(digitalPinToInterrupt(PINO_SENSOR_HALL), detectarIma, FALLING);

    Serial.println("Sistema pronto.");
}

void loop() {

    noInterrupts();
    unsigned long intervalo              = intervaloPulsos;
    unsigned long tempoUltimoPulsoLocal  = tempoUltimoPulso;
    bool          primeiroLocal          = primeiroPulso;
    interrupts();

    unsigned long tempoDesdeUltimoPulso = micros() - tempoUltimoPulsoLocal;

    if (tempoDesdeUltimoPulso > 2000000UL || primeiroLocal) {
        velocidadeKmh = 0;
        rpm           = 0;

        noInterrupts();
        intervaloPulsos = 0;
        primeiroPulso   = true;
        interrupts();

    } else if (intervalo > 0) {
        unsigned long intervaloEfetivo = max(intervalo, tempoDesdeUltimoPulso);

        float pulsosPorSegundo = 1000000.0f / (float)intervaloEfetivo;
        rpm           = pulsosPorSegundo * 60.0f;
        velocidadeKmh = (circunferenciaRoda * pulsosPorSegundo) * 3.6f;
    }
    if (millis() - tempoUltimoEnvio >= intervaloEnvio) {
        tempoUltimoEnvio = millis();
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("Wi-Fi desconectado. Reconectando...");
            WiFi.reconnect();
        }

        if (WiFi.status() == WL_CONNECTED) {
            char payload[64];
            snprintf(payload, sizeof(payload),
                     "velocidade=%.2f&rpm=%.2f", velocidadeKmh, rpm);

            HTTPClient clienteHttp;
            clienteHttp.begin(urlServidor);
            clienteHttp.setTimeout(3000);
            clienteHttp.addHeader("Content-Type", "application/x-www-form-urlencoded");

            int codigoResposta = clienteHttp.POST(String(payload));

            if (codigoResposta > 0) {
                String resposta = clienteHttp.getString();
                Serial.printf("[HTTP %d] vel=%.2f km/h  rpm=%.2f  resp: %s\n",
                              codigoResposta, velocidadeKmh, rpm, resposta.c_str());
            } else {
                Serial.printf("[ERRO HTTP] código: %d\n", codigoResposta);
            }

            clienteHttp.end();
        }
    }
}
