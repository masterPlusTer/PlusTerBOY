#include <FastLED.h>
#include <avr/pgmspace.h>  // Para usar PROGMEM

// Definición de pines
#define LED_PIN 2       // Pin donde está conectada la tira NeoPixel
#define NUM_LEDS 484    // Número de LEDs en la tira (22x22 = 484)
#define BUZZER_PIN 3    // Pin del buzzer
#define B_BUTTON_PIN 4  // Pin del botón B
#define A_BUTTON_PIN 5  // Pin del botón A
#define UP_PIN 6        // Pin del botón UP
#define DOWN_PIN 7      // Pin del botón DOWN
#define LEFT_PIN 8      // Pin del botón LEFT
#define RIGHT_PIN 9     // Pin del botón RIGHT

// Definir el array de LEDs
CRGB leds[NUM_LEDS];

// Definir la cuadrícula de 22x22 en PROGMEM
const uint8_t bitmap[22][22] PROGMEM = {
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 1, 1, 0, 2, 0, 0, 3, 0, 3, 0, 4, 4, 5, 5, 5, 6, 6, 6, 7, 7, 0},
  {0, 1, 0, 1, 2, 0, 0, 3, 0, 3, 4, 0, 0, 0, 5, 0, 6, 0, 0, 7, 0, 7},
  {0, 1, 0, 1, 2, 0, 0, 3, 0, 3, 4, 4, 4, 0, 5, 0, 6, 0, 0, 7, 0, 7},
  {0, 1, 1, 0, 2, 0, 0, 3, 0, 3, 0, 0, 4, 0, 5, 0, 6, 6, 0, 7, 7, 0},
  {0, 1, 0, 0, 2, 0, 0, 3, 0, 3, 0, 0, 4, 0, 5, 0, 6, 0, 0, 7, 0, 7},
  {0, 1, 0, 0, 2, 2, 2, 3, 3, 3, 4, 4, 0, 0, 5, 0, 6, 6, 6, 7, 0, 7},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {5, 5, 5, 5, 5, 5, 0, 0, 0, 0, 2, 2, 2, 0, 3, 3, 0, 0, 0, 0, 3, 3},
  {5, 5, 5, 5, 5, 5, 5, 5, 0, 0, 2, 2, 2, 0, 3, 3, 0, 0, 0, 0, 3, 3},
  {5, 5, 0, 0, 0, 0, 5, 5, 0, 2, 2, 0, 2, 2, 0, 3, 3, 0, 0, 3, 3, 0},
  {5, 5, 0, 0, 0, 0, 5, 5, 0, 2, 2, 0, 2, 2, 0, 3, 3, 0, 0, 3, 3, 0},
  {5, 5, 0, 0, 0, 0, 5, 5, 0, 2, 2, 0, 2, 2, 0, 0, 3, 3, 3, 3, 0, 0},
  {5, 5, 5, 5, 5, 5, 5, 0, 0, 2, 2, 0, 2, 2, 0, 0, 3, 3, 3, 3, 0, 0},
  {5, 5, 5, 5, 5, 5, 5, 0, 0, 2, 2, 0, 2, 2, 0, 0, 0, 3, 3, 0, 0, 0},
  {5, 5, 0, 0, 0, 0, 5, 5, 0, 2, 2, 0, 2, 2, 0, 0, 0, 3, 3, 0, 0, 0},
  {5, 5, 0, 0, 0, 0, 5, 5, 0, 2, 2, 0, 2, 2, 0, 0, 0, 3, 3, 0, 0, 0},
  {5, 5, 0, 0, 0, 0, 5, 5, 0, 2, 2, 0, 2, 2, 0, 0, 0, 3, 3, 0, 0, 0},
  {5, 5, 5, 5, 5, 5, 5, 0, 0, 0, 2, 2, 2, 0, 0, 0, 0, 3, 3, 0, 0, 0},
  {5, 5, 5, 5, 5, 5, 0, 0, 0, 0, 2, 2, 2, 0, 0, 0, 0, 3, 3, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

// Variables para el "pixel" actual
int pixelX = 0;  // Posición X del pixel
int pixelY = 0;  // Posición Y del pixel
uint8_t pixelColorIndex = 0;  // Índice del color actual (0, 1, 2, 3)
CRGB pixelColors[5] = {CRGB::White, CRGB::Black, CRGB::Red, CRGB::Green, CRGB::Blue};  // Colores disponibles

// Variable para controlar el brillo
uint8_t brightness = 25;  // Brillo inicial

// Variables para el debounce de los botones
unsigned long lastDebounceTime = 0;
#define DEBOUNCE_DELAY 50  // Tiempo de debounce en milisegundos

// Variable para el estado del botón B (toggle)
bool bButtonActive = false;  // Estado inicial: desactivado

void setup() {
  // Inicializar FastLED
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(brightness);  // Establecer el brillo inicial
  FastLED.clear();  // Apagar todos los LEDs
  FastLED.show();   // Enviar los datos a la tira

  // Configurar pines de entrada (botones)
  pinMode(B_BUTTON_PIN, INPUT_PULLUP);
  pinMode(A_BUTTON_PIN, INPUT_PULLUP);
  pinMode(UP_PIN, INPUT_PULLUP);
  pinMode(DOWN_PIN, INPUT_PULLUP);
  pinMode(LEFT_PIN, INPUT_PULLUP);
  pinMode(RIGHT_PIN, INPUT_PULLUP);

  // Configurar pin de salida (buzzer)
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);  // Asegurar que el buzzer esté apagado



  // Dibujar el bitmap en la matriz LED
  drawBitmap();
      // Reproducir melodía de inicio
  playStartMelody();
}

void loop() {
  // Leer el estado de los botones
  bool bButtonState = digitalRead(B_BUTTON_PIN);
  bool aButtonState = digitalRead(A_BUTTON_PIN);
  bool upState = digitalRead(UP_PIN);
  bool downState = digitalRead(DOWN_PIN);
  bool leftState = digitalRead(LEFT_PIN);
  bool rightState = digitalRead(RIGHT_PIN);

  // Verificar si algún botón está presionado (con debounce)
  if (millis() - lastDebounceTime > DEBOUNCE_DELAY) {
    if (bButtonState == LOW) {  // Botón B presionado
  FastLED.clear();  // Apagar todos los LEDs
  FastLED.show();   // Enviar los datos a la tira
      playClick();              // Emitir un "click"
      lastDebounceTime = millis();
    } else if (aButtonState == LOW) {  // Botón A presionado
      playClick();                     // Emitir un "click"
      bButtonActive = false;           // Desactivar el botón B
      changePixelColor();              // Cambiar el color del pixel
      lastDebounceTime = millis();
    } else if (upState == LOW) {  // Botón UP presionado
      playClick();                // Emitir un "click"
      movePixel(0, -1);           // Mover el pixel hacia arriba
      lastDebounceTime = millis();
    } else if (downState == LOW) {  // Botón DOWN presionado
      playClick();                  // Emitir un "click"
      movePixel(0, 1);              // Mover el pixel hacia abajo
      lastDebounceTime = millis();
    } else if (leftState == LOW) {  // Botón LEFT presionado
      playClick();                  // Emitir un "click"
      movePixel(-1, 0);             // Mover el pixel hacia la izquierda
      lastDebounceTime = millis();
    } else if (rightState == LOW) {  // Botón RIGHT presionado
      playClick();                   // Emitir un "click"
      movePixel(1, 0);               // Mover el pixel hacia la derecha
      lastDebounceTime = millis();
    }
  }

  // Dibujar el pixel en la matriz LED
  drawPixel();
}

// Función para mapear coordenadas (x, y) a la posición en la tira de LEDs
int getPixelIndex(int x, int y) {
  if (y % 2 != 0) {
    // Filas pares: invertir la dirección
    return (y * 22) + (21 - x);
  } else {
    // Filas impares: dirección normal
    return (y * 22) + x;
  }
}

// Función para dibujar el bitmap en la matriz LED
void drawBitmap() {
  for (int y = 0; y < 22; y++) {
    for (int x = 0; x < 22; x++) {
      int pixelIndex = getPixelIndex(x, y);  // Obtener el índice del LED
      uint8_t value = pgm_read_byte(&bitmap[y][x]);  // Leer desde PROGMEM
      if (value == 1) {
        leds[pixelIndex] = CRGB::Red;  // Encender el LED en rojo
      } else if (value == 2) {
        leds[pixelIndex] = CRGB::Green;  // Encender el LED en verde
      } else if (value == 3) {
        leds[pixelIndex] = CRGB::Blue;  // Encender el LED en azul
     
          } else if (value == 4) {
        leds[pixelIndex] = CRGB::Pink;  // Encender el LED en azul
     
          } else if (value == 5) {
        leds[pixelIndex] = CRGB::Purple;  // Encender el LED en azul
      
          } else if (value == 6) {
        leds[pixelIndex] = CRGB::White;  // Encender el LED en azul
      
          } else if (value == 7) {
        leds[pixelIndex] = CRGB::Yellow;  // Encender el LED en azul
      } else {
        leds[pixelIndex] = CRGB::Black;  // Apagar el LED
      }
    }
  }
  FastLED.show();  // Enviar los datos a la tira
}

// Función para dibujar el pixel en la matriz LED
void drawPixel() {
  int pixelIndex = getPixelIndex(pixelX, pixelY);  // Obtener el índice del LED
  leds[pixelIndex] = pixelColors[pixelColorIndex]; // Encender el LED con el color actual
  FastLED.show();                                 // Enviar los datos a la tira
}

// Función para mover el pixel
void movePixel(int dx, int dy) {
  // Pintar el píxel actual antes de moverlo
  int currentPixelIndex = getPixelIndex(pixelX, pixelY);
  leds[currentPixelIndex] = pixelColors[pixelColorIndex]; // Pintar el LED con el color actual
  FastLED.show();                                         // Enviar los datos a la tira

  // Calcular la nueva posición del pixel
  int newX = pixelX + dx;
  int newY = pixelY + dy;

  // Verificar que la nueva posición esté dentro de los límites de la matriz
  if (newX >= 0 && newX < 22 && newY >= 0 && newY < 22) {
    // Actualizar la posición del pixel
    pixelX = newX;
    pixelY = newY;


  }
}

// Función para cambiar el color del pixel
void changePixelColor() {
  pixelColorIndex = (pixelColorIndex + 1) % 4;  // Cambiar al siguiente color (0, 1, 2, 3)
}

// Función para pintar el pixel en la posición actual
void paintPixel() {
  int pixelIndex = getPixelIndex(pixelX, pixelY);  // Obtener el índice del LED
  leds[pixelIndex] = pixelColors[pixelColorIndex]; // Pintar el LED con el color actual
  FastLED.show();                                 // Enviar los datos a la tira
}

// Función para emitir un "click" con el buzzer
void playClick() {
  tone(BUZZER_PIN, 100, 20);  // Tono más grave y corto (1000 Hz, 20 ms)
  delay(20);                   // Esperar mientras suena el tono
  noTone(BUZZER_PIN);          // Apagar el buzzer
}


// Función para reproducir una melodía de inicio
void playStartMelody() {
  // Frecuencias de las notas (en Hz)
  int melody[] = {262, 330, 392, 523};  // Do, Mi, Sol, Do (octava alta)
  // Duración de cada nota (en milisegundos)
  int noteDurations[] = {200, 200, 200, 400};

  // Reproducir la melodía
  for (int i = 0; i < 4; i++) {
    tone(BUZZER_PIN, melody[i], noteDurations[i]);
    delay(noteDurations[i] + 50);  // Pequeña pausa entre notas
    noTone(BUZZER_PIN);
  }
}
