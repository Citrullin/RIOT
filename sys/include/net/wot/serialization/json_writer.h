
//Todo: Refactor and move into another c file.
typedef struct wot_td_norm_obj {
    struct wot_td_norm_obj * next;
    const char * key;
    void *value;
} wot_td_norm_obj_t;

typedef struct wot_td_norm_array {
    struct wot_td_norm_array * next;
    void *value;
} wot_td_norm_array_t;

typedef void (*wot_td_obj_serializer_t)(wot_td_serialize_receiver_t receiver, wot_td_ser_slicer_t *slicer, char *lang, void *data);

typedef struct {
    wot_td_serialize_receiver_t receiver;
    wot_td_ser_slicer_t *slicer;
    wot_td_obj_serializer_t serializer;
    wot_td_norm_obj_t *data;
    char *lang;
}  wot_td_obj_serializer_params_t;

typedef struct {
    wot_td_serialize_receiver_t receiver;
    wot_td_ser_slicer_t *slicer;
    wot_td_obj_serializer_t serializer;
    wot_td_norm_array_t *data;
    char *lang;
}  wot_td_array_serializer_params_t;

void _wot_td_serialize_json_obj(wot_td_obj_serializer_params_t *params);
void _wot_td_serialize_json_array(wot_td_array_serializer_params_t *params);