
#include "net/wot.h"
#include "net/wot/serialization.h"

void _itoa(int n, char s[]);
void _reverse(char s[]);
int _wot_td_fill_json_receiver(wot_td_serialize_receiver_t receiver, const char *string, uint32_t length, wot_td_ser_slicer_t *slicer);
void _wot_td_fill_json_string(wot_td_serialize_receiver_t receiver, const char *string, uint32_t length, wot_td_ser_slicer_t *slicer);
void _wot_td_fill_json_uri(wot_td_serialize_receiver_t receiver, wot_td_uri_t *uri, wot_td_ser_slicer_t *slicer);
void _wot_td_fill_json_date(wot_td_serialize_receiver_t receiver, wot_td_date_time_t *date, wot_td_ser_slicer_t *slicer);
void _wot_td_fill_json_obj_key(wot_td_serialize_receiver_t receiver, const char *string, uint32_t length, wot_td_ser_slicer_t *slicer);
void _wot_td_fill_json_bool(wot_td_serialize_receiver_t receiver, bool value, wot_td_ser_slicer_t *slicer);
void _wot_td_string_writer(wot_td_serialize_receiver_t receiver, wot_td_ser_slicer_t *slicer, char *lang, void *data);