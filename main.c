#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define ZERO_BIT_DELAY asm volatile("nop\n\tnop\n\t")

void _flash (const uint16_t count) {
	PORTB |= _BV(PORTB3);
	_delay_ms(3000);
	PORTB &= ~_BV(PORTB3);
	_delay_ms(2000);

	for (uint16_t i = 0; i < count; ++i) {
		PORTB |= _BV(PORTB3);
		_delay_ms(10);
		PORTB &= ~_BV(PORTB3);
		_delay_ms(1000);
	}

	PORTB |= _BV(PORTB3);
	_delay_ms(3000);
	PORTB &= ~_BV(PORTB3);
	_delay_ms(2000);

}

uint16_t read (const uint16_t* const ptr) {
	eeprom_busy_wait();
	return eeprom_read_word(ptr);
}

inline void send (const uint16_t value) {
	if (value == 0) {
		asm volatile(
			"sbi %0, 1\n\t"
			"nop\n\t"
			"nop\n\t"
			"cbi %0, 1\n\t"
			"nop\n\t"
			"nop\n\t"
			"nop\n\t"
			"nop\n\t"
			"nop\n\t"
			"nop\n\t"
			"nop\n\t"
			"nop\n\t"
			"nop\n\t"
			"nop\n\t"
			"nop\n\t"
			"nop\n\t"
			:: "I" _SFR_IO_ADDR(PORTB));
	} else {
		asm volatile(
			"sbi %0, 1\n\t"
			"nop\n\t"
			"nop\n\t"
			"nop\n\t"
			"nop\n\t"
			"nop\n\t"
			"nop\n\t"
			"nop\n\t"
			"cbi %0, 1\n\t"
			"nop\n\t"
			"nop\n\t"
			"nop\n\t"
			"nop\n\t"
			"nop\n\t"
			"nop\n\t"
			:: "I" _SFR_IO_ADDR(PORTB));
	}
}

void commit (void) {
	_delay_ms(100);
}

void init (void) {
	DDRB = _BV(DDB1) | _BV(DDB3);
	PORTB = 0;
	sei();
}

int main (void) {
	init();

	const uint16_t cmdlen = 1;
	const uint16_t cmdcount = read((uint16_t*)0);
	const uint16_t* const first = (uint16_t*)2;
	const uint16_t* const last = first + cmdcount * cmdlen;

	const uint16_t* ptr = first;

	while (1) {
		if (ptr == last) {
			ptr = first;
			commit();
			continue;
		}

		const uint16_t data = read(ptr);
		ptr += cmdlen;

		if (data & _BV(0)) {
			commit();
			const uint16_t cycles = data >> 1;
			for (uint16_t i = 0; i < cycles; ++i) _delay_ms(100);
			continue;
		}

		send(data & _BV(15));
		send(data & _BV(14));
		send(data & _BV(13));
		send(data & _BV(12));
		send(data & _BV(11));
		send(0);
		ZERO_BIT_DELAY; send(0);
		ZERO_BIT_DELAY; send(0);

		send(data & _BV(10));
		send(data & _BV(9));
		send(data & _BV(8));
		send(data & _BV(7));
		send(data & _BV(6));
		send(0);
		ZERO_BIT_DELAY; send(0);
		ZERO_BIT_DELAY; send(0);

		send(data & _BV(5));
		send(data & _BV(4));
		send(data & _BV(3));
		send(data & _BV(2));
		send(data & _BV(1));
		send(0);
		ZERO_BIT_DELAY; send(0);
		ZERO_BIT_DELAY; send(0);
	}
}
