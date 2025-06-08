
//un paint muy basico sin librerias externas y haciendo uso de ensamblador, es increible lo rapido que compila y sube el codigo en comparacion con la version que usa la libreria para los neopixel


#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

// Definición de pines
#define LED_PIN 2        // D2
#define BUZZER_PIN 3     // D3
#define BUTTON_B 4       // D4
#define BUTTON_A 5       // D5
#define BUTTON_UP 6      // D6
#define BUTTON_DOWN 7    // D7
#define BUTTON_LEFT 8    // D8 (PB0)
#define BUTTON_RIGHT 9   // D9 (PB1)

#define MATRIX_SIZE 22
#define NUM_LEDS (MATRIX_SIZE * MATRIX_SIZE)

uint8_t ledBuffer[NUM_LEDS * 3];

// Bitmap en PROGMEM
const uint8_t bitmap[22][22] PROGMEM = {
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,1,1,0,2,0,0,3,0,3,0,4,4,5,5,5,6,6,6,7,7,0},
  {0,1,0,1,2,0,0,3,0,3,4,0,0,0,5,0,6,0,0,7,0,7},
  {0,1,0,1,2,0,0,3,0,3,4,4,4,0,5,0,6,0,0,7,0,7},
  {0,1,1,0,2,0,0,3,0,3,0,0,4,0,5,0,6,6,0,7,7,0},
  {0,1,0,0,2,0,0,3,0,3,0,0,4,0,5,0,6,0,0,7,0,7},
  {0,1,0,0,2,2,2,3,3,3,4,4,0,0,5,0,6,6,6,7,0,7},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {5,5,5,5,5,5,0,0,0,0,2,2,2,0,3,3,0,0,0,0,3,3},
  {5,5,5,5,5,5,5,5,0,0,2,2,2,0,3,3,0,0,0,0,3,3},
  {5,5,0,0,0,0,5,5,0,2,2,0,2,2,0,3,3,0,0,3,3,0},
  {5,5,0,0,0,0,5,5,0,2,2,0,2,2,0,3,3,0,0,3,3,0},
  {5,5,0,0,0,0,5,5,0,2,2,0,2,2,0,0,3,3,3,3,0,0},
  {5,5,5,5,5,5,5,0,0,2,2,0,2,2,0,0,3,3,3,3,0,0},
  {5,5,5,5,5,5,5,0,0,2,2,0,2,2,0,0,0,3,3,0,0,0},
  {5,5,0,0,0,0,5,5,0,2,2,0,2,2,0,0,0,3,3,0,0,0},
  {5,5,0,0,0,0,5,5,0,2,2,0,2,2,0,0,0,3,3,0,0,0},
  {5,5,0,0,0,0,5,5,0,2,2,0,2,2,0,0,0,3,3,0,0,0},
  {5,5,5,5,5,5,5,0,0,0,2,2,2,0,0,0,0,3,3,0,0,0},
  {5,5,5,5,5,5,0,0,0,0,2,2,2,0,0,0,0,3,3,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};

// Variables para el pixel
int pixelX = 0;
int pixelY = 0;
uint8_t pixelColorIndex = 0;

// Colores (GRB)
const uint8_t colors[8][3] = {
  {0x00, 0x00, 0x00}, // 0: Apagado
  {0x00, 0xFF, 0x00}, // 1: Rojo
  {0xFF, 0x00, 0x00}, // 2: Verde
  {0x00, 0x00, 0xFF}, // 3: Azul
  {0x93, 0x14, 0xFF}, // 4: Pink
  {0x80, 0x00, 0x80}, // 5: Purple
  {0xFF, 0xFF, 0xFF}, // 6: White
  {0xFF, 0xFF, 0x00}  // 7: Yellow
};

// Variables para debounce
unsigned long lastDebounceTime = 0;
#define DEBOUNCE_DELAY 50

// Estado del botón B
bool bButtonActive = false;

// Prototipos
void sendByte(uint8_t byte);
void updateLEDs();
void setPixel(uint8_t x, uint8_t y, uint8_t r, uint8_t g, uint8_t b);
void clearMatrix();
void drawBitmap();
void drawPixel();
void movePixel(int8_t dx, int8_t dy);
void changePixelColor();
void playClick();
void playStartMelody();
uint8_t readButtons();
void beep(uint16_t freq, uint16_t dur);

// Función para enviar un byte (optimizada en ensamblador)
void sendByte(uint8_t byte) {
  asm volatile (
    "ldi r18, 8      \n"   // Contador de 8 bits
    "next_bit:       \n"
    "sbi %[port], %[pin] \n" // HIGH
    "lsl %[byte]     \n"   // Shift left
    "brcs bit_is_one \n"   // Si bit=1
    
    // Bit 0
    "nop             \n"
    "nop             \n"
    "cbi %[port], %[pin] \n" // LOW
    "nop             \n"
    "nop             \n"
    "rjmp end_bit    \n"
    
    // Bit 1
    "bit_is_one:     \n"
    "nop             \n"
    "nop             \n"
    "nop             \n"
    "nop             \n"
    "cbi %[port], %[pin] \n" // LOW
    
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

// Actualizar LEDs
void updateLEDs() {
  cli();
  for(uint16_t i = 0; i < NUM_LEDS * 3; i++) {
    sendByte(ledBuffer[i]);
  }
  sei();
  _delay_us(50);
}

// Establecer color de un pixel
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

// Limpiar matriz
void clearMatrix() {
  memset(ledBuffer, 0, NUM_LEDS * 3);
}

// Dibujar bitmap
void drawBitmap() {
  for(uint8_t y = 0; y < MATRIX_SIZE; y++) {
    for(uint8_t x = 0; x < MATRIX_SIZE; x++) {
      uint8_t colorIndex = pgm_read_byte(&bitmap[y][x]);
      setPixel(x, y, 
               colors[colorIndex][1], 
               colors[colorIndex][0], 
               colors[colorIndex][2]);
    }
  }
}

// Dibujar pixel actual
void drawPixel() {
  setPixel(pixelX, pixelY, 
           colors[pixelColorIndex][1],
           colors[pixelColorIndex][0],
           colors[pixelColorIndex][2]);
}

// Mover pixel
void movePixel(int8_t dx, int8_t dy) {
  int8_t newX = pixelX + dx;
  int8_t newY = pixelY + dy;
  
  if(newX >= 0 && newX < MATRIX_SIZE && newY >= 0 && newY < MATRIX_SIZE) {
    pixelX = newX;
    pixelY = newY;
  }
}

// Cambiar color del pixel
void changePixelColor() {
  pixelColorIndex = (pixelColorIndex + 1) % 4; // 0-3
}

// Leer botones
uint8_t readButtons() {
  uint8_t buttons = 0;
  
  if(!(PIND & (1 << BUTTON_A))) buttons |= 0x01;
  if(!(PIND & (1 << BUTTON_B))) buttons |= 0x02;
  if(!(PIND & (1 << BUTTON_UP))) buttons |= 0x04;
  if(!(PIND & (1 << BUTTON_DOWN))) buttons |= 0x08;
  
  if(!(PINB & (1 << 0))) buttons |= 0x10; // LEFT
  if(!(PINB & (1 << 1))) buttons |= 0x20; // RIGHT
  
  return buttons;
}

// Beep sin usar delayMicroseconds()
void beep(uint16_t freq, uint16_t dur) {
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

// Click
void playClick() {
  beep(1000, 20);
}

// Melodía de inicio
void playStartMelody() {
  uint16_t notes[] = {262, 330, 392, 523};
  uint16_t durs[] = {200, 200, 200, 400};
  
  for(uint8_t i = 0; i < 4; i++) {
    beep(notes[i], durs[i]);
    for(uint16_t j = 0; j < durs[i] + 50; j++) { _delay_ms(1); }
  }
}

void setup() {
  // Configurar pines
  DDRD |= (1 << LED_PIN) | (1 << BUZZER_PIN);
  DDRD &= ~((1 << BUTTON_A) | (1 << BUTTON_B) | (1 << BUTTON_UP) | (1 << BUTTON_DOWN));
  PORTD |= (1 << BUTTON_A) | (1 << BUTTON_B) | (1 << BUTTON_UP) | (1 << BUTTON_DOWN);
  
  DDRB &= ~((1 << 0) | (1 << 1));
  PORTB |= (1 << 0) | (1 << 1);
  
  // Inicializar matriz
  clearMatrix();
  drawBitmap();
  updateLEDs();
  
  // Melodía de inicio
  playStartMelody();
}

void loop() {
  uint8_t buttons = readButtons();
  
  if(millis() - lastDebounceTime > DEBOUNCE_DELAY) {
    if(buttons & 0x02) { // Botón B
      clearMatrix();
      playClick();
      lastDebounceTime = millis();
    } 
    else if(buttons & 0x01) { // Botón A
      playClick();
      changePixelColor();
      lastDebounceTime = millis();
    } 
    else if(buttons & 0x04) { // UP
      playClick();
      movePixel(0, -1);
      lastDebounceTime = millis();
    } 
    else if(buttons & 0x08) { // DOWN
      playClick();
      movePixel(0, 1);
      lastDebounceTime = millis();
    } 
    else if(buttons & 0x10) { // LEFT
      playClick();
      movePixel(-1, 0);
      lastDebounceTime = millis();
    } 
    else if(buttons & 0x20) { // RIGHT
      playClick();
      movePixel(1, 0);
      lastDebounceTime = millis();
    }
  }
  
  drawPixel();
  updateLEDs();
  _delay_ms(20);
}
