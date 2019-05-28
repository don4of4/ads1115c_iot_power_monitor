// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// CAVEAT: This sample is to demonstrate azure IoT client concepts only and is not a guide design principles or style
// Checking of return codes and error values shall be omitted for brevity.  Please practice sound engineering practices
// when writing production code.

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

#include "iothub.h"
#include "iothub_device_client.h"
#include "iothub_client_options.h"
#include "iothubtransportmqtt.h"
#include "iothub_message.h"

#include "azure_c_shared_utility/threadapi.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/shared_util_options.h"

#define SAMPLE_MQTT

static bool g_continueRunning = true;
static size_t g_message_count_send_confirmations = 0;
static IOTHUB_DEVICE_CLIENT_HANDLE g_device_handle;
static IOTHUB_MESSAGE_HANDLE g_message_handle;
static size_t g_messages_sent = 0;



static void send_confirm_callback(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void *userContextCallback) {
    (void) userContextCallback;
    // When a message is sent this callback will get envoked
    g_message_count_send_confirmations++;
    (void) printf("Confirmation callback received for message %lu with result %s\r\n",
                  (unsigned long) g_message_count_send_confirmations,
                  MU_ENUM_TO_STRING(IOTHUB_CLIENT_CONFIRMATION_RESULT, result));
}

static void
connection_status_callback(IOTHUB_CLIENT_CONNECTION_STATUS result, IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason,
                           void *user_context) {
    (void) reason;
    (void) user_context;
    // This sample DOES NOT take into consideration network outages.
    if (result == IOTHUB_CLIENT_CONNECTION_AUTHENTICATED) {
        (void) printf("The device client is connected to iothub\r\n");
    } else {
        (void) printf("The device client has been disconnected\r\n");
    }
}

void init_iotclient(const char *connectionString){
    // Used to initialize IoTHub SDK subsystem
    (void) IoTHub_Init();

    (void) printf("Creating IoTHub Device handle\r\n");

    // Create the iothub handle here
    g_device_handle = IoTHubDeviceClient_CreateFromConnectionString(connectionString, MQTT_Protocol);

    if (g_device_handle == NULL) {
        (void) printf("Failure creating Iothub device.  Hint: Check you connection string.\r\n");
        return;
    }

    // Consider Url encoding
    //bool urlEncodeOn = true;
    //IoTHubDeviceClient_SetOption(g_device_handle, OPTION_AUTO_URL_ENCODE_DECODE, &urlEncodeOn);


    // Setting connection status callback to get indication of connection to iothub
    (void) IoTHubDeviceClient_SetConnectionStatusCallback(g_device_handle, connection_status_callback, NULL);
}

void cleanup_iotclient(){

    // Clean up the iothub sdk handle
    IoTHubDeviceClient_Destroy(g_device_handle);

    // Free all the sdk subsystem
    IoTHub_Deinit();
}

void send_message(const char *msg_text){
    g_message_handle = IoTHubMessage_CreateFromString(msg_text);

    //message_handle = IoTHubMessage_CreateFromByteArray((const unsigned char*)msgText, strlen(msgText)));

    // Set Message properties
    // Header information, encoding,
    (void) IoTHubMessage_SetContentTypeSystemProperty(g_message_handle, "application%2fjson");
    (void) IoTHubMessage_SetContentEncodingSystemProperty(g_message_handle, "utf-8");

    // Add custom properties to message
    (void) IoTHubMessage_SetProperty(g_message_handle, "deviceId", "1");

    (void) printf("Sending message %d to IoTHub\r\n", (int) (g_messages_sent + 1));
    IoTHubDeviceClient_SendEventAsync(g_device_handle, g_message_handle, send_confirm_callback, NULL);

    // The message is copied to the sdk so the we can destroy it
    IoTHubMessage_Destroy(g_message_handle);

    g_messages_sent++;
}

int sendsin(void) {

    init_iotclient();

    // buffer
    size_t msg_length = 1024;
    const char* msg_text = malloc(msg_length);

    float amperage;

    do {
        amperage = 4.0 * sin(g_messages_sent / 100.0) + 5.0;
        // Construct the iothub message from a string or a byte array
        snprintf((char *) msg_text, msg_length, "{\"Amperage\":%lf}", amperage);

        send_message(msg_text);

        ThreadAPI_Sleep(100);

    } while (g_continueRunning);

    cleanup_iotclient();

    return 0;
}
