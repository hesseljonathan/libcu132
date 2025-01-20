#ifndef LIBCU132_H
#define LIBCU132_H
#include <stdbool.h>

#ifdef _WIN32
    #define LIBCU132_API __declspec(dllexport)
#else
    #define LIBCU132_API
#endif

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

LIBCU132_API CU_RESULT cu_init(CU132 **device);
LIBCU132_API CU_RESULT cu_connect(CU132 *device, char *serial_fd);
LIBCU132_API void cu_destroy(CU132 *device);

LIBCU132_API void cu_register_status_callback(CU132 *device, CU_STATUS_CALLBACK_T callback);
LIBCU132_API void cu_register_data_callback(CU132 *device, CU_DATA_CALLBACK_T callback);

LIBCU132_API CU_RESULT cu_poll(CU132 *device);
LIBCU132_API CU_RESULT cu_get_version(CU132 *device, int *version);

#endif
