#include <cerrno>
#include <cstdint>
#include <cstring>
#include <fcntl.h>
#include <iomanip>
#include <iostream>
#include <string>
#include <termios.h>
#include <unistd.h>
#include <vector>

int main(int argc, char *argv[]) {
  const char *device = argc > 1 ? argv[1] : "/dev/ttyACM0";
  const int serial = open(device, O_RDONLY | O_NOCTTY);
  if (serial == -1) {
    std::cerr << "Could not open " << device << ": " << std::strerror(errno)
              << '\n';
    return 1;
  }

  termios settings{};
  if (tcgetattr(serial, &settings) == -1) {
    std::cerr << "Could not read serial settings: " << std::strerror(errno)
              << '\n';
    close(serial);
    return 1;
  }

  cfmakeraw(&settings);
  cfsetispeed(&settings, B115200);
  cfsetospeed(&settings, B115200);
  settings.c_cflag |= CLOCAL | CREAD;
  settings.c_cflag &= ~CSTOPB;
  settings.c_cflag &= ~PARENB;
  settings.c_cflag &= ~CSIZE;
  settings.c_cflag |= CS8;
  settings.c_cc[VMIN] = 1;
  settings.c_cc[VTIME] = 0;

  if (tcsetattr(serial, TCSANOW, &settings) == -1) {
    std::cerr << "Could not configure serial port: " << std::strerror(errno)
              << '\n';
    close(serial);
    return 1;
  }

  std::vector<unsigned char> frame;
  char buffer[128];
  while (true) {
    const ssize_t count = read(serial, buffer, sizeof(buffer));
    if (count == -1) {
      if (errno == EINTR) {
        continue;
      }
      std::cerr << "Serial read failed: " << std::strerror(errno) << '\n';
      close(serial);
      return 1;
    }
    if (count == 0) {
      continue;
    }

    for (ssize_t index = 0; index < count; ++index) {
      frame.push_back(static_cast<unsigned char>(buffer[index]));
      while (frame.size() >= 2) {
        if (frame[0] != 0xA5 || frame[1] != 0x5A) {
          frame.erase(frame.begin());
          continue;
        }
        if (frame.size() < 9) {
          break;
        }

        const unsigned char ax_h = frame[2];
        const unsigned char ax_l = frame[3];
        const unsigned char az_h = frame[4];
        const unsigned char az_l = frame[5];
        const unsigned char gy_h = frame[6];
        const unsigned char gy_l = frame[7];
        const unsigned char expected_crc =
            static_cast<unsigned char>(ax_h ^ ax_l ^ az_h ^ az_l ^ gy_h ^ gy_l);

        if (frame[8] == expected_crc) {
          auto to_int16 = [](unsigned char high, unsigned char low) -> int16_t {
            return static_cast<int16_t>(
                (static_cast<uint16_t>(high) << 8) | low);
          };

          const int16_t raw_ax = to_int16(ax_h, ax_l);
          const int16_t raw_az = to_int16(az_h, az_l);
          const int16_t raw_gy = to_int16(gy_h, gy_l);

          constexpr double accel_lsb_per_g = 16384.0; // MPU9250 ±2g range
          constexpr double gyro_lsb_per_dps = 131.0;  // MPU9250 ±250 °/s range

          const double accel_x_g = static_cast<double>(raw_ax) / accel_lsb_per_g;
          const double accel_z_g = static_cast<double>(raw_az) / accel_lsb_per_g;
          const double gyro_y_dps = static_cast<double>(raw_gy) / gyro_lsb_per_dps;

          std::cout << "Accel X: " << std::fixed << std::setprecision(4)
                    << accel_x_g << " g | "
                    << "Accel Z: " << accel_z_g << " g | "
                    << "Gyro Y: " << std::setprecision(2)
                    << gyro_y_dps << " deg/s\n"
                    << std::flush;
          frame.erase(frame.begin(), frame.begin() + 9);
        } else {
          frame.erase(frame.begin());
        }
      }
    }
  }
}
