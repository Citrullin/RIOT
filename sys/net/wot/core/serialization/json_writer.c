#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "net/wot.h"
#include "net/wot/serialization.h"
#include "net/wot/serialization/io.h"
#include "net/wot/serialization/json_writer.h"
#include "net/wot/serialization/json_keys.h"

//Gets rid of all the functions with { }. Create generalized function for it.
void _json_write_obj(wot_td_obj_serializer_params_t *params){
    wot_td_norm_obj_t *data = params->data;
    _wot_io_send(params->receiver, "{", 1, params->slicer);

    while(data != NULL){
        _wot_io_send_obj_key(params->receiver, data->key, strlen(data->key), params->slicer);
        params->writer(params->receiver, params->slicer, params->lang, data->value);
        if(data->next != NULL){
             _wot_io_send(params->receiver, ",", 1, params->slicer);
        }
        data = data->next;
    }
    
    _wot_io_send(params->receiver, "}", 1, params->slicer);
}

//Gets rid of all the functions with [ ]. Create generalized function for it.
void _json_write_array(wot_td_array_serializer_params_t *params){
    wot_td_norm_array_t *data = params->data;
    _wot_io_send(params->receiver, "[", 1, params->slicer);

    while(data != NULL){
        params->writer(params->receiver, params->slicer, params->lang, data->value);
        if(data->next != NULL){
             _wot_io_send(params->receiver, ",", 1, params->slicer);
        }
        data = data->next;
    }
    
    _wot_io_send(params->receiver, "]", 1, params->slicer);
}

void _string_writer(wot_td_serialize_receiver_t receiver, wot_td_ser_slicer_t *slicer, char *lang, void *data){
    (void)lang;
    const char *string = (const char *) data;
    _wot_io_send_string(receiver, string, strlen(string), slicer);
}

void _serialize_sec_def_string(wot_td_serialize_receiver_t receiver, wot_td_ser_slicer_t *slicer, char *lang, void *data){
    (void)lang;
    wot_td_security_definition_t *sec_def = (wot_td_security_definition_t *)data;
    _wot_io_send_string(receiver, sec_def->key, strlen(sec_def->key), slicer);
}

void _serialize_lang(wot_td_serialize_receiver_t receiver, wot_td_ser_slicer_t *slicer, char * lang, void * data){
    (void)lang;
    const char *value = data;
    _wot_io_send_string(receiver, value, strlen(value), slicer);
}

void _wot_td_security_writer(wot_td_serialize_receiver_t receiver, wot_td_ser_slicer_t *slicer, char *lang, void *data){
    (void)lang;
    wot_td_security_t *security = (wot_td_security_t*)data;
    _wot_io_send_string(receiver, security->definition->key, strlen(security->definition->key), slicer);
}

void _write_security(wot_td_serialize_receiver_t receiver, wot_td_ser_slicer_t *slicer, char *lang, void *data){
    wot_td_sec_scheme_t *scheme = (wot_td_sec_scheme_t *)data;
    _wot_io_send(receiver, "{", 1, slicer);

    _wot_io_send_obj_key(receiver, _json_key_scheme, sizeof(_json_key_scheme)-1, slicer);
    _security_scheme_string(receiver, scheme->scheme_type, slicer);
    _wot_io_send(receiver, ",", 1, slicer);
    _write_description_list(receiver, scheme->descriptions, lang, slicer);
    if(scheme->proxy != NULL){
        _wot_io_send(receiver, ",", 1, slicer);
        _wot_io_send_obj_key(receiver, "proxy", sizeof("proxy")-1, slicer);
        _wot_io_send_uri(receiver, scheme->proxy, slicer);
    }

    if(scheme->scheme_type != SECURITY_SCHEME_NONE){
        _wot_io_send(receiver, ",", 1, slicer);
    }
    _write_security_schema(receiver, scheme, slicer);

    _wot_io_send(receiver, "}", 1, slicer);
}