cmake_minimum_required(VERSION 3.7)
project(ads1115c_power_monitor C)

set(CMAKE_C_STANDARD 99)

include_directories(includes)

# Include the required azure iot files
include_directories(/usr/local/include
    "/usr/local/include/azureiot/"
    "/usr/local/include/azureiot/inc/"
    "/usr/local/include/azure_c_shared_utility")

#However, the file(GLOB...) allows for wildcard additions:
file(GLOB SOURCES "implementation/*.c")
file(GLOB PARSON "includes/parson.h" "includes/parson.c")

add_executable(ads1115c_power_monitor main.c ${SOURCES} ${PARSON} includes/iotclient.h)
target_link_libraries(ads1115c_power_monitor
        wiringPi
        serializer
        iothub_client
        iothub_client_mqtt_transport
        umqtt
        aziotsharedutil
        ssl
        crypto
        curl
        pthread
        m
        ssl
        crypto)


