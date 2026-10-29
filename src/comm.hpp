#pragma once

#include "lib.hpp"

#define BAUD 115200UL
#define UBRR_UART ((F_CPU) / (8UL * BAUD) - 1)

class UART {
private:
  void uart_init(void);
  void uart_putchar(char c);
public:
  UART() {
    uart_init();    
  }

  void write_sample(int16_t value);
  void write_sample(int16_t accel_x, int16_t accel_z, int16_t gyro_y);
};

class SPI {
private: 
  void spi_masterInit();
public:
  SPI() {
    spi_masterInit();
  }
  uint8_t spi_read_byte();
  uint8_t spi_transfer(uint8_t);
  uint8_t spi_read_register(uint8_t address);
  void spi_write_register(uint8_t address, uint8_t value);
};
