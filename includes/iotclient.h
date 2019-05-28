//
// Created by me on 5/27/2019.
//

#ifndef ADS1115C_POWER_MONITOR_IOTCLIENT_H
#define ADS1115C_POWER_MONITOR_IOTCLIENT_H

void init_iotclient();
void cleanup_iotclient();
void send_message(char *msg_text);
int sendsin(void);

#endif //ADS1115C_POWER_MONITOR_IOTCLIENT_H
