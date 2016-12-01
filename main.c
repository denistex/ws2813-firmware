#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define ZERO_BIT_DELAY asm volatile("nop\n\tnop\n\t")

void _flash1 (const uint16_t count) {
	for (uint16_t i = 0; i < count; ++i) {
		PORTB |= _BV(PORTB3);
		_delay_ms(5);
		PORTB &= ~_BV(PORTB3);
		_delay_ms(995);
	}
}

void _flash2 (const uint16_t count) {
	for (uint16_t i = 0; i < count; ++i) {
		PORTB |= _BV(PORTB3);
		_delay_ms(50);
		PORTB &= ~_BV(PORTB3);
		_delay_ms(950);
	}
}

void ledon (void) {
	PORTB |= _BV(PORTB3);
}

void ledoff (void) {
	PORTB &= ~_BV(PORTB3);
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

void init (void) {
	DDRB = _BV(DDB1) | _BV(DDB3);
	PORTB = 0;
	sei();
}

int main (void) {
	init();

	const uint16_t* ptr = 0;

	uint8_t repeat = 0;
	while (1) {
		const uint16_t data = read(ptr);
		if (repeat == 0) {
			ptr++;
		} else {
			repeat--;
		}

		if (data & _BV(0)) {
			if (data & _BV(1)) {
				const uint16_t command = data >> 2;

				if (command == 0) {
					ptr = 0;
					ledon();
					_delay_us(100); // commit delay
					ledoff();
				}
			} else {
				const uint16_t delay = data >> 2;
				for (uint16_t i = 0; i < delay; ++i) _delay_ms(1);
			}
		} else {
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
}
