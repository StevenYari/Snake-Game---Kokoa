#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH        128
#define SCREEN_HEIGHT        64
#define OLED_RESET            4
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ── Buzzer ───────────────────────────────────────────────
#define BUZZER_PIN 8

// Notas (frecuencias en Hz)
#define NOTE_C4  262
#define NOTE_D4  294
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_G4  392
#define NOTE_A4  440
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_D5  587
#define NOTE_E5  659
#define NOTE_G5  784
#define NOTE_A5  880
#define NOTE_REST  0

// ── Melodía del menú (estilo Snake clásico Nokia) ────────
const int menuMelody[] = {
  NOTE_E5, NOTE_E5, NOTE_REST, NOTE_E5, NOTE_REST,
  NOTE_C5, NOTE_E5, NOTE_REST, NOTE_G5, NOTE_REST,
  NOTE_G4, NOTE_REST
};
const int menuDurations[] = {
  100, 100, 50, 100, 50,
  100, 100, 50, 200, 100,
  200, 150
};
const int menuLength = 12;

// ── Melodía de game over ─────────────────────────────────
const int gameoverMelody[] = {
  NOTE_G5, NOTE_E5, NOTE_C5, NOTE_G4,
  NOTE_A4, NOTE_REST, NOTE_A4, NOTE_G4
};
const int gameoverDurations[] = {
  150, 150, 150, 300,
  150, 50, 150, 400
};
const int gameoverLength = 8;

// ── Estado de música no bloqueante ───────────────────────
int       melodyNote     = 0;
bool      melodyPlaying  = false;
bool      melodyLoop     = false;
unsigned long noteStart  = 0;
int       noteDuration   = 0;
const int* melodyPtr     = nullptr;
const int* durPtr        = nullptr;
int       melodyLen      = 0;

void startMelody(const int* mel, const int* dur, int len, bool loop = false) {
  melodyPtr    = mel;
  durPtr       = dur;
  melodyLen    = len;
  melodyNote   = 0;
  melodyLoop   = loop;
  melodyPlaying = true;
  noteStart    = millis();
  noteDuration = dur[0];
  if (mel[0] != NOTE_REST) tone(BUZZER_PIN, mel[0], dur[0]);
}

void stopMelody() {
  melodyPlaying = false;
  noTone(BUZZER_PIN);
}

void updateMelody() {
  if (!melodyPlaying) return;
  if (millis() - noteStart >= (unsigned long)noteDuration) {
    melodyNote++;
    if (melodyNote >= melodyLen) {
      if (melodyLoop) melodyNote = 0;
      else { melodyPlaying = false; noTone(BUZZER_PIN); return; }
    }
    noteStart    = millis();
    noteDuration = durPtr[melodyNote];
    if (melodyPtr[melodyNote] != NOTE_REST)
      tone(BUZZER_PIN, melodyPtr[melodyNote], noteDuration);
    else
      noTone(BUZZER_PIN);
  }
}

// ── Efectos de sonido puntuales ──────────────────────────
void soundEat() {
  // Chirp ascendente al comer
  tone(BUZZER_PIN, NOTE_C5, 40);
  delay(40);
  tone(BUZZER_PIN, NOTE_E5, 40);
  delay(40);
  tone(BUZZER_PIN, NOTE_G5, 60);
  delay(60);
  noTone(BUZZER_PIN);
}

void soundMove() {
  // Click suave en cada movimiento (opcional, puedes comentarlo)
  tone(BUZZER_PIN, NOTE_A4, 12);
  delay(12);
  noTone(BUZZER_PIN);
}

// ────────────────────────────────────────────────────────
const byte buttonPins[] = {4, 2, 5, 3}; // LEFT, UP, RIGHT, DOWN

typedef enum { START, RUNNING, GAMEOVER } State;
typedef enum { LEFT, UP, RIGHT, DOWN } Direction;

#define SNAKE_PIECE_SIZE     3
#define MAX_SANKE_LENGTH   165
#define MAP_SIZE_X          20
#define MAP_SIZE_Y          20
#define STARTING_SNAKE_SIZE  5
#define SNAKE_MOVE_DELAY    30

State    gameState;
int8_t   snake[MAX_SANKE_LENGTH][2];
uint8_t  snake_length;
Direction dir;
Direction newDir;
int8_t   fruit[2];

// ── Setup ────────────────────────────────────────────────
void setup() {
  Serial.begin(9600);
  pinMode(BUZZER_PIN, OUTPUT);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  for (byte i = 0; i < 4; i++)
    pinMode(buttonPins[i], INPUT_PULLUP);

  randomSeed(analogRead(A0));
  setupGame();
}

void setupGame() {
  gameState = START;
  dir       = RIGHT;
  newDir    = RIGHT;
  resetSnake();
  generateFruit();
  display.clearDisplay();
  drawMap();
  drawScore();
  drawPressToStart();
  display.display();

  // Melodía de menú en bucle
  startMelody(menuMelody, menuDurations, menuLength, true);
}

void resetSnake() {
  snake_length = STARTING_SNAKE_SIZE;
  for (int i = 0; i < snake_length; i++) {
    snake[i][0] = MAP_SIZE_X / 2 - i;
    snake[i][1] = MAP_SIZE_Y / 2;
  }
}

// ── Loop ─────────────────────────────────────────────────
int moveTime = 0;

void loop() {
  updateMelody(); // mantiene la música sin bloquear

  switch (gameState) {

    case START:
      if (buttonPress()) {
        stopMelody();
        gameState = RUNNING;
      }
      break;

    case RUNNING:
      moveTime++;
      readDirection();
      if (moveTime >= SNAKE_MOVE_DELAY) {
        dir = newDir;
        soundMove(); // click de movimiento

        display.clearDisplay();
        if (moveSnake()) {
          gameState = GAMEOVER;
          drawGameover();
          display.display();
          // Melodía de game over (una sola vez)
          playBlockingMelody(gameoverMelody, gameoverDurations, gameoverLength);
        } else {
          drawMap();
          drawScore();
          display.display();
          checkFruit();
        }
        moveTime = 0;
      }
      break;

    case GAMEOVER:
      if (buttonPress()) {
        delay(500);
        setupGame();
        gameState = START;
      }
      break;
  }

  delay(10);
}

// Melodía bloqueante solo para game over (corta y puntual)
void playBlockingMelody(const int* mel, const int* dur, int len) {
  for (int i = 0; i < len; i++) {
    if (mel[i] != NOTE_REST)
      tone(BUZZER_PIN, mel[i], dur[i]);
    else
      noTone(BUZZER_PIN);
    delay(dur[i] + 10);
  }
  noTone(BUZZER_PIN);
}

// ── Controles ────────────────────────────────────────────
bool buttonPress() {
  for (byte i = 0; i < 4; i++)
    if (digitalRead(buttonPins[i]) == LOW) return true;
  return false;
}

void readDirection() {
  for (byte i = 0; i < 4; i++) {
    if (digitalRead(buttonPins[i]) == LOW && i != ((int)dir + 2) % 4) {
      newDir = (Direction)i;
      return;
    }
  }
}

// ── Lógica del juego ─────────────────────────────────────
bool moveSnake() {
  int8_t x = snake[0][0];
  int8_t y = snake[0][1];

  switch (dir) {
    case LEFT:  x -= 1; break;
    case UP:    y -= 1; break;
    case RIGHT: x += 1; break;
    case DOWN:  y += 1; break;
  }

  if (collisionCheck(x, y)) return true;

  for (int i = snake_length - 1; i > 0; i--) {
    snake[i][0] = snake[i - 1][0];
    snake[i][1] = snake[i - 1][1];
  }
  snake[0][0] = x;
  snake[0][1] = y;
  return false;
}

void checkFruit() {
  if (fruit[0] == snake[0][0] && fruit[1] == snake[0][1]) {
    soundEat(); // ¡chirp al comer!
    if (snake_length + 1 <= MAX_SANKE_LENGTH) snake_length++;
    generateFruit();
  }
}

void generateFruit() {
  bool b;
  do {
    b = false;
    fruit[0] = random(0, MAP_SIZE_X);
    fruit[1] = random(0, MAP_SIZE_Y);
    for (int i = 0; i < snake_length; i++) {
      if (fruit[0] == snake[i][0] && fruit[1] == snake[i][1]) {
        b = true; break;
      }
    }
  } while (b);
}

bool collisionCheck(int8_t x, int8_t y) {
  for (int i = 1; i < snake_length; i++)
    if (x == snake[i][0] && y == snake[i][1]) return true;
  if (x < 0 || y < 0 || x >= MAP_SIZE_X || y >= MAP_SIZE_Y) return true;
  return false;
}

// ── Dibujo ───────────────────────────────────────────────
void drawMap() {
  int offsetMapX = SCREEN_WIDTH - SNAKE_PIECE_SIZE * MAP_SIZE_X - 2;
  int offsetMapY = 2;

  display.drawRect(fruit[0] * SNAKE_PIECE_SIZE + offsetMapX,
                   fruit[1] * SNAKE_PIECE_SIZE + offsetMapY,
                   SNAKE_PIECE_SIZE, SNAKE_PIECE_SIZE, SSD1306_INVERSE);

  display.drawRect(offsetMapX - 2, 0,
                   SNAKE_PIECE_SIZE * MAP_SIZE_X + 4,
                   SNAKE_PIECE_SIZE * MAP_SIZE_Y + 4, SSD1306_WHITE);

  for (int i = 0; i < snake_length; i++)
    display.fillRect(snake[i][0] * SNAKE_PIECE_SIZE + offsetMapX,
                     snake[i][1] * SNAKE_PIECE_SIZE + offsetMapY,
                     SNAKE_PIECE_SIZE, SNAKE_PIECE_SIZE, SSD1306_WHITE);
}

void drawScore() {
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(2, 2);
  display.print(F("Score:"));
  display.println(snake_length - STARTING_SNAKE_SIZE);
}

void drawPressToStart() {
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(2, 20);
  display.print(F("Press a\n button to\n start the\n game!"));
}

void drawGameover() {
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(2, 50);
  display.println(F("GAMEOVER"));
}
