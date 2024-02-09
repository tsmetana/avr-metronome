CC=avr-gcc
MCU=attiny13
CFLAGS=-Wall -Os -mmcu=$(MCU) --param=min-pagesize=0
PROGRAMMER=usbasp
PGM_PORT=usb

.PHONY: clean flash

all: main.hex

main.hex: main.elf
	-rm -f main.hex
	avr-objcopy -j .text -j .data -O ihex main.elf main.hex
	avr-size --format=avr --mcu=$(MCU) main.elf

main.elf: main.o
	$(CC) $(CFLAGS) -o main.elf main.o

main.o: main.c
	$(CC) $(CFLAGS) -c main.c -o main.o

clean:
	-rm -f main.hex main.elf main.o

flash:
	sudo avrdude -c $(PROGRAMMER) -P $(PGM_PORT) -p $(MCU) -U flash:w:main.hex:i
