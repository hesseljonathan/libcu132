#include "libcu132/libcu132.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libserialport.h>

#define CMD_INIT        0x22
#define CMD_VERSION     0x30
#define CMD_TERMINATOR  0x24

#define MAX_RLEN 18
typedef unsigned char CU132_RESPONSE[MAX_RLEN];

struct CU132 {
    char *serial_fd;
    CU132_RESPONSE status;
    CU_STATUS_CALLBACK_T callback_status;
    CU_DATA_CALLBACK_T callback_data;
    
};

// Function to simulate writing to the serial port (stdout)
CU_RESULT ser_write(unsigned char code) {
    int bytes_written = fwrite(&code, 1, 1, stdout);
    if (bytes_written != 1) {
        perror("Error writing to stdout");
        return ERROR;
    }
    fflush(stdout);
    return SUCCESS;
}

// Function to simulate reading from the serial port (stdin)
CU_RESULT ser_read(CU132_RESPONSE data, int nbyte) {
    int bytes_read = fread(data, 1, nbyte, stdin);
    if (bytes_read != nbyte) {
        perror("Error reading from stdin");
        return ERROR;
    }
    return SUCCESS;
}

CU_RESULT ser_read_until(CU132_RESPONSE data, int nbyte, unsigned char terminator) {
    int cursor = 0;
    while (cursor < nbyte) {
        char current;
        int bytes_read = fread(&current, 1, 1, stdin);
        if (bytes_read == 0) {
            // End of input or error
            if (feof(stdin)) {
                break;  // End of input
            } else if (ferror(stdin)) {
                perror("Error reading from stdin");
                return ERROR;
            }
        }
        data[cursor] = current;
        cursor += 1;
        if (current == terminator) break; //Stop at specified stop sign
    }
    return SUCCESS;
}

CU_RESULT ser_request(CU132_RESPONSE data, unsigned char cmd) {
    CU_RESULT result;
    CU132_RESPONSE response; //Response buffer is 18 chars maximum
    result = ser_write(CMD_INIT); //The " (0x22) signals a command to follow
    result = ser_write(cmd);
    result = ser_read(response, 1);
    if (result != SUCCESS) return ERROR;
    if (response[0] == cmd) { //Command gets confirmed, signals there is data to come
        ser_read_until(response, MAX_RLEN, CMD_TERMINATOR); //The $ (0x24) signals end of response
        memcpy(data, response, MAX_RLEN);

    } else return ERROR; //The # (0x23) signals unknown command

    return SUCCESS;
}

void debug_repr(CU132_RESPONSE buffer) {
    for (int i = 0; i < MAX_RLEN; i++)
    {
        fprintf(stderr, "%02X ", buffer[i]);
        if (buffer[i] == CMD_TERMINATOR) break;
    }
}

bool sanity_check(CU132_RESPONSE response) {
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

void process_status(CU132 *device, CU132_RESPONSE response) {
    if (device->callback_status == NULL) return;
    sanity_check(response);
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
    device->callback_status(status);
}

void process_data(CU132 *device, CU132_RESPONSE response) {
    if (device->callback_data == NULL) return;
    sanity_check(response);
    unsigned char id = response[0] & 0xf;
    unsigned int timestamp = 0;
    timestamp = timestamp | ((response[7] & 0xf) << 0);
    timestamp = timestamp | ((response[8] & 0xf) << 4);
    timestamp = timestamp | ((response[5] & 0xf) << 8);
    timestamp = timestamp | ((response[6] & 0xf) << 12);
    timestamp = timestamp | ((response[3] & 0xf) << 16);
    timestamp = timestamp | ((response[4] & 0xf) << 20);
    timestamp = timestamp | ((response[1] & 0xf) << 24);
    timestamp = timestamp | ((response[2] & 0xf) << 28);
    unsigned char sensor = response[9] & 0xf;
    device->callback_data(id, timestamp, sensor);
}

CU_RESULT cu132_init(CU132 **device, char *serial_fd) {
    CU132 *mem = malloc(sizeof(CU132));
    if (mem == NULL) return ERROR;
    memset(mem, 0, sizeof(CU132));
    *device = mem;
    struct sp_port *port;
    enum sp_return result = sp_get_port_by_name(serial_fd, &port);
    if (result != SP_OK) return ERROR;
    sp_set_baudrate(port, 19200);
    sp_set_bits(port, 8);
    sp_set_parity(port, SP_PARITY_NONE);
    sp_set_stopbits(port, 1);
    sp_set_flowcontrol(port, SP_FLOWCONTROL_NONE);
    result = sp_open(port, SP_MODE_READ_WRITE);
    if (result != SP_OK) return ERROR;
    return SUCCESS;
}

void cu132_destroy(CU132 *device) {
    if (device == NULL) return;
    free(device);
    device = NULL;
}

void cu132_register_status_callback(CU132 *device, CU_STATUS_CALLBACK_T callback) {
    device->callback_status = callback;
}
void cu132_register_data_callback(CU132 *device, CU_DATA_CALLBACK_T callback) {
    device->callback_data = callback;
}

CU_RESULT cu132_poll(CU132 *device) {
    CU_RESULT result;
    CU132_RESPONSE response;
    result = ser_request(response, 0x3f); //The ? (0x3f) is the poll command
    if (result != SUCCESS) return ERROR;
    if (response[0] == 0x3a) { //The : (0x3a) signals a status response
        if (memcmp(device->status, response, MAX_RLEN) != 0) //Status changed
        {
            memcpy(device->status, response, MAX_RLEN);
            process_status(device, response);
        }
    } else { //The only other option is a data response
        process_data(device, response);
    }
    return SUCCESS;
}

CU_RESULT cu132_get_version(CU132 *device, int *version) {
    CU_RESULT result;
    CU132_RESPONSE response;
    result = ser_request(response, CMD_VERSION); //The 0 (0x30) is the version command
    if (result != SUCCESS) return ERROR;
    sanity_check(response);
    int num = atoi((char*) response);
    if (num == 0) return ERROR;
    *version = num;
    return SUCCESS;
}