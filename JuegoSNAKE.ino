#include <Adafruit_NeoPixel.h>

// Definir los pines
uint8_t buttons[] = {8, 9, 6, 7, 4, 5};
uint8_t buzzerPin = 3;
const int NEOPIXEL_PIN = 2;

// Definir el número de LEDs en la matriz
const int NUM_LEDS_X = 22;
const int NUM_LEDS_Y = 22;
const int NUM_LEDS = NUM_LEDS_X * NUM_LEDS_Y;

// Crear una instancia de la matriz de NeoPixels
Adafruit_NeoPixel pixels(NUM_LEDS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// Definir la frecuencia del buzzer (en Hz)
uint16_t buzzerFrequency = 2000;

// Almacenar el estado anterior de los botones
bool prevButtonState[6];
unsigned long lastButtonCheckTime = 0;
const unsigned long buttonCheckInterval = 10; // Intervalo de verificación de los botones en milisegundos

int snakeX = 15, snakeY = 15;
int foodX = random(NUM_LEDS_X), foodY = random(NUM_LEDS_Y);
bool gameOver = false;

void setup() {
  Serial.begin(9600);

  // Configurar el pin del buzzer como salida
  pinMode(buzzerPin, OUTPUT);

  // Configurar los pines de los pulsadores como entrada con resistencia de pull-up
  for (uint8_t i = 0; i < 6; i++) {
    pinMode(buttons[i], INPUT_PULLUP);
    prevButtonState[i] = true;
  }

  // Inicializar la matriz de NeoPixels
  pixels.begin();
  pixels.setBrightness(5); // Reducir el brillo de los LEDs
}

void loop() {
    // Leer el estado de los pulsadores
    for (uint8_t i = 0; i < 6; i++) {
        bool buttonState = digitalRead(buttons[i]);

        // Verificar si algún botón ha sido presionado
        if (!buttonState && prevButtonState[i]) {
            // Activar el buzzer a máxima amplitud y a la frecuencia definida
            tone(buzzerPin, buzzerFrequency, 100);


            //cada que se presiona un boton imprime los valores en consola 
            Serial.println("");
Serial.print("foodX = ");
Serial.println(foodX);
Serial.print("foodY = ");
Serial.println(foodY);
Serial.println(" ");
Serial.print("snakeX = ");
Serial.println(snakeX);
Serial.print("snakeY = ");
Serial.println(snakeY);
Serial.println("");

            // Mover la serpiente según el botón presionado
            switch (i) {
                case 0:
                    snakeX--;
                    break;
                case 1:
                    snakeX++;
                    break;
                case 2:
                    snakeY--;
                    break;
                case 3:
                    snakeY++;
                    break;
            }

            // Verificar si la serpiente ha chocado contra los bordes
            if (snakeX < 0 || snakeX >= NUM_LEDS_X || snakeY < 0 || snakeY >= NUM_LEDS_Y) {
                gameOver = true;
            }

            // Dibujar la serpiente y la comida
            pixels.clear();
            pixels.setPixelColor(foodX + foodY * NUM_LEDS_X, pixels.Color(255, 0, 255)); // Comida

            // Dibujar la serpiente con filas invertadas de manera alternativa
            for (int y = 0; y < NUM_LEDS_Y; y++) {
                for (int x = 0; x < NUM_LEDS_X; x++) {
                    int ledIndex = (y % 2 == 0) ? x + y * NUM_LEDS_X : (NUM_LEDS_X - 1) - x + y * NUM_LEDS_X;
                    if (x == snakeX && y == snakeY) {
                        pixels.setPixelColor(ledIndex, pixels.Color(255, 0, 0)); // Serpiente
                    }
                }
            }

            pixels.show();

            // Actualizar el estado anterior de los botones
            prevButtonState[i] = buttonState;

            // Verificar si la serpiente ha comido la comida
            if (snakeX == foodX && snakeY == foodY) {
                // Generar una nueva posición para la comida
                foodX = random(NUM_LEDS_X);
                foodY = random(NUM_LEDS_Y);
            }

            // Verificar si el juego ha terminado
            if (gameOver) {
                // Parpadear los LEDs en rojo para indicar el fin del juego
                for (int i = 0; i < 5; i++) {
                    pixels.clear();
                    pixels.setPixelColor((NUM_LEDS_X - 1) - snakeX + snakeY * NUM_LEDS_X, pixels.Color(255, 0, 0));
                    pixels.show();
                    delay(250);
                    pixels.clear();
                    pixels.show();
                    delay(250);
                }
                gameOver = false;
                snakeX = 15;
                snakeY = 15;
                foodX = random(NUM_LEDS_X);
                foodY = random(NUM_LEDS_Y);
            }
        }
        prevButtonState[i] = buttonState;
    }

    // Desactivar el buzzer y apagar la matriz de NeoPixels
    noTone(buzzerPin);
    pixels.show();
}
