
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

typedef void (*json_writer_t)(wot_td_serialize_receiver_t receiver, wot_td_ser_slicer_t *slicer, char *lang, void *data);

typedef struct {
    wot_td_serialize_receiver_t receiver;
    wot_td_ser_slicer_t *slicer;
    json_writer_t writer;
    wot_td_norm_obj_t *data;
    char *lang;
}  wot_td_obj_serializer_params_t;

typedef struct {
    wot_td_serialize_receiver_t receiver;
    wot_td_ser_slicer_t *slicer;
    json_writer_t writer;
    wot_td_norm_array_t *data;
    char *lang;
}  wot_td_array_serializer_params_t;

void _json_write_obj(wot_td_obj_serializer_params_t *params);
void _json_write_array(wot_td_array_serializer_params_t *params);
void _string_writer(wot_td_serialize_receiver_t receiver, wot_td_ser_slicer_t *slicer, char *lang, void *data);
void _serialize_lang(wot_td_serialize_receiver_t receiver, wot_td_ser_slicer_t *slicer, char * lang, void * data);
void _wot_td_security_writer(wot_td_serialize_receiver_t receiver, wot_td_ser_slicer_t *slicer, char *lang, void *data);
void _serialize_sec_def_string(wot_td_serialize_receiver_t receiver, wot_td_ser_slicer_t *slicer, char *lang, void *data);
void _write_security(wot_td_serialize_receiver_t receiver, wot_td_ser_slicer_t *slicer, char *lang, void *data);