# 🐍 Snake Game — Arduino + OLED + Buzzer

Juego Snake clásico para Arduino con pantalla OLED SSD1306 y efectos de sonido en buzzer pasivo.

## 🎮 Características

- Pantalla OLED 128×64 (SSD1306, I2C)
- 4 botones direccionales
- Buzzer pasivo con:
  - Melodía en el menú (estilo Nokia, en bucle no bloqueante)
  - Click suave en cada movimiento
  - Chirp ascendente al comer la fruta
  - Melodía de game over
- Score en pantalla
- Generación aleatoria de frutas
- Detección de colisión con paredes y cuerpo

## 🧰 Hardware necesario

| Componente | Cantidad |
|---|---|
| Arduino Uno / Nano | 1 |
| Pantalla OLED SSD1306 128×64 (I2C) | 1 |
| Buzzer pasivo | 1 |
| Pulsadores / botones | 4 |
| Resistencias 10kΩ (pull-down, opcional) | 4 |
| Cables y protoboard | — |

## 🔌 Conexiones

### OLED SSD1306 (I2C)
| OLED | Arduino |
|---|---|
| VCC | 3.3V o 5V |
| GND | GND |
| SDA | A4 (Uno) / A4 (Nano) |
| SCL | A5 (Uno) / A5 (Nano) |

> Dirección I2C: `0x3D` (cambiar a `0x3C` si la pantalla no responde)

### Botones
| Función | Pin Arduino |
|---|---|
| LEFT | 4 |
| UP | 2 |
| RIGHT | 5 |
| DOWN | 3 |

Cada botón conectado entre el pin y GND (usa `INPUT_PULLUP`).

### Buzzer pasivo
| Buzzer | Arduino |
|---|---|
| + (positivo) | Pin 8 |
| − (negativo) | GND |

⚠️ Usar buzzer **pasivo**, no activo. La función `tone()` requiere un buzzer pasivo para generar distintas frecuencias.

## 📚 Librerías requeridas

Instalar desde el Library Manager del IDE de Arduino:

- **Adafruit GFX Library** — `Adafruit Industries`
- **Adafruit SSD1306** — `Adafruit Industries`

## 🚀 Cómo cargar

1. Clonar o descargar este repositorio
2. Abrir `snake_buzzer.ino` en el IDE de Arduino
3. Instalar las librerías indicadas
4. Seleccionar tu placa y puerto en **Herramientas**
5. Cargar con el botón Upload (→)

## 🎵 Sonidos

| Momento | Sonido |
|---|---|
| Menú / inicio | Melodía estilo Nokia en bucle |
| Movimiento | Click corto (se puede silenciar comentando `soundMove()`) |
| Comer fruta | Chirp do-mi-sol |
| Game over | Melodía descendente |

## ⚙️ Configuración

En el archivo `.ino` puedes ajustar estas constantes:

```cpp
#define SNAKE_PIECE_SIZE     3   // Tamaño de cada bloque en px
#define MAX_SANKE_LENGTH   165   // Longitud máxima de la serpiente
#define MAP_SIZE_X          20   // Ancho del mapa en celdas
#define MAP_SIZE_Y          20   // Alto del mapa en celdas
#define STARTING_SNAKE_SIZE  5   // Tamaño inicial
#define SNAKE_MOVE_DELAY    30   // Velocidad (menor = más rápido)
#define BUZZER_PIN           8   // Pin del buzzer
```

## 📄 Licencia

MIT — libre para usar, modificar y compartir.
