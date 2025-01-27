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

typedef struct {
    unsigned char id;
    unsigned int timestamp;
    unsigned char sensor_id;
} CU_SENSOR;

typedef enum {
    RESPONSE_SENSOR,
    RESPONSE_STATUS
} CU_POLL_RESPONSE_TYPE;

typedef union {
    CU_STATUS status;
    CU_SENSOR sensor;
} CU_POLL_RESPONSE_DATA;

typedef struct {
    CU_POLL_RESPONSE_TYPE type;
    CU_POLL_RESPONSE_DATA data;
} CU_POLL_RESPONSE;

LIBCU132_API CU_RESULT cu_init(CU132 **device);
LIBCU132_API CU_RESULT cu_connect(CU132 *device, char *serial_fd);
LIBCU132_API void cu_destroy(CU132 *device);

LIBCU132_API CU_RESULT cu_poll(CU132 *device, CU_POLL_RESPONSE *response);
LIBCU132_API CU_RESULT cu_get_version(CU132 *device, int *version);

#endif
