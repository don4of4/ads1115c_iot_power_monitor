// Created by Don Scott for a Raspberry Pi Power Monitor with YMDC SCT013

// Thank you to Lewis Loflin lewis@bvu.net for the basis of this code.
// http://www.bristolwatch.com/rpi/ads1115.html

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>    // read/write usleep
#include <stdlib.h>    // exit function
#include <inttypes.h>  // uint8_t, etc
#include <linux/i2c-dev.h> // I2C bus definitions

#include "../includes/ads1115c.h"

// Connect ADDR to GRD.
// Setup to use ADC0 single ended
// Note PCF8591 defaults to 0x48!
const int asd_address = 0x48;

// The state of the device
enum device_state {Closed, Open, Ready} device_state;
int handle;
uint8_t writeBuf[3];
uint8_t readBuf[2];
const float VPS = 4.096 / 32768.0; // volts per step

/*
The resolution of the ADC in single ended mode 
we have 15 bit rather than 16 bit resolution, 
the 16th bit being the sign of the differential 
reading.
*/

void openHandle(){

  // open device on /dev/i2c-1 the default on Raspberry Pi B
  if ((handle = open("/dev/i2c-1", O_RDWR)) < 0) {
    printf("Error: Couldn't open device! %d\n", handle);
    exit (1);
  }

  // connect to ADS1115 as i2c slave
  if (ioctl(handle, I2C_SLAVE, asd_address) < 0) {
    printf("Error: Couldn't find device on address!\n");
    exit (1);
  }

  device_state = Open;

}

void initializeDevice(){
  if (device_state != Open){
     perror("initializeDevice: not Open");
  }
  // set config register and start conversion
  // AIN0 and GND, 4.096v, 128s/s
  // Refer to page 19 area of spec sheet
  writeBuf[0] = 1; // config register is 1
  writeBuf[1] = 0b11000010; // 0xC2 single shot off
  // Don:  Is the 100 input selector one bit off?
  // bit 15 flag bit for single shot not used here

  // Bits 14-12 input selection:
  // 100 ANC0; 101 ANC1; 110 ANC2; 111 ANC3
  // Bits 11-9 Amp gain. Default to 010 here 001 P19
  // Bit 8 Operational mode of the ADS1115.
  // 0 : Continuous conversion mode
  // 1 : Power-down single-shot mode (default)

  writeBuf[2] = 0b10000101; // bits 7-0  0x85
  // Bits 7-5 data rate default to 100 for 128SPS
  // Bits 4-0  comparator functions see spec sheet.

  // begin conversion
  if (write(handle, writeBuf, 3) != 3) {
    perror("Write to register 1");
    exit (1);
  }

  sleep(1);

  // set read pointer to 0
  readBuf[0] = 0;
  if (write(handle, readBuf, 1) != 1) {
    perror("Write register select");
    exit(-1);
  }

  device_state = Ready;
}

int16_t readDevice(){
    int16_t val;

    // read conversion register
    if (read(handle, readBuf, 2) != 2) {
      perror("Read conversion");
      exit(-1);
    }

    // could also multiply by 256 then add readBuf[1]
    val = readBuf[0] << 8 | readBuf[1];

    // with +- LSB sometimes generates very low neg number.
    if (val < 0)  val = 0;
    
    return val;
}

float readDeviceAmps(){
  int16_t val = readDevice(handle);

  float voltage = val * VPS; // convert to voltage
  float amps = voltage * 5.78704677428;

  return amps;
}

void closeHandle(){
  // power down ASD1115
  writeBuf[0] = 1;    // config register is 1
  writeBuf[1] = 0b11000011; // bit 15-8 0xC3 single shot on
  writeBuf[2] = 0b10000101; // bits 7-0  0x85

  if (write(handle, writeBuf, 3) != 3) {
    perror("Write to register 1");
    exit (1);
  }

  close(handle);
  device_state = Closed;
}
