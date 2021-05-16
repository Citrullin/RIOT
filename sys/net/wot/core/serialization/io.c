#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "net/wot.h"
#include "net/wot/serialization.h"

const char wot_td_ser_true[] = "true";
const char wot_td_ser_false[] = "false";

void _reverse(char s[])
{
    int i, j;

    for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
        char c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

void _itoa(int n, char s[])
{
    int i, sign;
    if ((sign = n) < 0)
        n = -n;
    i = 0;
    do {
        s[i++] = n % 10 + '0';
    } while ((n /= 10) > 0);
    if (sign < 0)
        s[i++] = '-';
    s[i] = '\0';
    _reverse(s);
}

//Todo: Validate Thing struct. Separate functions?

//Fixme: circuit breaker pattern? IPC message passing? Make it more elegant.
//Fixme: Not working with slicer->start.
//https://riot-os.org/api/group__core__msg.html
int _wot_io_send(wot_td_serialize_receiver_t receiver, const char *string, uint32_t length, wot_td_ser_slicer_t *slicer){
    for(uint32_t i = 0; i < length; i++){
        if(slicer->cur <= slicer->end){
            receiver(&string[i]);
        }

        slicer->cur += 1;
    }

    return 0;
}

//Todo: Clearer naming convention?
void _wot_io_send_string(wot_td_serialize_receiver_t receiver, const char *string, uint32_t length, wot_td_ser_slicer_t *slicer){
    _wot_io_send(receiver, "\"", 1, slicer);
    _wot_io_send(receiver, string, length, slicer);
    _wot_io_send(receiver, "\"", 1, slicer);
}

void _wot_io_send_uri(wot_td_serialize_receiver_t receiver, wot_td_uri_t *uri, wot_td_ser_slicer_t *slicer){
    _wot_io_send(receiver, "\"", 1, slicer);
    //Fixme: Not very clean to have it here. Find better solution.
    if(uri->schema != NULL){
        _wot_io_send(receiver, uri->schema, strlen(uri->schema), slicer);
    }
    _wot_io_send(receiver, uri->value, strlen(uri->value), slicer);
    _wot_io_send(receiver, "\"", 1, slicer);
}

void _wot_io_send_date(wot_td_serialize_receiver_t receiver, wot_td_date_time_t *date, wot_td_ser_slicer_t *slicer){
    _wot_io_send(receiver, "\"", 1, slicer);
    char s[11];
    _itoa(date->year, s);
    _wot_io_send(receiver, s, strlen(s), slicer);
    _wot_io_send(receiver, "-", 1, slicer);
    _itoa(date->month, s);
    _wot_io_send(receiver, s, strlen(s), slicer);
    _wot_io_send(receiver, "-", 1, slicer);
    _itoa(date->day, s);
    _wot_io_send(receiver, s, strlen(s), slicer);
    _wot_io_send(receiver, "T", 1, slicer);
    _itoa(date->hour, s);
    _wot_io_send(receiver, s, strlen(s), slicer);
    _wot_io_send(receiver, ":", 1, slicer);
    _itoa(date->minute, s);
    _wot_io_send(receiver, s, strlen(s), slicer);
    _wot_io_send(receiver, ":", 1, slicer);
    _itoa(date->second, s);
    _wot_io_send(receiver, s, strlen(s), slicer);
    _itoa(date->timezone_offset, s);
    _wot_io_send(receiver, s, strlen(s), slicer);

    _wot_io_send(receiver, "\"", 1, slicer);
}

void _wot_io_send_obj_key(wot_td_serialize_receiver_t receiver, const char *string, uint32_t length, wot_td_ser_slicer_t *slicer){
    _wot_io_send_string(receiver, string, length, slicer);
    _wot_io_send(receiver, ":", 1, slicer);
}

void _wot_io_send_bool(wot_td_serialize_receiver_t receiver, bool value, wot_td_ser_slicer_t *slicer){
    if(value){
        _wot_io_send(receiver, wot_td_ser_true, sizeof(wot_td_ser_true)-1, slicer);
    }else{
        _wot_io_send(receiver, wot_td_ser_false, sizeof(wot_td_ser_false)-1, slicer);
    }
}
