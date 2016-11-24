#include <avr/interrupt.h>
#include <stdlib.h>
#include <util/delay.h>

// this is check firmware for attiny25, attiny45 or attiny85
// outputs 4 different square waves:
// PB1: 15.26 Hz (timer 1 at 64 MHz with 16384 prescaler and OCR1C = 0xFF)
// PB2: thin (50 ms) peaks at 1 Hz (each second) from main loop
// PB3: 1 Hz from timer 0 (16 MHz with 256 prescaler and overflows counter)
// PB4: 1 Hz from timer 1 (64 MHz with 16384 prescaler and overflows counter)

// fuses:
// avrdude -U lfuse:w:0xf1:m -U hfuse:w:0xdf:m -U efuse:w:0xff:m

// isis proteus warnings:
//  - core and timer 0 is working at 64 MHz (16 MHz is actually max for
//    internal osc with PLL)
//  - proteus runs timer 1 at 64 MHz (this is correct)
//  - proteus doens't set PLOCK bit when PLL is locked so comment out
//    checking line before simulation (additional comment provided there)

ISR (TIM0_OVF_vect) {
	static const uint8_t MAX_T0 = 122;
	static uint8_t g_counter_t0 = 0;

	if (g_counter_t0++ == MAX_T0) {
		g_counter_t0 = 0;
		PORTB ^= _BV(PORTB3);
	}
}

ISR (TIMER1_COMPA_vect) {
	static const uint8_t MAX_T1 = 7;
	static uint8_t g_counter_t1 = 0;

	if (g_counter_t1++ == MAX_T1) {
		g_counter_t1 = 0;
		PORTB ^= _BV(PORTB4);
	}
}

void init_pll (void) {
	const uint8_t PLL_DELAY = 100;

	PORTB |= _BV(PORTB4);

	_delay_us(PLL_DELAY);

	// WARNING: isis proteus hungs here, comment out before simulation
	while (!(PLLCSR & _BV(PLOCK))) {}

	// PCKE: async mode of timer/counter 1
	PLLCSR |= _BV(PCKE);

	PORTB &= ~_BV(PORTB4);
}

void configure (void) {
	// DDB[4:0] = 11001: output direction on pins PB4, PB3, PB2, PB1
	DDRB = _BV(DDB4) | _BV(DDB3) | _BV(DDB2) | _BV(DDB1);
	PORTB = 0;

	init_pll();

	TCNT0 = 0;
	TCNT1 = 0;

	OCR1A = 0x7F;
	OCR1C = 0xFF;

	// TOIE0 = 1: overflow interrupt of timer/counter 0 enabled
	// OCIE1A = 1: compare match interrupt of timer/counter 1 enabled
	TIMSK = _BV(TOIE0) | _BV(OCIE1A);

	// CS0[2:0] = 100: timer/counter 0 clock source is CLKio / 256
	TCCR0B =  _BV(CS02);

	// CTC1 = 1: clear timer/counter on compare match
	// PWM1A = 1: pulse width modulator a enabled
	// COM1A[1:0] = 10: clear the OC1A output line
	// CS1[3:0] = 1111: prescaling source: PCK / 16384
	TCCR1 = _BV(CTC1) | _BV(PWM1A) | _BV(COM1A1)
		| _BV(CS13) | _BV(CS12) | _BV(CS11) | _BV(CS10);

	// enable interrupts globally
	sei();
}

int main (void) {
	configure();

    while (1) {		
		_delay_ms(950);
		PORTB |= _BV(PORTB2);
		_delay_ms(50);
		PORTB &= ~_BV(PORTB2);
    }
}
