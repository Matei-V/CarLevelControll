#include <cerrno>
#include <cstring>
#include <fcntl.h>
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
        if (frame.size() < 5) {
          break;
        }

        const unsigned char high = frame[2];
        const unsigned char low = frame[3];
        if (frame[4] == static_cast<unsigned char>(high ^ low)) {
          const unsigned int raw_value =
              (static_cast<unsigned int>(high) << 8) | low;
          const int value = raw_value >= 0x8000 ? raw_value - 0x10000
                                                : raw_value;
          std::cout << "X: " << value << '\n' << std::flush;
          frame.erase(frame.begin(), frame.begin() + 5);
        } else {
          frame.erase(frame.begin());
        }
      }
    }
  }
}