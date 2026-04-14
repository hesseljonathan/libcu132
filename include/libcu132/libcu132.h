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
} cu_result_t;

typedef struct cu132_t cu132_t;

typedef struct {
    unsigned char fuel_levels[6];
    unsigned char start_light;
    unsigned char fuel_mode;
    bool pitlane;
    bool lapcounter;
    unsigned char cars_in_pit;
} cu_status_t;

typedef struct {
    unsigned char id;
    unsigned int timestamp;
    unsigned char sensor_id;
} cu_sensor_t;

typedef enum {
    RESPONSE_SENSOR,
    RESPONSE_STATUS
} cu_poll_response_type_t;

typedef union {
    cu_status_t status;
    cu_sensor_t sensor;
} cu_poll_response_data_t;

//Wrapper for the structured response (tagged union)
typedef struct {
    cu_poll_response_type_t type;
    cu_poll_response_data_t data;
} cu_poll_response_t;

LIBCU132_API cu_result_t cu_init(cu132_t **device);
LIBCU132_API cu_result_t cu_connect(cu132_t *device, const char *port_name);
LIBCU132_API void cu_destroy(cu132_t *device);

LIBCU132_API cu_result_t cu_poll(cu132_t *device, cu_poll_response_t *response);
LIBCU132_API cu_result_t cu_get_version(cu132_t *device, int *version);

#endif
