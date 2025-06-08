// prueba de codigo sin usar fastled o similares, se usa ensamblador para sustituir lo que vendria siendo dicha libreria, en fin ,es que no tengo nada mejor que hacer... 


#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>

#define LED_PIN PD2        // Pin D2 (PD2 en ATmega328P)
#define MATRIX_SIZE 22     // 22x22 = 484 LEDs
#define NUM_LEDS (MATRIX_SIZE * MATRIX_SIZE)

// Buffer para almacenar todos los colores de los LEDs (formato GRB)
uint8_t ledBuffer[NUM_LEDS * 3];

// Prototipos de funciones
void sendByteASM(uint8_t byte);
void updateLEDs();
void setPixel(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b);
void setup();
void loop();

// Función optimizada en ensamblador para enviar un byte
void sendByteASM(uint8_t byte) {
  asm volatile (
    "ldi r16, 8      \n"   // Contador de 8 bits
    "next_bit_%=:    \n"
    "sbi %[port], %[pin] \n"  // Encender pin (HIGH)
    "lsl %[byte]     \n"   // Shift izquierda (MSB primero)
    "brcs bit_1_%=   \n"   // Si el bit es 1, saltar
    
    // Bit "0" (HIGH ~0.35µs, LOW ~0.8µs)
    "nop             \n"   // Delay para bit "0"
    "nop             \n"
    "cbi %[port], %[pin] \n"  // Apagar pin (LOW)
    "nop             \n"
    "nop             \n"
    "rjmp end_bit_%= \n"
    
    // Bit "1" (HIGH ~0.7µs, LOW ~0.6µs)
    "bit_1_%=:       \n"
    "nop             \n"   // Delay extra para bit "1"
    "nop             \n"
    "nop             \n"
    "nop             \n"
    "cbi %[port], %[pin] \n"  // Apagar pin (LOW)
    
    "end_bit_%=:     \n"
    "dec r16         \n"   // Decrementar contador
    "brne next_bit_%= \n"  // Repetir si quedan bits
    :
    : [port] "I" (_SFR_IO_ADDR(PORTD)),
      [pin] "I" (LED_PIN),
      [byte] "r" (byte)
    : "r16"
  );
}

// Actualiza todos los LEDs con el contenido del buffer
void updateLEDs() {
  cli();  // Deshabilitar interrupciones
  
  // Enviar todo el buffer
  uint8_t *ptr = ledBuffer;
  for(uint16_t i = 0; i < NUM_LEDS * 3; i++) {
    sendByteASM(*ptr++);
  }
  
  sei();  // Rehabilitar interrupciones
  _delay_us(50);  // Tiempo de reset
}

// Establece el color de un pixel específico en la matriz
void setPixel(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b) {
  // Mapeo de coordenadas a índice lineal (serpentina)
  uint16_t index;
  if(y % 2 == 0) {
    // Filas pares: izquierda a derecha
    index = y * MATRIX_SIZE + x;
  } else {
    // Filas impares: derecha a izquierda
    index = y * MATRIX_SIZE + (MATRIX_SIZE - 1 - x);
  }
  
  // Almacenar en buffer (formato GRB)
  uint16_t offset = index * 3;
  ledBuffer[offset] = g;
  ledBuffer[offset+1] = r;
  ledBuffer[offset+2] = b;
}

// Limpia toda la matriz (todos los LEDs apagados)
void clearMatrix() {
  memset(ledBuffer, 0, NUM_LEDS * 3);
}

void setup() {
  DDRD |= (1 << LED_PIN);  // Configurar D2 como salida
  PORTD &= ~(1 << LED_PIN); // Inicializar en LOW
  clearMatrix();
  updateLEDs();
}

void loop() {
  // Efecto: Onda pulsante roja desde el centro
  static uint8_t brightness = 0;
  static int8_t direction = 1;
  
  brightness += direction;
  if(brightness >= 100 || brightness <= 0) {
    direction *= -1;
  }
  
  // Calcular color base
  uint8_t r = brightness;
  uint8_t g = 0;
  uint8_t b = 0;
  
  // Dibujar patrón circular
  for(uint8_t y = 0; y < MATRIX_SIZE; y++) {
    for(uint8_t x = 0; x < MATRIX_SIZE; x++) {
      // Calcular distancia desde el centro
      int8_t dx = x - MATRIX_SIZE/2;
      int8_t dy = y - MATRIX_SIZE/2;
      uint8_t distance = sqrt(dx*dx + dy*dy);
      
      // Ajustar brillo según la distancia
      uint8_t pixelBrightness = brightness - distance*3;
      if(pixelBrightness > brightness) pixelBrightness = 0;
      
      setPixel(x, y, r * pixelBrightness / 100, g, b);
    }
  }
  
  updateLEDs();
  _delay_ms(20);
}
