#include "src/comm.hpp"
#include <avr/io.h>


int main() {
  UART serial_usb;
  SPI gy_91;

  gy_91.spi_write_register(0x6B, 0x01);
  _delay_ms(100);
  gy_91.spi_write_register(0x1C, 0x00);
  gy_91.spi_write_register(0x1B, 0x00);

  while (true) {
    uint8_t ax_high = gy_91.spi_read_register(0x3B);
    uint8_t ax_low = gy_91.spi_read_register(0x3C);
    int16_t x_acceleration = (int16_t)(((uint16_t)ax_high << 8) | ax_low);

    uint8_t az_high = gy_91.spi_read_register(0x3F);
    uint8_t az_low = gy_91.spi_read_register(0x40);
    int16_t z_acceleration = (int16_t)(((uint16_t)az_high << 8) | az_low);

    uint8_t gy_high = gy_91.spi_read_register(0x45);
    uint8_t gy_low = gy_91.spi_read_register(0x46);
    int16_t y_gyro = (int16_t)(((uint16_t)gy_high << 8) | gy_low);

    serial_usb.write_sample(x_acceleration, z_acceleration, y_gyro);
    _delay_ms(50);
  }
  
}


