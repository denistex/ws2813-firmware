#include <avr/eeprom.h> 
#include <avr/interrupt.h>
#include <stdlib.h>
#include <util/delay.h>

#define BZ 0x10
#define B1 0x27

#define RGBLEN 24
unsigned char g_index = 0;
unsigned char g_rgb[] = {
	B1, BZ, BZ, B1, BZ, B1, B1, BZ,
	B1, B1, BZ, BZ, B1, BZ, B1, BZ,
	B1, B1, B1, B1, BZ, BZ, BZ, BZ
};

void next_bit (void) {
	if (g_index == RGBLEN) {
		TCCR1 = _BV(CTC1) | _BV(PWM1A) | _BV(COM1A1);
	} else {
		OCR1A = g_rgb[g_index];
		g_index++;
	}
}

ISR (TIMER1_COMPA_vect) {
	next_bit();
}

void init_pll (void) {
	static const uint8_t PLL_DELAY = 100;

	PORTB |= _BV(PORTB2);

	_delay_us(PLL_DELAY);

	// WARNING: isis proteus hungs here, comment out before simulation
	while (!(PLLCSR & _BV(PLOCK))) {}

	PLLCSR |= _BV(PCKE);

	PORTB &= ~_BV(PORTB2);
}

void init (void) {
	DDRB = _BV(DDB2) | _BV(DDB1);
	PORTB = 0;

	init_pll();

	TCNT1 = 0;
	OCR1C = 0x50;

	TIMSK = _BV(OCIE1A);

	next_bit();
	TCCR1 = _BV(CTC1) | _BV(PWM1A) | _BV(COM1A1) | _BV(CS10);

	sei();
}

void read_next_rgb (void) {
	static void* eeprom_ptr = 0;

	const unsigned char byte = 8;
	const unsigned char len = RGBLEN / byte;
	unsigned char buffer[len];

	eeprom_busy_wait();
	eeprom_read_block(buffer, eeprom_ptr, len);
	eeprom_ptr += len;

	// testing code
	buffer[0] = 0xF0;
	buffer[1] = 0xCA;
	buffer[2] = 0x96;
	// testing code

	for (unsigned char i = 0; i < len; ++i) {
		const unsigned char value = buffer[i];
		for (signed char j = byte - 1; j >= 0; --j) {
			const unsigned char index = i * byte + byte - j - 1;
			g_rgb[index] = value & _BV(j) ? B1 : BZ;
		}
	}
}

int main (void) {
	init();
	while (1) {
		if ((TCCR1 & _BV(CS10)) == 0) {
			read_next_rgb();

			g_index = 0;
			next_bit();
			TCCR1 = _BV(CTC1) | _BV(PWM1A) | _BV(COM1A1) | _BV(CS10);
		}
	}
}
