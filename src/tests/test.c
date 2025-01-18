#include "libcu132/libcu132.h"
#include <stdio.h>

void test_cb_data(unsigned char id, unsigned int timestamp, unsigned char sensor) {
    fprintf(stderr, "%s\nid=%u, time=%u, sensor=%u\n", "DATA UPDATE", id, timestamp, sensor);
}

void test_cb_status(CU_STATUS status) {
    fprintf(stderr, "%s\n", "STATUS_UPDATE");
    fprintf(stderr, "fuel: %u, %u, %u, %u, %u, %u\n", status.fuel_levels[0], status.fuel_levels[1], status.fuel_levels[2], status.fuel_levels[3], status.fuel_levels[4], status.fuel_levels[5]);
    fprintf(stderr, "lights=%u, fuelmode=%u, pl=%u, lc=%u, inpit=%00x\n", status.start_light, status.fuel_mode, status.pitlane, status.lapcounter, status.cars_in_pit);
}

int main() {
    fprintf(stderr, "%s", "Starting test...\n");
    CU132 *device;
    CU_RESULT result = cu132_init(&device, "/dev/ttyUSB0");
    if (result != SUCCESS) {
        cu132_destroy(device);
        return 1;
    }

    cu132_register_data_callback(device, *test_cb_data);
    cu132_register_status_callback(device, *test_cb_status);

    while (true) {
        result = cu132_poll(device);
        if (result != SUCCESS)
            fprintf(stderr, "%s", "error, continue\n");
    }

    cu132_destroy(device);
}