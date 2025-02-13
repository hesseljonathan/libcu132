#include "libcu132/libcu132.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libserialport.h>

//Magic values (commands) for the CU
#define CMD_INIT        0x22
#define CMD_VERSION     0x30
#define CMD_TERMINATOR  0x24
#define CMD_POLL        0x3f
#define CMD_RSTATUS     0x3a

#define CU_TIMEOUT      1000

//The raw response will only be 18 bytes or less so we use a fixed buffer here
#define MAX_RLEN        18
typedef unsigned char CU_RESPONSE[MAX_RLEN];

//Wrapper struct for to keep track of references
struct CU132 {
    struct sp_port *serial_port;
    bool connected;
};

//Write 1 byte to the CU (blocking)
CU_RESULT cu_write(CU132 *device, const unsigned char code) {
    if (device == NULL) return ERROR;
    enum sp_return result = sp_blocking_write(device->serial_port, &code, 1, CU_TIMEOUT);
    if (result != 1) return ERROR;
    return SUCCESS;
}

//Read 1 byte from the CU into data (blocking)
CU_RESULT cu_read(CU132 *device, unsigned char *data) {
    if (device == NULL) return ERROR;
    enum sp_return result = sp_blocking_read(device->serial_port, data, 1, CU_TIMEOUT);
    if (result != 1) return ERROR;
    return SUCCESS;
}

//Read bytes from the CU into data until terminator is encountered (blocking)
CU_RESULT cu_read_until(CU132 *device, CU_RESPONSE data, const unsigned char terminator) {
    if (device == NULL) return ERROR;
    int cursor = 0;
    while (cursor < MAX_RLEN) {
        unsigned char current;
        CU_RESULT result = cu_read(device, &current);
        if (result != SUCCESS) return ERROR;
        data[cursor] = current;
        cursor += 1;
        if (current == terminator) break; //Stop at specified stop sign
    }
    return SUCCESS;
}


//Writes and reads from the CU according to the underlying protocol (blocking)
CU_RESULT cu_request(CU132 *device, CU_RESPONSE data, const unsigned char cmd) {
    CU_RESULT result;
    CU_RESPONSE response; //Response buffer is 18 chars maximum
    result = cu_write(device, CMD_INIT); //The " (0x22) signals a command to follow
    result = cu_write(device, cmd);
    result = cu_read(device, response);
    if (result != SUCCESS) return ERROR;
    if (response[0] == cmd) { //Command gets confirmed, signals there is data to come
        cu_read_until(device, response, CMD_TERMINATOR); //The $ (0x24) signals end of response
        memcpy(data, response, MAX_RLEN);

    } else return ERROR; //The # (0x23) signals unknown command

    return SUCCESS;
}

void debug_repr(CU_RESPONSE buffer) {
    for (int i = 0; i < MAX_RLEN; i++)
    {
        fprintf(stderr, "%02X ", buffer[i]);
        if (buffer[i] == CMD_TERMINATOR) break;
    }
}

//Verifies the checksum of a given raw response
bool sanity_check(const CU_RESPONSE response) {
    unsigned char calculated = 0;
    unsigned char last = 0;
    int length = MAX_RLEN;
    for (int i = 0; i < MAX_RLEN; i++) {
        if (response[i] == CMD_TERMINATOR) {
            length = i;
            break;
        }
        calculated += last;
        calculated = calculated & 0xf;
        calculated = calculated | 0x30;
        last = response[i];
    }
    if (length == 0 || length == MAX_RLEN) return false;
    if (calculated != last) return false;
    return true;
}

//Interprets a raw response as a structured status response
CU_STATUS cu_process_status(const CU_RESPONSE response) {
    CU_STATUS status;
    status.fuel_levels[0] = response[1] & 0xf;
    status.fuel_levels[1] = response[2] & 0xf;
    status.fuel_levels[2] = response[3] & 0xf;
    status.fuel_levels[3] = response[4] & 0xf;
    status.fuel_levels[4] = response[5] & 0xf;
    status.fuel_levels[5] = response[6] & 0xf;
    status.start_light = response[9] & 0xf;
    status.fuel_mode = response[10] & 0x03;
    status.pitlane = (response[10] & 0x4) != 0;
    status.lapcounter = (response[10] & 0x8) != 0;
    status.cars_in_pit = (response[11] & 0xf) | ((response[12] & 0xf) << 4);
    return status;
}

//Interprets a raw response as a structured sensor response
CU_SENSOR cu_process_data(CU_RESPONSE response) {
    CU_SENSOR sensor;
    sensor.id = response[0] & 0xf;
    unsigned int timestamp = 0;
    timestamp = timestamp | ((response[7] & 0xf) << 0);
    timestamp = timestamp | ((response[8] & 0xf) << 4);
    timestamp = timestamp | ((response[5] & 0xf) << 8);
    timestamp = timestamp | ((response[6] & 0xf) << 12);
    timestamp = timestamp | ((response[3] & 0xf) << 16);
    timestamp = timestamp | ((response[4] & 0xf) << 20);
    timestamp = timestamp | ((response[1] & 0xf) << 24);
    timestamp = timestamp | ((response[2] & 0xf) << 28);
    sensor.timestamp = timestamp;
    sensor.sensor_id = response[9] & 0xf;
    return sensor;
}

//Allocates and initiates a CU wrapper struct and stores reference into device
CU_RESULT cu_init(CU132 **device) {
    if (device == NULL) return ERROR;
    *device = malloc(sizeof(CU132));
    if (*device == NULL) return ERROR;
    memset(*device, 0, sizeof(CU132));
    return SUCCESS;
}

//Opens the serial port on port_name
CU_RESULT cu_connect(CU132 *device, const char *port_name) {
    if (device == NULL) return ERROR;
    enum sp_return result = sp_get_port_by_name(port_name, &device->serial_port);
    if (result != SP_OK) return ERROR;
    sp_set_baudrate(device->serial_port, 19200);
    sp_set_bits(device->serial_port, 8);
    sp_set_parity(device->serial_port, SP_PARITY_NONE);
    sp_set_stopbits(device->serial_port, 1);
    sp_set_flowcontrol(device->serial_port, SP_FLOWCONTROL_NONE);
    result = sp_open(device->serial_port, SP_MODE_READ_WRITE);
    if (result != SP_OK) return ERROR;
    device->connected = true;
    return SUCCESS;
}

//Closes the serial port and frees and the given CU wrapper
void cu_destroy(CU132 **device) {
    if (device == NULL || *device == NULL) return;
    if ((*device)->serial_port != NULL) {
        sp_close((*device)->serial_port);
        sp_free_port((*device)->serial_port);
    }
    free(*device);
    *device = NULL;
}

//Polls the CU as specified in the underlying protocol and stores the structured response (blocking)
CU_RESULT cu_poll(CU132 *device, CU_POLL_RESPONSE *response) {
    CU_RESULT result;
    CU_RESPONSE raw;
    result = cu_request(device, raw, CMD_POLL); //The ? (0x3f) is the poll command
    if (result != SUCCESS) return ERROR;
    if (raw[0] == CMD_RSTATUS) { //The : (0x3a) signals a status response
        response->type = RESPONSE_STATUS;
        response->data.status = cu_process_status(raw);
    } else { //The only other option is a data response
        response->type = RESPONSE_SENSOR;
        response->data.sensor = cu_process_data(raw);
    }
    return SUCCESS;
}

//Requests the firmware version of the CU (blocking)
CU_RESULT cu_get_version(CU132 *device, int *version) {
    CU_RESULT result;
    CU_RESPONSE raw;
    result = cu_request(device, raw, CMD_VERSION); //The 0 (0x30) is the version command
    if (result != SUCCESS) return ERROR;
    sanity_check(raw);
    int num = atoi((char*) raw);
    if (num == 0) return ERROR;
    *version = num;
    return SUCCESS;
}
