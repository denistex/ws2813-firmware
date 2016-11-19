#include <avr/interrupt.h>
#include <stdlib.h>
#include <util/delay.h>

const uint8_t BIT0 = 0x0F;
const uint8_t BIT1 = 0x26;

const uint8_t BIT_MAX = 0x0F;

uint16_t g_buffer = 0x96CA;

void next_bit (void) {
	OCR1A = g_buffer & _BV(BIT_MAX) ? BIT1 : BIT0;
	g_buffer <<= 1;
}

ISR (TIMER1_COMPA_vect) {
	next_bit();
}

void configure (void) {
	DDRB = _BV(DDB1);
	PORTB = 0;

	_delay_us(100);
	while (PLLCSR & _BV(PLOCK)) {}
	PLLCSR |= _BV(PCKE);

	TCNT1 = 0;
	OCR1C = 0x4F;

	next_bit();

	TIMSK = _BV(OCIE1A);
	TCCR1 = _BV(CTC1) | _BV(PWM1A) | _BV(COM1A1) | _BV(CS10);

	sei();
}

int main (void) {
	configure();
	while (1) {}
}
