#ifndef LIBCU132_H
#define LIBCU132_H
#include <stdbool.h>

typedef enum {
    SUCCESS = 0,
    ERROR = 1
} CU_RESULT;

typedef struct CU132 CU132;

typedef struct {
    unsigned char fuel_levels[6];
    unsigned char start_light;
    unsigned char fuel_mode;
    bool pitlane;
    bool lapcounter;
    unsigned char cars_in_pit;
} CU_STATUS;

typedef void (*CU_STATUS_CALLBACK_T)(CU_STATUS status);
typedef void (*CU_DATA_CALLBACK_T)(unsigned char id, unsigned int timestamp, unsigned char sensor);

CU_RESULT cu132_init(CU132 **device, char *serial_fd);
void cu132_destroy(CU132 *device);

void cu132_register_status_callback(CU132 *device, CU_STATUS_CALLBACK_T callback);
void cu132_register_data_callback(CU132 *device, CU_DATA_CALLBACK_T callback);

CU_RESULT cu132_poll(CU132 *device);
CU_RESULT cu132_get_version(CU132 *device, int *version);

#endif