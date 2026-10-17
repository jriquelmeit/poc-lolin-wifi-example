#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "credentials.h"

/**
 * PoC Wi-Fi y Diagnóstico para Lolin NodeMCU V3 (ESP8266)
 *
 * LED Integrado (GPIO2 / D4 / LED_BUILTIN):
 *   - Lógica activa en BAJO (Active-Low):
 *       LOW  = Encendido
 *       HIGH = Apagado
 *
 * Patrones de LED:
 *   - Conectando: Parpadeo rápido (100 ms ON / 100 ms OFF)
 *   - Conectado:  Parpadeo suave / latido (100 ms ON / 900 ms OFF)
 */

// Lógica de encendido/apagado para active-low
constexpr uint8_t LED_STATE_ON = LOW;
constexpr uint8_t LED_STATE_OFF = HIGH;

// Tiempos de parpadeo (ms)
constexpr unsigned long BLINK_CONNECTING_ON = 100;
constexpr unsigned long BLINK_CONNECTING_OFF = 100;

constexpr unsigned long BLINK_CONNECTED_ON = 100;
constexpr unsigned long BLINK_CONNECTED_OFF = 900;

// Variables de estado del LED
unsigned long lastLedToggle = 0;
bool ledState = false;

// Variables de estado de conexión
bool wasConnected = false;
unsigned long connectionStartTime = 0;
unsigned long lastStatusPrint = 0;
constexpr unsigned long STATUS_PRINT_INTERVAL = 30000; // Cada 30 s reporte de estado

void setLed(bool turnOn) {
    digitalWrite(LED_BUILTIN, turnOn ? LED_STATE_ON : LED_STATE_OFF);
    ledState = turnOn;
}

void printWifiDetails() {
    Serial.println(F("\n========================================"));
    Serial.println(F("         CONEXIÓN WI-FI EXITOSA         "));
    Serial.println(F("========================================"));
    Serial.printf("SSID:            %s\n", WiFi.SSID().c_str());
    Serial.printf("IP Asignada:     %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("Máscara de red:  %s\n", WiFi.subnetMask().toString().c_str());
    Serial.printf("Puerta de enlace:%s\n", WiFi.gatewayIP().toString().c_str());
    Serial.printf("DNS Primario:    %s\n", WiFi.dnsIP().toString().c_str());
    Serial.printf("Dirección MAC:   %s\n", WiFi.macAddress().c_str());
    Serial.printf("Potencia (RSSI): %d dBm\n", WiFi.RSSI());
    Serial.printf("Canal Wi-Fi:     %d\n", WiFi.channel());
    Serial.printf("Tiempo conex.:   %lu ms\n", millis() - connectionStartTime);
    Serial.println(F("========================================\n"));
    Serial.println(F("Patrón LED: Latido suave (Conectado OK)\n"));
}

void printChipInfo() {
    Serial.println(F("\n--- Lolin NodeMCU V3 (ESP8266) PoC Wi-Fi ---"));
    Serial.printf("Chip ID:         0x%08X\n", ESP.getChipId());
    Serial.printf("Frecuencia CPU:  %d MHz\n", ESP.getCpuFreqMHz());
    Serial.printf("Versión SDK:     %s\n", ESP.getSdkVersion());
    Serial.printf("Flash Size:      %u bytes\n", ESP.getFlashChipRealSize());
    Serial.printf("Memoria Libre:   %u bytes\n", ESP.getFreeHeap());
    Serial.printf("Dirección MAC:   %s\n", WiFi.macAddress().c_str());
    Serial.println(F("--------------------------------------------"));
}

void startWifiConnection() {
    Serial.println(F("\nIniciando conexión Wi-Fi..."));
    Serial.printf("Conectando a SSID: \"%s\"\n", WIFI_SSID);

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    connectionStartTime = millis();
    Serial.print(F("Esperando enlace"));
}

void updateLedPattern(bool isConnected) {
    unsigned long currentMillis = millis();
    unsigned long interval = 0;

    if (isConnected) {
        interval = ledState ? BLINK_CONNECTED_ON : BLINK_CONNECTED_OFF;
    } else {
        interval = ledState ? BLINK_CONNECTING_ON : BLINK_CONNECTING_OFF;
    }

    if (currentMillis - lastLedToggle >= interval) {
        lastLedToggle = currentMillis;
        setLed(!ledState);
    }
}

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    setLed(false); // Apagado inicialmente

    Serial.begin(115200);
    // Pequeño margen para estabilizar la línea serie tras reset
    delay(200);

    printChipInfo();
    startWifiConnection();
}

void loop() {
    bool isConnected = (WiFi.status() == WL_CONNECTED);

    // Detección de transiciones de estado
    if (isConnected && !wasConnected) {
        wasConnected = true;
        printWifiDetails();
    } else if (!isConnected && wasConnected) {
        wasConnected = false;
        Serial.println(F("\n[ALERTA] ¡Conexión Wi-Fi perdida!"));
        Serial.println(F("Reintentando conexión automáticamente..."));
        Serial.print(F("Patrón LED: Parpadeo rápido (Reconectando)"));
        connectionStartTime = millis();
    }

    // Si aún no está conectado, mostrar progreso en consola cada 500 ms
    if (!isConnected) {
        static unsigned long lastDot = 0;
        if (millis() - lastDot >= 500) {
            lastDot = millis();
            Serial.print(F("."));
        }
    } else {
        // Si está conectado, emitir diagnóstico periódico cada 30 segundos
        if (millis() - lastStatusPrint >= STATUS_PRINT_INTERVAL) {
            lastStatusPrint = millis();
            Serial.printf("[Heartbeat] Wi-Fi OK | IP: %s | RSSI: %d dBm | RAM libre: %u bytes\n",
                          WiFi.localIP().toString().c_str(),
                          WiFi.RSSI(),
                          ESP.getFreeHeap());
        }
    }

    // Actualizar parpadeo del LED sin bloquear la ejecución
    updateLedPattern(isConnected);
}
