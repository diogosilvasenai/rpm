/*
 * ============================================================
 *  Velocímetro IoT - ESP32
 *  Componente: Sistemas Embarcados / IoT
 * ============================================================
 *  Correções aplicadas em relação ao código base:
 *   1. Timeout de 15s no setup() para não travar sem Wi-Fi
 *   2. clienteHttp.setTimeout(3000) evita bloqueio longo no POST
 *   3. Removido delay(100) desnecessário no loop()
 *   4. snprintf substitui concatenação de String (evita heap frag.)
 *   5. Reconexão automática Wi-Fi no loop()
 *   6. Velocidade corrigida na desaceleração (usa tempoAtual se maior)
 *   7. Credenciais em #define separado (fácil de trocar)
 * ============================================================
 */

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>

// ---------- Configurações de rede ----------
#define NOME_REDE   "Alexandre"
#define SENHA_REDE  "xawc8090"
const String urlServidor = "http://172.19.243.39/projeto_sesi/salvar_dados.php";

// ---------- Hardware ----------
#define PINO_SENSOR_HALL 4

// ---------- Variáveis do sensor (acessadas pela ISR) ----------
volatile unsigned long tempoUltimoPulso  = 0;
volatile unsigned long intervaloPulsos   = 0;
volatile bool          primeiroPulso     = true;

// ---------- Parâmetros da roda ----------
const float diametroRoda = 0.60;          // metros
float circunferenciaRoda;

// ---------- Resultados calculados ----------
float velocidadeKmh = 0;
float rpm           = 0;

// ---------- Controle de envio ----------
unsigned long tempoUltimoEnvio          = 0;
const unsigned long intervaloEnvio      = 2000; // ms entre envios HTTP

// ============================================================
//  ISR - Detecta a passagem do ímã pelo sensor Hall
//  Roda em IRAM para não ser afetada por cache miss
// ============================================================
void IRAM_ATTR detectarIma() {
    unsigned long tempoAtual = micros();

    // Debounce por software: ignora pulsos com menos de 5 ms de diferença
    if (tempoAtual - tempoUltimoPulso > 5000) {
        if (!primeiroPulso) {
            // Salva o intervalo entre este e o pulso anterior
            intervaloPulsos = tempoAtual - tempoUltimoPulso;
        } else {
            // Primeiro pulso: só marca o tempo, não calcula ainda
            primeiroPulso = false;
        }
        tempoUltimoPulso = tempoAtual;
    }
}

// ============================================================
//  SETUP
// ============================================================
void setup() {
    Serial.begin(115200);
    Serial.println("\n--- Velocímetro IoT ESP32 ---");

    // Conecta ao Wi-Fi com timeout de 15 segundos
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
        // Continua sem Wi-Fi: sensor ainda funciona, envio será pulado
        Serial.println("\nAVISO: Wi-Fi não disponível. Operando offline.");
    }

    // Configura o pino do sensor
    pinMode(PINO_SENSOR_HALL, INPUT_PULLUP);

    // Pré-calcula a circunferência da roda
    circunferenciaRoda = diametroRoda * 3.141592;

    // Registra a interrupção
    attachInterrupt(digitalPinToInterrupt(PINO_SENSOR_HALL), detectarIma, FALLING);

    Serial.println("Sistema pronto.");
}

// ============================================================
//  LOOP
// ============================================================
void loop() {

    // ----- 1. Lê variáveis voláteis de forma segura -----
    noInterrupts();
    unsigned long intervalo              = intervaloPulsos;
    unsigned long tempoUltimoPulsoLocal  = tempoUltimoPulso;
    bool          primeiroLocal          = primeiroPulso;
    interrupts();

    unsigned long tempoDesdeUltimoPulso = micros() - tempoUltimoPulsoLocal;

    // ----- 2. Calcula velocidade e RPM -----
    if (tempoDesdeUltimoPulso > 2000000UL || primeiroLocal) {
        // Sem pulsos há mais de 2 segundos → velocidade zero
        velocidadeKmh = 0;
        rpm           = 0;

        noInterrupts();
        intervaloPulsos = 0;
        primeiroPulso   = true;
        interrupts();

    } else if (intervalo > 0) {
        // CORREÇÃO: se o ESP32 está "esperando" mais tempo do que o
        // último intervalo registrado, usa o tempo atual (desaceleração)
        unsigned long intervaloEfetivo = max(intervalo, tempoDesdeUltimoPulso);

        float pulsosPorSegundo = 1000000.0f / (float)intervaloEfetivo;
        rpm           = pulsosPorSegundo * 60.0f;
        velocidadeKmh = (circunferenciaRoda * pulsosPorSegundo) * 3.6f;
    }

    // ----- 3. Envia dados via HTTP a cada intervaloEnvio -----
    if (millis() - tempoUltimoEnvio >= intervaloEnvio) {
        tempoUltimoEnvio = millis(); // Atualiza ANTES do POST para não acumular

        // Tenta reconectar se perdeu o Wi-Fi
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("Wi-Fi desconectado. Reconectando...");
            WiFi.reconnect();
        }

        if (WiFi.status() == WL_CONNECTED) {
            // Monta o payload com buffer estático (sem alocação dinâmica)
            char payload[64];
            snprintf(payload, sizeof(payload),
                     "velocidade=%.2f&rpm=%.2f", velocidadeKmh, rpm);

            HTTPClient clienteHttp;
            clienteHttp.begin(urlServidor);
            clienteHttp.setTimeout(3000); // CORREÇÃO: timeout de 3s no POST
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

    // Sem delay() aqui — controle de tempo feito 100% via millis()
}
