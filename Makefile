default:
	avr-g++ -Os -DF_CPU=16000000UL -mmcu=atmega328p -c src/comm.cpp -o build/comm.o
	avr-g++ -Os -DF_CPU=16000000UL -mmcu=atmega328p -c main.cpp -o build/main.o
	avr-g++ -mmcu=atmega328p -o build/main.bin build/comm.o build/main.o
	avr-objcopy -O ihex build/main.bin build/main.hex
	sudo avrdude -F -V -c arduino -p ATMEGA328P -P /dev/ttyACM0 -b 115200 -U flash:w:build/main.hex

host: host.cpp
	g++ -std=c++17 -Wall -Wextra -O2 host.cpp -o build/host
