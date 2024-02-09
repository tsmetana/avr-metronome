// 9.6 MHz, built in oscillator
#define F_CPU 9600000
 
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

uint8_t beeps;


void pin_setup(void)
{
    /* Output at PB1: beeper 
       Output at PB0: LED */
    DDRB |= (1 << PB1) | (1 << PB0);  

    /* Input at PB3 */
    DDRB &= ~(1 << DDB3);
    
    /* Input at PB4 */
    DDRB &= ~(1 << DDB4);

    /* Allow interrupts on PB3 and PB4 */
    PCMSK |= (1 << PCINT4) | (1 << PCINT3);

    /* Interrupt on raising edge */
    MCUCR |= (1 << ISC01) | (1 << ISC00);

    /* Enable pin change interrupts */
    GIMSK |= (1 << PCIE);
}


void adc_setup(void)
{
    /* Set the ADC input to PB2 */
    ADMUX |= (1 << MUX0);
    ADMUX |= (1 << ADLAR);
 
    /* ADC prescaler to clock/8 */
    ADCSRA |= (1 << ADPS1) | (1 << ADPS0);

    /* Enable ADC */
    ADCSRA |= (1 << ADEN);
}


void pwm_setup(void)
{
	/* Timer0 prescaler to clock/8
	   which gives approx. 1.3kHz with 9.6MHz clock */
    TCCR0B |=  (0 << CS02) | (1 << CS01) | (0 << CS00);
 
    /* Fast PWM mode */
    TCCR0A |= (1 << WGM01) | (1 << WGM00);
 
    /* Clear OC0B on Compare Match */
    TCCR0A |= (1 << COM0B1);
 
    /* PWM off */
    OCR0B = 255;
}


int adc_read(void)
{
    /* Start the conversion */
    ADCSRA |= (1 << ADSC);
 
    /* Wait till it's done */
    while (ADCSRA & (1 << ADSC));

    /* We care only about the 8 bits in ADCH */
    return ADCH;
}


ISR(PCINT0_vect)
{
    if (!(PINB & (1 << PINB3))) {
        /* Button on PB3 pressed */
        ++beeps;
    } else if ((!(PINB & (1 << PINB4))) && beeps > 0) {
        /* Button on PB4 pressed */
        --beeps;
    }

}


#define MIN_WAIT_MS   1
#define PWM_OFF     255
#define PWM_50      128
#define BEEP_LONG     6
#define BEEP_SHORT    3
#define DEFAULT_BEEPS 4

int main(void)
{
    uint16_t adc_in;
    uint16_t counter;
    uint8_t beep_count = 0;

    pin_setup();
    adc_setup();
    pwm_setup();

    beeps = DEFAULT_BEEPS;

    /* Enable interrupts */
    sei();

    while (1) {
        adc_in = 0;
        /* 10 samples are still inaccurate but not noticeable at least */
        for (counter = 0; counter < 10; counter++) {
	    	adc_in += adc_read();
        }
        /* Magic: this should be /10 but to compensate for nonlinearity
           of the potentiometer, let's divide it a bit more... */
        counter = (adc_in / 15) + 5;
        while (counter-- != 0) {
            /* _delay_ms() doesn't do what one would expect
               but this is good enough */
            _delay_ms(MIN_WAIT_MS);
        }

        /* Start beeping and flash LED */
        OCR0B = PWM_50;
        PORTB |= (1 << PB0);

        /* Make the first beep in the series longer, mind the 
           beep_count can be changed from the interrupt handler */
        if (beep_count++ == 0) {
            _delay_ms(BEEP_LONG);
        } else {
            _delay_ms(BEEP_SHORT);
            if (beep_count >= beeps) {
                beep_count = 0;
            }
        }

		/* PWM off: stop beeping and turn LED off */
        OCR0B = PWM_OFF;
        PORTB &= ~(1 << PB0);
    }
}