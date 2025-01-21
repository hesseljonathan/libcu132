#include "libcu132/libcu132.h"
#include <stdio.h>
#include <stdlib.h>

void test_cb_data(unsigned char id, unsigned int timestamp, unsigned char sensor) {
    printf("%s\n", "DATA UPDATE\n");
    printf("id=%u, time=%u, sensor=%u\n", id, timestamp, sensor);
}

void test_cb_status(CU_STATUS status) {
    printf("%s\n", "STATUS_UPDATE\n");
    printf("fuel: %u, %u, %u, %u, %u, %u\n", status.fuel_levels[0], status.fuel_levels[1], status.fuel_levels[2], status.fuel_levels[3], status.fuel_levels[4], status.fuel_levels[5]);
    printf("lights=%u, fuelmode=%u, pl=%u, lc=%u, inpit=%00x\n", status.start_light, status.fuel_mode, status.pitlane, status.lapcounter, status.cars_in_pit);
}

void print_usage() {
    printf("%s\n", "Usage: moncu132 PORTNAME [TIMEOUT]");
    printf("%s\n", "  PORTNAME    The name of the serial port the CU is connected to");
    printf("%s\n", "  TIMEOUT     Timeout in seconds");
}

int main(int argc, char *argv[]) {
    printf("libcu132 0.0.1 monitor\n");
    if (argc < 2) {
        print_usage();
        return 0;
    }
    char *serial_fd = argv[1];
    int timeout = 0;
    if (argc > 2) {
        timeout = atoi(argv[2]);
    }
    printf("Connecting on %s\n", serial_fd);
    CU132 *device;
    CU_RESULT result = cu_init(&device);
    if (result != SUCCESS) {
        perror("Failed to allocate memory");
        cu_destroy(device);
        return 1;
    }
    result = cu_connect(device, serial_fd);
    if (result != SUCCESS) {
        cu_destroy(device);
        perror("Failed to connect to cu");
        return 1;
    }
    int version;
    result = cu_get_version(device, &version);
    if (result == SUCCESS) {
        printf("Found cu with firmware %u", version);
    }
    cu_register_data_callback(device, *test_cb_data);
    cu_register_status_callback(device, *test_cb_status);
    result = cu_poll(device);
    if (result != SUCCESS) {
        perror("Protocol error");
        return 1;
    }
    cu_destroy(device);
}
