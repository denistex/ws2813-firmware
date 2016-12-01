#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define ZERO_BIT_DELAY asm volatile("nop\n\tnop\n\t")

typedef union {
	uint16_t word;
	uint8_t byte[2];
} word_t;

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

	static const uint8_t CMD_END = 0xFF;
	static const uint8_t CMD_REPEAT = 0x07;

	static const uint8_t MASK_DELAY = 0x03;

	const uint16_t* ptr = 0;
	uint8_t repeat = 0;
	word_t data;
	word_t next;

	next.byte[0] = CMD_END;

	while (1) {
		if (next.byte[0] == CMD_END) {
			ptr = 0;
			repeat = 0;

			next.word = read(ptr);
			ptr++;

			// special case for empty program
			if (next.byte[0] == CMD_END) {
				ledon();
				_delay_ms(1000);
				ledoff();
				_delay_ms(1000);
				continue;
			}

			ledon();
			_delay_us(100); // commit delay
			ledoff();
		}

		if (repeat == 0) {
			data.word = next.word;
			next.word = read(ptr);
			ptr++;

			if (data.byte[0] == CMD_REPEAT) {
				repeat = data.byte[1];
				if (repeat == 0) repeat++;
				data.word = next.word;
			}
		} else {
			if (repeat == 1) {
				next.word = read(ptr);
				ptr++;
			}
			repeat--;
		}

		if ((data.byte[0] & MASK_DELAY) == 1) {
			const uint16_t delay = data.word >> 2;
			for (uint16_t i = 0; i < delay; ++i) _delay_ms(2);
		} else {
			send(data.word & _BV(15));
			send(data.word & _BV(14));
			send(data.word & _BV(13));
			send(data.word & _BV(12));
			send(data.word & _BV(11));
			send(0);
			ZERO_BIT_DELAY; send(0);
			ZERO_BIT_DELAY; send(0);

			send(data.word & _BV(10));
			send(data.word & _BV(9));
			send(data.word & _BV(8));
			send(data.word & _BV(7));
			send(data.word & _BV(6));
			send(0);
			ZERO_BIT_DELAY; send(0);
			ZERO_BIT_DELAY; send(0);

			send(data.word & _BV(5));
			send(data.word & _BV(4));
			send(data.word & _BV(3));
			send(data.word & _BV(2));
			send(data.word & _BV(1));
			send(0);
			ZERO_BIT_DELAY; send(0);
			ZERO_BIT_DELAY; send(0);
		}
	}
}
