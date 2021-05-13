#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "net/wot.h"
#include "net/wot/serialization.h"
#include "net/wot/serialization/io.h"
#include "net/wot/serialization/json_writer.h"


//Gets rid of all the functions with { }. Create generalized function for it.
void _wot_td_serialize_json_obj(wot_td_obj_serializer_params_t *params){
    wot_td_norm_obj_t *data = params->data;
    _wot_td_fill_json_receiver(params->receiver, "{", 1, params->slicer);

    while(data != NULL){
        _wot_td_fill_json_obj_key(params->receiver, data->key, strlen(data->key), params->slicer);
        params->serializer(params->receiver, params->slicer, params->lang, data->value);
        if(data->next != NULL){
             _wot_td_fill_json_receiver(params->receiver, ",", 1, params->slicer);
        }
        data = data->next;
    }
    
    _wot_td_fill_json_receiver(params->receiver, "}", 1, params->slicer);
}

//Gets rid of all the functions with [ ]. Create generalized function for it.
void _wot_td_serialize_json_array(wot_td_array_serializer_params_t *params){
    wot_td_norm_array_t *data = params->data;
    _wot_td_fill_json_receiver(params->receiver, "[", 1, params->slicer);

    while(data != NULL){
        params->serializer(params->receiver, params->slicer, params->lang, data->value);
        if(data->next != NULL){
             _wot_td_fill_json_receiver(params->receiver, ",", 1, params->slicer);
        }
        data = data->next;
    }
    
    _wot_td_fill_json_receiver(params->receiver, "]", 1, params->slicer);
}