#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "ads1115c.h"
#include "iotclient.h"

int main() {
    // Open a handle to the device
    openHandle();

    // Setup the ads1115c settings
    initializeDevice();

    // Setup the iot client
    const char* connectionString = getenv("IOTHUB_CONN_STRING");
    init_iotclient(connectionString);

    size_t msg_length = 1024;
    const char* msg_text = malloc(msg_length);
    int16_t val;
    float myfloat;

    while (1)   {
        val = readDevice();

        myfloat = val * VPS; // convert to voltage

        float amps = myfloat * 5.78704677428;
        float watts = amps * 120;

        printf("Conversion number %d HEX 0x%02x DEC %d %7.6f volts. Estimated amperage: %7.6f Estimated Wattage: %6.2f \n",
               val, val, myfloat, amps, watts);

        snprintf((char *) msg_text, msg_length, "{\"Amperage\":%lf}", amps);

        send_message(msg_text);

        // Sleep for 200ns
        usleep(1000 * 50);

    } // end while loop

    closeHandle();
    cleanup_iotclient();

    return 0;
}