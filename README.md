# Lolin NodeMCU V3 (ESP8266) - Conexión Wi-Fi & Diagnóstico en CLion

[![PlatformIO](https://img.shields.io/badge/PlatformIO-Core_6-orange?logo=platformio)](https://platformio.org/)
[![ESP8266](https://img.shields.io/badge/Board-NodeMCU_V3_(ESP8266)-blue?logo=espressif)](https://www.espressif.com/)
[![Framework](https://img.shields.io/badge/Framework-Arduino-00979D?logo=arduino)](https://www.arduino.cc/)
[![IDE](https://img.shields.io/badge/IDE-CLion-000000?logo=clion)](https://www.jetbrains.com/clion/)

Proyecto de prueba de concepto (**PoC**) para conectar el módulo **Lolin NodeMCU V3** (ESP8266) a una red Wi-Fi utilizando **CLion** y **PlatformIO**. 

Incluye:
* Conexión a red Wi-Fi (modo estación 2.4 GHz).
* Indicación visual mediante el **LED integrado** usando patrones de parpadeo no bloqueantes.
* Diagnóstico completo del chip y de la red por el **puerto serie** (115200 baudios).
* Detección y reconexión automática en caso de pérdida de señal.

---

## Características de la Placa y LED Integrado

* **Microcontrolador**: ESP8266 (módulo ESP-12E / ESP-12F).
* **Frecuencia de reloj**: 80 MHz (hasta 160 MHz).
* **Memoria Flash**: 4 MB.
* **Chip conversor USB a Serie**: Generalmente **WCH CH340G**.
* **LED Integrado**: Conectado a **GPIO2** (`D4` en la serigrafía NodeMCU, definido como `LED_BUILTIN`). Es monocromático (azul).

> [!NOTE]
> En la mayoría de las placas NodeMCU/ESP8266, el LED integrado opera con **lógica invertida (activa en bajo)**:
> * `digitalWrite(LED_BUILTIN, LOW)`: **Enciende** el LED.
> * `digitalWrite(LED_BUILTIN, HIGH)`: **Apaga** el LED.

### Patrones de Estado del LED
Como el LED físico es de un solo color, se utilizan cadencias temporales diferenciadas para representar el estado:

| Estado Wi-Fi | Patrón del LED | Frecuencia |
| :--- | :--- | :--- |
| **Conectando / Reconectando** | Parpadeo rápido | 100 ms ON / 100 ms OFF |
| **Conectado (OK)** | Latido suave (heartbeat) | 100 ms ON / 900 ms OFF |

---

## Requisitos Previos

### Hardware
* Placa de desarrollo **Lolin NodeMCU V3**.
* Red Wi-Fi de **2.4 GHz** (ESP8266 no es compatible con redes de 5 GHz).
* Cable **Micro-USB** con soporte de transferencia de datos.

### Software
* **CLion** (JetBrains).
* **PlatformIO Core (`pio`)**:
  ```bash
  # En macOS con Homebrew:
  brew install platformio
  ```
* **Driver USB CH340**: En macOS suele estar integrado. Si no detecta el puerto, instala el driver CH34x de WCH.

---

## Configuración de Credenciales Wi-Fi

1. Copia la plantilla de credenciales si aún no existe `include/credentials.h`:
   ```bash
   cp include/credentials.h.example include/credentials.h
   ```
2. Edita `include/credentials.h` con el nombre y contraseña de tu red Wi-Fi:
   ```cpp
   #define WIFI_SSID "Tu_Red_WiFi_2.4GHz"
   #define WIFI_PASSWORD "Tu_Password_Seguro"
   ```
*(Este archivo está ignorado en `.gitignore` para no exponer tus credenciales en el repositorio).*

---

## Estructura del Proyecto

```text
poc-lolin-example/
├── CMakeLists.txt              # Configuración CMake para CLion
├── platformio.ini              # Configuración del entorno nodemcuv2
├── include/
│   ├── credentials.h.example   # Plantilla de configuración Wi-Fi
│   └── credentials.h           # Credenciales locales (ignorado por git)
├── src/
│   └── main.cpp                # Conexión Wi-Fi, gestión de LED y consola
├── lib/                        # Librerías locales específicas
└── test/                       # Pruebas unitarias
```

---

## Puesta en Marcha

### 1. Clonar el Repositorio
```bash
git clone https://github.com/jriquelmeit/poc-lolin-example.git
cd poc-lolin-example
```

### 2. Abrir en CLion
1. Inicia **CLion**.
2. Selecciona **File > Open...** y elige la carpeta clonada.
3. CLion detectará y cargará automáticamente la configuración mediante `CMakeLists.txt` y los metadatos de PlatformIO.

---

## Flujo de Trabajo en CLion

En la barra superior de ejecución de CLion encontrarás los siguientes targets listos para usar:

| Target de CLion | Acción |
| :--- | :--- |
| **`PLATFORMIO_BUILD`** | Compila el firmware y genera el binario `.pio/build/nodemcuv2/firmware.bin`. |
| **`PLATFORMIO_UPLOAD`** | Compila y transfiere el binario al NodeMCU conectado por USB. |
| **`PLATFORMIO_MONITOR`** | Abre la terminal serie a 115200 baudios para ver la salida del ESP8266. |
| **`PLATFORMIO_CLEAN`** | Limpia los artefactos generados durante la compilación. |

> [!TIP]
> Para habilitar la interfaz visual completa de PlatformIO en CLion (árbol de tareas, gestor de dependencias), puedes instalar el plugin **PlatformIO for CLion** desde **Settings > Plugins**.

---

## Comandos desde Terminal

También puedes compilar y cargar el firmware directamente desde la línea de comandos:

### Compilar el proyecto
```bash
pio run
```

### Subir el firmware a la placa
Conecta la placa mediante el cable Micro-USB y ejecuta:
```bash
pio run -t upload
```

> [!IMPORTANT]
> Si tienes múltiples puertos serie o necesitas especificar uno manualmente, puedes definirlo en `platformio.ini`:
> ```ini
> upload_port = /dev/cu.usbserial-XXXX
> ```

### Monitor Serie
Para observar la salida de depuración por consola:
```bash
pio device monitor
```
*(Para salir del monitor serie, presiona `Ctrl + ]`)*

---

## Diagnóstico y Solución de Problemas

### 1. El puerto serie no es detectado
* Comprueba que el cable USB transporte datos y no solo energía.
* Lista los dispositivos conectados:
  ```bash
  pio device list
  # O en macOS:
  ls /dev/cu.*
  ```
  Debería figurar un dispositivo como `/dev/cu.usbserial-*` o `/dev/cu.wchusbserial*`.

### 2. Error al flashear (`timed out waiting for packet header`)
* Mantén presionado el botón **FLASH** del módulo NodeMCU.
* Pulsa y suelta una vez el botón **RST**.
* Suelta el botón **FLASH** y vuelve a lanzar el comando de subida.

### 3. Caracteres ilegibles en el monitor serie
* Asegúrate de que la velocidad esté fijada en **115200 baudios**, que es la configurada en `Serial.begin(115200)` y en `platformio.ini` (`monitor_speed = 115200`).
