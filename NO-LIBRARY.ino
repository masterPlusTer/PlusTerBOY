// prueba de codigo sin usar fastled o similares, se usa ensamblador para sustituir lo que vendria siendo dicha libreria, en fin ,es que no tengo nada mejor que hacer... 


#include <avr/io.h>
#include <util/delay.h>

#define LED_PIN PD2  // Pin D2 (PD2 en ATmega328P)

// Función en ensamblador para enviar un byte a los LEDs
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

// Función para enviar color completo (G, R, B - orden WS2812B)
void sendColor(uint8_t r, uint8_t g, uint8_t b) {
  cli();  // Deshabilitar interrupciones
  sendByteASM(g);  // Orden GRB
  sendByteASM(r);
  sendByteASM(b);
  sei();  // Rehabilitar interrupciones
  _delay_us(50);  // Tiempo de reset
}

void setup() {
  DDRD |= (1 << LED_PIN);  // Configurar D2 como salida
  PORTD &= ~(1 << LED_PIN); // Inicializar en LOW
}

void loop() {
  // Efecto de respiración en ROJO
  for (int i = 0; i < 256; i++) {
    sendColor(i, 0, 0);  // Incrementar brillo ROJO
    _delay_ms(10);
  }
  for (int i = 255; i >= 0; i--) {
    sendColor(i, 0, 0);  // Decrementar brillo ROJO
    _delay_ms(10);
  }
}
