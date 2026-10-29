#include "comm.hpp"

void UART::uart_init(void) {
    // Enable 2x speed for accurate 115200 baud rate at 16 MHz
    UCSR0A = (1 << U2X0);

    UBRR0H = (uint8_t)(UBRR_UART >> 8);
    UBRR0L = (uint8_t)(UBRR_UART);

    // Enable Transmitter
    UCSR0B = (1 << TXEN0);

    // 8 data bits, 1 stop bit, no parity (8N1)
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
  }

void UART::uart_putchar(char c) {
  while (!(UCSR0A & (1 << UDRE0))) {
    // Wait until transmit buffer is ready
  }
  UDR0 = c;
}

void UART::write_sample(int16_t value) {
  const uint16_t raw_value = (uint16_t)value;
  const uint8_t high = (uint8_t)(raw_value >> 8);
  const uint8_t low = (uint8_t)raw_value;

  uart_putchar((char)0xA5);
  uart_putchar((char)0x5A);
  uart_putchar((char)high);
  uart_putchar((char)low);
  uart_putchar((char)(high ^ low));
}

void UART::write_sample(int16_t accel_x, int16_t accel_z, int16_t gyro_y) {
  const uint16_t raw_ax = (uint16_t)accel_x;
  const uint8_t ax_h = (uint8_t)(raw_ax >> 8);
  const uint8_t ax_l = (uint8_t)raw_ax;

  const uint16_t raw_az = (uint16_t)accel_z;
  const uint8_t az_h = (uint8_t)(raw_az >> 8);
  const uint8_t az_l = (uint8_t)raw_az;

  const uint16_t raw_gy = (uint16_t)gyro_y;
  const uint8_t gy_h = (uint8_t)(raw_gy >> 8);
  const uint8_t gy_l = (uint8_t)raw_gy;

  const uint8_t checksum = ax_h ^ ax_l ^ az_h ^ az_l ^ gy_h ^ gy_l;

  uart_putchar((char)0xA5);
  uart_putchar((char)0x5A);
  uart_putchar((char)ax_h);
  uart_putchar((char)ax_l);
  uart_putchar((char)az_h);
  uart_putchar((char)az_l);
  uart_putchar((char)gy_h);
  uart_putchar((char)gy_l);
  uart_putchar((char)checksum);
}


void SPI::spi_masterInit() {
  DDRB = (1 << DDB5) | (1 << DDB3) | (1 << DDB2);
  PORTB |= (1 << PB2);
  SPCR = (1 << SPE) | (1 << MSTR) | (1 << CPOL) | (1 << CPHA) | (1 << SPR0);
}

uint8_t SPI::spi_transfer(uint8_t data) {
  SPDR = data;

  while (!(SPSR & (1 << SPIF)));

  return SPDR;
}

uint8_t SPI::spi_read_byte() {

  return SPDR;
}

uint8_t SPI::spi_read_register(uint8_t address) {
  PORTB &= ~(1 << PB2);
  spi_transfer(address | 0x80);
  uint8_t value = spi_transfer(0x00);
  PORTB |= (1 << PB2);
  return value;
}

void SPI::spi_write_register(uint8_t address, uint8_t value) {
  PORTB &= ~(1 << PB2);
  spi_transfer(address & 0x7F);
  spi_transfer(value);
  PORTB |= (1 << PB2);
}
