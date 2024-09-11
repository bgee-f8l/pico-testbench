#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "pico/stdio/driver.h"
#include "pico/stdio_usb.h"

void usb_printf(const char *format, ...) {
  char buffer[1000];
  va_list args;

  va_start(args, format);
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);

  // output over serial  
  stdio_usb.out_chars(buffer, strlen(buffer));
}