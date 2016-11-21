#include <avr/interrupt.h>
#include <stdlib.h>
#include <util/delay.h>

#define ISS_DELAY 100
//#define ISS_DELAY 200

const unsigned char BIT0 = 0x0F;
const unsigned char BIT1 = 0x26;

#define BIT_MAX 0x07
unsigned char g_bit = _BV(BIT_MAX);

unsigned char g_buffer = 0x96;

void next_bit (void) {
	if (g_bit == 0) {
		TCCR1 = _BV(CTC1) | _BV(PWM1A) | _BV(COM1A1);
	} else {
		OCR1A = g_buffer & g_bit ? BIT1 : BIT0;
		g_bit >>= 1;
	}
}

ISR (TIMER1_COMPA_vect) {
	next_bit();
}

void init_pll (void) {
	PORTB = _BV(PORTB2);

	_delay_us(ISS_DELAY);
	while (PLLCSR & _BV(PLOCK)) {}
	PLLCSR |= _BV(PCKE);

	PORTB = 0;
}

void init (void) {
	DDRB = _BV(DDB2) | _BV(DDB1);

	init_pll();

	TCNT1 = 0;
	OCR1C = 0x4F;

	TIMSK = _BV(OCIE1A);

	next_bit();
	TCCR1 = _BV(CTC1) | _BV(PWM1A) | _BV(COM1A1) | _BV(CS10);

	sei();
}

int main (void) {
	init();
	while (1) {
		if ((TCCR1 & _BV(CS10)) == 0) {
			PORTB |= _BV(PORTB2);
			g_bit = _BV(BIT_MAX);
			g_buffer = 0xCA;

			next_bit();
			TCCR1 = _BV(CTC1) | _BV(PWM1A) | _BV(COM1A1) | _BV(CS10);
		}
	}
}
