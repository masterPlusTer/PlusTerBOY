#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>

// Definición de pines
#define LED_PIN 2        // D2 para NeoPixel
#define BUZZER_PIN 3     // D3 para buzzer
#define BUTTON_LEFT 8    // D8 (PB0)
#define BUTTON_RIGHT 9   // D9 (PB1)
#define BUTTON_UP 6      // D6
#define BUTTON_DOWN 7    // D7
#define BUTTON_A 5       // D5
#define BUTTON_B 4       // D4

// Tamaño de la matriz
#define MATRIX_SIZE 22
#define NUM_LEDS (MATRIX_SIZE * MATRIX_SIZE)

// Buffer para los LEDs (formato GRB)
uint8_t ledBuffer[NUM_LEDS * 3];

// Variables del juego
int snakeX = 10, snakeY = 10;
int foodX, foodY;
int snakeLength = 3;
int tailX[100], tailY[100];
enum Direction { STOP, LEFT, RIGHT, UP, DOWN };
Direction dir;
bool gameOver = false;
unsigned long lastUpdate = 0;
const unsigned long updateInterval = 200;

// Prototipos de funciones
void sendByteASM(uint8_t byte);
void updateLEDs();
void setPixel(uint8_t x, uint8_t y, uint8_t r, uint8_t g, uint8_t b);
void clearMatrix();
void drawGame();
void generateFood();
void readInput();
void updateGame();
void playTone(uint16_t freq, uint16_t dur);
void gameOverAnimation();
void showTitle();
void clearAnimation();
void drawLetterS(uint8_t offsetX, uint8_t offsetY, uint8_t r, uint8_t g, uint8_t b);
void drawLetterN(uint8_t offsetX, uint8_t offsetY, uint8_t r, uint8_t g, uint8_t b);
void drawLetterA(uint8_t offsetX, uint8_t offsetY, uint8_t r, uint8_t g, uint8_t b);
void drawLetterK(uint8_t offsetX, uint8_t offsetY, uint8_t r, uint8_t g, uint8_t b);
void drawLetterE(uint8_t offsetX, uint8_t offsetY, uint8_t r, uint8_t g, uint8_t b);

// Función para enviar un byte en ensamblador
void sendByteASM(uint8_t byte) {
  asm volatile (
    "ldi r18, 8      \n"
    "next_bit:       \n"
    "sbi %[port], %[pin] \n"
    "lsl %[byte]     \n"
    "brcs bit_is_one \n"
    "nop             \n"
    "nop             \n"
    "cbi %[port], %[pin] \n"
    "nop             \n"
    "nop             \n"
    "rjmp end_bit    \n"
    "bit_is_one:     \n"
    "nop             \n"
    "nop             \n"
    "nop             \n"
    "nop             \n"
    "cbi %[port], %[pin] \n"
    "end_bit:        \n"
    "dec r18         \n"
    "brne next_bit   \n"
    :
    : [port] "I" (_SFR_IO_ADDR(PORTD)),
      [pin] "I" (LED_PIN),
      [byte] "r" (byte)
    : "r18"
  );
}

void updateLEDs() {
  cli();
  for(uint16_t i = 0; i < NUM_LEDS * 3; i++) {
    sendByteASM(ledBuffer[i]);
  }
  sei();
  _delay_us(50);
}

void setPixel(uint8_t x, uint8_t y, uint8_t r, uint8_t g, uint8_t b) {
  if(x >= MATRIX_SIZE || y >= MATRIX_SIZE) return;
  
  uint16_t index;
  if(y % 2 == 0) {
    index = y * MATRIX_SIZE + x;
  } else {
    index = y * MATRIX_SIZE + (MATRIX_SIZE - 1 - x);
  }
  
  uint16_t offset = index * 3;
  ledBuffer[offset] = g;
  ledBuffer[offset+1] = r;
  ledBuffer[offset+2] = b;
}

void clearMatrix() {
  memset(ledBuffer, 0, NUM_LEDS * 3);
}

// Función para dibujar la letra S
void drawLetterS(uint8_t offsetX, uint8_t offsetY, uint8_t r, uint8_t g, uint8_t b) {
  setPixel(offsetX+1, offsetY, r, g, b);
  setPixel(offsetX+2, offsetY, r, g, b);
  setPixel(offsetX+3, offsetY, r, g, b);
  setPixel(offsetX, offsetY+1, r, g, b);
  setPixel(offsetX, offsetY+2, r, g, b);
  setPixel(offsetX+1, offsetY+2, r, g, b);
  setPixel(offsetX+2, offsetY+2, r, g, b);
  setPixel(offsetX+3, offsetY+3, r, g, b);
  setPixel(offsetX+3, offsetY+4, r, g, b);
  setPixel(offsetX, offsetY+4, r, g, b);
  setPixel(offsetX+1, offsetY+4, r, g, b);
  setPixel(offsetX+2, offsetY+4, r, g, b);
}

// Función para dibujar la letra N
void drawLetterN(uint8_t offsetX, uint8_t offsetY, uint8_t r, uint8_t g, uint8_t b) {
  for(uint8_t i = 0; i < 5; i++) {
    setPixel(offsetX, offsetY+i, r, g, b);
    setPixel(offsetX+3, offsetY+i, r, g, b);
  }
  setPixel(offsetX+1, offsetY+1, r, g, b);
  setPixel(offsetX+2, offsetY+2, r, g, b);
  setPixel(offsetX+1, offsetY+3, r, g, b);
}

// Función para dibujar la letra A
void drawLetterA(uint8_t offsetX, uint8_t offsetY, uint8_t r, uint8_t g, uint8_t b) {
  for(uint8_t i = 1; i < 5; i++) {
    setPixel(offsetX, offsetY+i, r, g, b);
    setPixel(offsetX+3, offsetY+i, r, g, b);
  }
  setPixel(offsetX+1, offsetY, r, g, b);
  setPixel(offsetX+2, offsetY, r, g, b);
  setPixel(offsetX+1, offsetY+2, r, g, b);
  setPixel(offsetX+2, offsetY+2, r, g, b);
}

// Función para dibujar la letra K
void drawLetterK(uint8_t offsetX, uint8_t offsetY, uint8_t r, uint8_t g, uint8_t b) {
  for(uint8_t i = 0; i < 5; i++) {
    setPixel(offsetX, offsetY+i, r, g, b);
  }
  setPixel(offsetX+1, offsetY+2, r, g, b);
  setPixel(offsetX+2, offsetY+1, r, g, b);
  setPixel(offsetX+2, offsetY+3, r, g, b);
  setPixel(offsetX+3, offsetY, r, g, b);
  setPixel(offsetX+3, offsetY+4, r, g, b);
}

// Función para dibujar la letra E
void drawLetterE(uint8_t offsetX, uint8_t offsetY, uint8_t r, uint8_t g, uint8_t b) {
  for(uint8_t i = 0; i < 5; i++) {
    setPixel(offsetX, offsetY+i, r, g, b);
  }
  setPixel(offsetX+1, offsetY, r, g, b);
  setPixel(offsetX+2, offsetY, r, g, b);
  setPixel(offsetX+1, offsetY+2, r, g, b);
  setPixel(offsetX+2, offsetY+2, r, g, b);
  setPixel(offsetX+1, offsetY+4, r, g, b);
  setPixel(offsetX+2, offsetY+4, r, g, b);
}

// Mostrar título SNAKE
void showTitle() {
  clearMatrix();
  
  // Dibujar SNAKE centrado (cada letra ocupa 4x5 píxeles)
  uint8_t startX = (MATRIX_SIZE - 20) / 2; // 4 letras * 4 + 4 espacios = 20
  
  drawLetterS(startX, 8, 0, 255, 0);       // Verde
  drawLetterN(startX+5, 8, 255, 0, 0);     // Rojo
  drawLetterA(startX+10, 8, 0, 0, 255);    // Azul
  drawLetterK(startX+15, 8, 255, 0, 255);  // Magenta
  drawLetterE(startX+20, 8, 255, 255, 0);  // Amarillo
  
  updateLEDs();
  
  // Animación de aparición
  for(int i = 0; i < 5; i++) {
    playTone(400 + i*100, 50);
    _delay_ms(100);
  }
  
  // Esperar 2 segundos o hasta que se presione un botón
  for(int i = 0; i < 20; i++) {
    if(!(PINB & (1 << 0)) || !(PINB & (1 << 1)) || 
       !(PIND & (1 << BUTTON_UP)) || !(PIND & (1 << BUTTON_DOWN))) {
      break;
    }
    _delay_ms(100);
  }
}

// Animación de borrado
void clearAnimation() {
  for(int y = 0; y < MATRIX_SIZE; y++) {
    for(int x = 0; x < MATRIX_SIZE; x++) {
      setPixel(x, y, 0, 0, 0);
    }
    updateLEDs();
    _delay_ms(30);
    playTone(100 + y*20, 20);
  }
}

void generateFood() {
  foodX = rand() % MATRIX_SIZE;
  foodY = rand() % MATRIX_SIZE;
  
  for(int i = 0; i < snakeLength; i++) {
    if(foodX == tailX[i] && foodY == tailY[i]) {
      generateFood();
      return;
    }
  }
}

void drawGame() {
  clearMatrix();
  
  // Dibujar comida (magenta)
  setPixel(foodX, foodY, 255, 0, 255);
  
  // Dibujar cabeza de la serpiente (rojo)
  setPixel(snakeX, snakeY, 255, 0, 0);
  
  // Dibujar cola (verde)
  for(int i = 0; i < snakeLength; i++) {
    setPixel(tailX[i], tailY[i], 0, 255, 0);
  }
  
  updateLEDs();
}

void readInput() {
  if(!(PINB & (1 << 0))) { // LEFT
    if(dir != RIGHT) dir = LEFT;
    playTone(500, 50);
  }
  if(!(PINB & (1 << 1))) { // RIGHT
    if(dir != LEFT) dir = RIGHT;
    playTone(500, 50);
  }
  if(!(PIND & (1 << BUTTON_UP))) { // UP
    if(dir != DOWN) dir = UP;
    playTone(600, 50);
  }
  if(!(PIND & (1 << BUTTON_DOWN))) { // DOWN
    if(dir != UP) dir = DOWN;
    playTone(600, 50);
  }
  if(!(PIND & (1 << BUTTON_A))) { // PAUSE
    playTone(800, 100);
    while(!(PIND & (1 << BUTTON_A))) {} // Esperar a soltar
  }
  if(!(PIND & (1 << BUTTON_B))) { // RESET
    gameOver = true;
  }
}

void updateGame() {
  // Mover cola
  for(int i = snakeLength-1; i > 0; i--) {
    tailX[i] = tailX[i-1];
    tailY[i] = tailY[i-1];
  }
  tailX[0] = snakeX;
  tailY[0] = snakeY;
  
  // Mover cabeza
  switch(dir) {
    case LEFT:  snakeX--; break;
    case RIGHT: snakeX++; break;
    case UP:    snakeY--; break;
    case DOWN:  snakeY++; break;
    case STOP:  break;
  }
  
  // Detectar colisiones
  if(snakeX < 0 || snakeX >= MATRIX_SIZE || snakeY < 0 || snakeY >= MATRIX_SIZE) {
    gameOver = true;
  }
  
  for(int i = 0; i < snakeLength; i++) {
    if(snakeX == tailX[i] && snakeY == tailY[i]) {
      gameOver = true;
    }
  }
  
  // Comer comida
  if(snakeX == foodX && snakeY == foodY) {
    snakeLength++;
    playTone(1000, 100);
    generateFood();
  }
}

void playTone(uint16_t freq, uint16_t dur) {
  if(freq == 0) {
    PORTD &= ~(1 << BUZZER_PIN);
    return;
  }
  
  uint32_t cycles = (uint32_t)freq * dur / 1000;
  uint16_t halfPeriod = 500000 / freq;
  
  for(uint32_t i = 0; i < cycles; i++) {
    PORTD |= (1 << BUZZER_PIN);
    for(uint16_t j = 0; j < halfPeriod; j++) { _delay_us(1); }
    PORTD &= ~(1 << BUZZER_PIN);
    for(uint16_t j = 0; j < halfPeriod; j++) { _delay_us(1); }
  }
}

void gameOverAnimation() {
  // Animación de game over
  for(int i = 0; i < 3; i++) {
    for(int y = 0; y < MATRIX_SIZE; y++) {
      for(int x = 0; x < MATRIX_SIZE; x++) {
        setPixel(x, y, 255, 0, 0);
      }
    }
    updateLEDs();
    playTone(200, 200);
    _delay_ms(200);
    
    clearMatrix();
    updateLEDs();
    _delay_ms(200);
  }
  
  // Animación de borrado
  clearAnimation();
}

void setup() {
  // Configurar pines
  DDRD |= (1 << LED_PIN) | (1 << BUZZER_PIN);
  DDRD &= ~((1 << BUTTON_UP) | (1 << BUTTON_DOWN) | (1 << BUTTON_A) | (1 << BUTTON_B));
  PORTD |= (1 << BUTTON_UP) | (1 << BUTTON_DOWN) | (1 << BUTTON_A) | (1 << BUTTON_B);
  
  DDRB &= ~((1 << 0) | (1 << 1));
  PORTB |= (1 << 0) | (1 << 1);
  
  // Mostrar título
  showTitle();
  clearAnimation();
  
  // Inicializar juego
  dir = STOP;
  generateFood();
  
  // Melodía de inicio
  playTone(523, 200);
  playTone(659, 200);
  playTone(784, 200);
}

void loop() {
  readInput();
  
  if(millis() - lastUpdate > updateInterval && !gameOver) {
    if(dir != STOP) {
      updateGame();
      drawGame();
    }
    lastUpdate = millis();
  }
  
  if(gameOver) {
    gameOverAnimation();
    
    // Reiniciar juego
    snakeX = 10;
    snakeY = 10;
    snakeLength = 3;
    dir = STOP;
    generateFood();
    gameOver = false;
    drawGame();
  }
}
