#include <stdio.h>
#include <stdlib.h>

//#include <conio.h>
#include <dos.h>

#define TIME_REG_AMOUNT 6

typedef unsigned char byte;

enum time_registers {
  sec   = 0x00,
  min   = 0x02,
  hour  = 0x04,
  day   = 0x07,
  month = 0x08,
  year  = 0x09,
};

enum state_registers {
  A = 0x0A, // bit 7 - is time update, bit 4-6 - divider, 0-3 conversion value
  B = 0x0B,
  C = 0x0C,
  D = 0x0D,
};

unsigned   time[TIME_REG_AMOUNT];
const byte time_registers[TIME_REG_AMOUNT] = {sec, min, hour, day, month, year};

unsigned bcd_to_dec(unsigned bcd) { return (bcd / 16 * 10) + (bcd % 16); }
unsigned dec_to_bcd(unsigned dec) { return (dec / 10 * 16) + (dec % 10); }

void print_time(void) {
  int i;
  for (i = 0; i < TIME_REG_AMOUNT; ++i) {
    outp(0x70, A);
    // Don't read time while 7 bit is set
    if (inp(0x71) & 0x80) {
      i--;
      continue;
    }
    outp(0x70, time_registers[i]);
    time[i] = inp(0x71);
    time[i] = bcd_to_dec(time[i]);
  }

  printf("%02u:%02u:%02u %02u.%02u.20%02u\n",
         time[2], time[1], time[0], time[3], time[4], time[5]);
}

void set_time(void) {
  int i;

  puts("Enter time in format hours:min:sec");
  if (scanf("%u:%u:%u", &time[2], &time[1], &time[0]) != 3) {
    fprintf(stderr, "Failed to enter time\n");
  }
  puts("Enter date in format day.month.year");
  if (scanf("%u.%u.%u", &time[3], &time[4], &time[5]) != 3) {
    fprintf(stderr, "Failed to enter date\n");
  }

  for (i = 0; i < TIME_REG_AMOUNT; ++i) {
    time[i] = dec_to_bcd(time[i]);
  }

  disable();

  // Don't read time while 7 bit is set
  do {
    outp(0x70, 0x0A);
  } while (inp(0x71) & 0x80);

  // Off clock refresh
  outp(0x70, 0x0B);
  outp(0x71, inp(0x71) | 0x80);

  for (i = 0; i < TIME_REG_AMOUNT; i++) {
    outp(0x70, time_registers[i]);
    outp(0x71, time[i]);
  }

  // On clock refresh
  outp(0x70, 0x0B);
  outp(0x71, inp(0x71) & 0x7F);

  enable();
}

int main(void) {
  while (1) {
    puts("1 - print time");
    puts("2 - set time");
    puts("0 - exit");

    switch (getchar()) {
      case '1':
        print_time();
        break;

      case '2':
        set_time();
        break;

      case '0':
        exit(EXIT_SUCCESS);

      default:
        break;
    }

    getchar();
  }
}
