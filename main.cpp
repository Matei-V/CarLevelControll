#include "src/comm.hpp"
#include <avr/io.h>


int main() {
  UART serial_usb;
  SPI gy_91;

  gy_91.spi_write_register(0x6B, 0x01);
  _delay_ms(100);
  gy_91.spi_write_register(0x1C, 0x00);

  while (true) {
    uint8_t high = gy_91.spi_read_register(0x3B);
    uint8_t low = gy_91.spi_read_register(0x3C);
    int16_t x_acceleration = (int16_t)(((uint16_t)high << 8) | low);

    serial_usb.write_sample(x_acceleration);
    _delay_ms(50);
  }
  
}


