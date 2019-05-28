#ifndef dscott_ads1115c
#define dscott_ads1115c

#include <stdint.h>

const float VPS;

void openHandle();
void initializeDevice();
int16_t readDevice();
float readDeviceAmps();
void closeHandle();

#endif
