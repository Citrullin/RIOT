#ifndef NET_WOT_H
#define NET_WOT_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct json_ld_context {
    const char *key;
    const char *value;
    struct json_ld_context *next;
} json_ld_context_t;

typedef struct wot_td_type {
    const char *value;
    struct wot_td_type *next;
} wot_td_type_t;

typedef enum {
    SECURITY_SCHEME_NONE,
    SECURITY_SCHEME_BASIC,
    SECURITY_SCHEME_DIGEST,
    SECURITY_SCHEME_API_KEY,
    SECURITY_SCHEME_BEARER,
    SECURITY_SCHEME_PSK,
    SECURITY_SCHEME_OAUTH2
} wot_td_sec_scheme_type_t;

typedef enum {
    SECURITY_SCHEME_IN_DEFAULT,
    SECURITY_SCHEME_IN_HEADER,
    SECURITY_SCHEME_IN_QUERY,
    SECURITY_SCHEME_IN_BODY,
    SECURITY_SCHEME_IN_COOKIE
} wot_td_sec_scheme_in_t;

typedef struct {
    wot_td_sec_scheme_in_t in;
    const char *name;
} wot_td_basic_sec_scheme_t;

typedef enum {
    SECURITY_DIGEST_QOP_AUTH,
    SECURITY_DIGEST_QOP_AUTH_INT
} wot_td_digest_qop_t;

typedef struct {
    wot_td_digest_qop_t qop;
    wot_td_sec_scheme_in_t in;
    const char *name;
} wot_td_digest_sec_scheme_t;

typedef struct {
    wot_td_sec_scheme_in_t in;
    const char *name;
} wot_td_api_key_sec_scheme_t;

typedef struct {
    const char *schema;
    const char *value;
} wot_td_uri_t;

typedef struct {
    wot_td_uri_t *authorization;
    const char *alg;
    const char *format;
    wot_td_sec_scheme_in_t in;
    const char *name;
} wot_td_bearer_sec_scheme_t;

typedef struct {
    const char *identity;
} wot_td_psk_sec_scheme_t;

typedef struct wot_td_auth_scope {
    const char *value;
    struct wot_td_auth_scope *next;
} wot_td_auth_scopes_t;

typedef struct {
    wot_td_uri_t *authorization;
    wot_td_uri_t *token;
    wot_td_uri_t *refresh;
    wot_td_auth_scopes_t *scopes;
    const char *flow;
} wot_td_oauth2_sec_scheme_t;

typedef struct wot_td_multi_language {
    const char *tag;
    const char *value;
    struct wot_td_multi_language *next;
} wot_td_multi_lang_t;

typedef struct {
    wot_td_type_t *type; /** (@)type in specs **/
    wot_td_multi_lang_t *descriptions;
    wot_td_uri_t *proxy;
    wot_td_sec_scheme_type_t scheme_type;
    void *scheme;
} wot_td_sec_scheme_t;

typedef struct wot_td_sec_def {
    const char *key;
    wot_td_sec_scheme_t *value;
    struct wot_td_sec_def *next;
} wot_td_security_definition_t;

typedef struct wot_td_sec {
    wot_td_security_definition_t *definition;
    struct wot_td_sec *next;
} wot_td_security_t;

typedef enum {
    FORM_OP_READ_PROPERTY,
    FORM_OP_WRITE_PROPERTY,
    FORM_OP_OBSERVE_PROPERTY,
    FORM_OP_UNOBSERVE_PROPERTY,
    FORM_OP_INVOKE_ACTION,
    FORM_OP_SUBSCRIBE_EVENT,
    FORM_OP_UNSUBSCRIBE_EVENT,
    FORM_OP_READ_ALL_PROPERTIES,
    FORM_OP_WRITE_ALL_PROPERTIES,
    FORM_OP_READ_MULTIPLE_PROPERTIES,
    FORM_OP_WRITE_MULTIPLE_PROPERTIES
} wot_td_form_op_type_t;

typedef struct wot_td_form_op {
    wot_td_form_op_type_t op_type;
    struct wot_td_form_op *next;
} wot_td_form_op_t;

typedef enum {
    MEDIA_TYPE_NONE,
    MEDIA_TYPE_JSON,
    MEDIA_TYPE_TEXT_PLAIN,
    MEDIA_TYPE_JSON_LD,
    MEDIA_TYPE_CSV
} wot_td_media_type_t;

typedef struct wot_td_media_type_parameter {
    const char *key;
    const char *value;
    struct wot_td_media_type_parameter *next;
} wot_td_media_type_parameter_t;

typedef struct {
    wot_td_media_type_t media_type;
    wot_td_media_type_parameter_t *media_type_parameter;
} wot_td_content_type_t;

typedef enum {
    CONTENT_ENCODING_NONE,
    CONTENT_ENCODING_GZIP,
    CONTENT_ENCODING_COMPRESS,
    CONTENT_ENCODING_DEFLATE,
    CONTENT_ENCODING_IDENTITY,
    CONTENT_ENCODING_BROTLI
} wot_td_content_encoding_type_t;

typedef struct {
    wot_td_content_type_t *content_type;
} wot_td_expected_res_t;

typedef void (*wot_td_serialize_receiver_t)(const char *c);
typedef void (*wot_td_ser_parser_t)(wot_td_serialize_receiver_t receiver, const char * name, const void * data);

typedef struct wot_td_extension {
    const char *name;
    const void *data;
    wot_td_ser_parser_t parser;
    struct wot_td_extension *next;
} wot_td_extension_t;

typedef struct wot_td_form {
    wot_td_form_op_t *op;
    wot_td_uri_t *href;
    wot_td_content_type_t *content_type;
    wot_td_content_encoding_type_t content_encoding;
    const char *sub_protocol;
    wot_td_security_t *security;
    wot_td_auth_scopes_t *scopes;
    wot_td_expected_res_t *expected_response;
    wot_td_extension_t *extensions;
    struct wot_td_form *next;
} wot_td_form_t;

typedef struct {
    const double *minimum;
    const double *maximum;
} wot_td_number_schema_t;

typedef struct {
    const int32_t *minimum;
    const int32_t *maximum;
} wot_td_integer_schema_t;

typedef struct wot_td_object_required {
    const char *value;
    struct wot_td_object_required *next;
} wot_td_object_required_t;

typedef struct wot_td_data_enum {
    const char *value;
    struct wot_td_data_enum *next;
} wot_td_data_enums_t;

typedef enum {
    JSON_TYPE_NONE,
    JSON_TYPE_OBJECT,
    JSON_TYPE_ARRAY,
    JSON_TYPE_STRING,
    JSON_TYPE_NUMBER,
    JSON_TYPE_INTEGER,
    JSON_TYPE_BOOLEAN,
    JSON_TYPE_NULL
} wot_td_json_type_t;

typedef struct {
    wot_td_type_t *type; /** (@)type in specs **/
    wot_td_multi_lang_t *titles;
    wot_td_multi_lang_t *descriptions;
    wot_td_json_type_t json_type;
    const char *constant;
    const char *unit;
    struct wot_td_data_schemas *one_of; /** check validity of data **/
    wot_td_data_enums_t *enumeration;
    bool read_only;
    bool write_only;
    const char *format;
    void *schema; /** Type determined by type **/
} wot_td_data_schema_t;

typedef struct wot_td_data_schemas {
    wot_td_data_schema_t *value;
    struct wot_td_data_schemas *next;
} wot_td_data_schemas_t;

typedef struct {
    wot_td_data_schemas_t *items;
    const uint32_t *min_items;
    const uint32_t *max_items;
} wot_td_array_schema_t;

typedef struct wot_td_data_schema_map {
    const char *key;
    wot_td_data_schema_t *value;
    struct wot_td_data_schema_map *next;
} wot_td_data_schema_map_t;

typedef struct {
    wot_td_data_schema_map_t *properties;
    wot_td_object_required_t *required;
} wot_td_object_schema_t;

typedef struct {
    wot_td_type_t *type; /** (@)type in specs **/
    wot_td_multi_lang_t *titles;
    wot_td_multi_lang_t *descriptions;
    wot_td_form_t *forms;
    wot_td_data_schema_map_t *uri_variables;
} wot_td_int_affordance_t;

typedef struct wot_td_prop_affordance {
    const char *key;
    bool observable;
    wot_td_data_schema_t *data_schema;
    wot_td_int_affordance_t *int_affordance;
    struct wot_td_prop_affordance *next;
} wot_td_prop_affordance_t;

typedef struct wot_td_action_affordance {
    const char *key;
    wot_td_data_schema_t *input;
    wot_td_data_schema_t *output;
    bool safe;
    bool idempotent;
    wot_td_int_affordance_t *int_affordance;
    struct wot_td_action_affordance *next;
} wot_td_action_affordance_t;

typedef struct wot_td_event_affordance {
    const char *key;
    wot_td_data_schema_t *subscription;
    wot_td_data_schema_t *data;
    wot_td_data_schema_t *cancellation;
    wot_td_int_affordance_t *int_affordance;
    struct wot_td_event_affordance *next;
} wot_td_event_affordance_t;

typedef struct {
    const char *instance;
} wot_td_version_info_t;

typedef struct {
    const int32_t year;
    const uint8_t month;
    const uint8_t day;
    const uint8_t hour;
    const uint8_t minute;
    const uint8_t second;
    const int16_t timezone_offset;
} wot_td_date_time_t;

typedef struct wot_td_link {
    wot_td_uri_t *href;
    wot_td_media_type_t type;
    const char *rel;
    wot_td_uri_t *anchor;
    struct wot_td_link *next;
} wot_td_link_t;

typedef struct {
    json_ld_context_t *context; /*< The context elements. Linked list. */
    wot_td_type_t *type;
    wot_td_uri_t *id;
    wot_td_multi_lang_t *titles;
    wot_td_multi_lang_t *descriptions;
    wot_td_version_info_t *version;
    wot_td_date_time_t *created;
    wot_td_date_time_t *modified;
    wot_td_uri_t *support;
    wot_td_uri_t *base;
    wot_td_prop_affordance_t *properties;
    wot_td_action_affordance_t *actions;
    wot_td_event_affordance_t *events;
    wot_td_link_t *links;
    wot_td_form_t *forms;
    wot_td_security_t *security;
    wot_td_security_definition_t *security_def;
    char *default_language_tag;
} wot_td_thing_t;

int wot_td_thing_context_add(wot_td_thing_t *thing, json_ld_context_t *context);
int wot_td_thing_context_rm(wot_td_thing_t *thing, json_ld_context_t *context);
json_ld_context_t * wot_td_thing_context_find_nth(wot_td_thing_t *thing, uint8_t pos);
json_ld_context_t * wot_td_thing_context_find_key(wot_td_thing_t *thing, char *key);
json_ld_context_t * wot_td_thing_context_find_value(wot_td_thing_t *thing, char *value);


int wot_td_thing_title_add(wot_td_thing_t *thing, wot_td_multi_lang_t *title);
int wot_td_thing_title_rm(wot_td_thing_t *thing, wot_td_multi_lang_t *title);
wot_td_multi_lang_t * wot_td_thing_titles_find_nth(wot_td_thing_t *thing, uint8_t pos);
wot_td_multi_lang_t * wot_td_thing_titles_find_tag(wot_td_thing_t *thing, char *tag);
wot_td_multi_lang_t * wot_td_thing_titles_find_value(wot_td_thing_t *thing, char *value);


int wot_td_thing_desc_add(wot_td_thing_t *thing, wot_td_multi_lang_t *desc);
int wot_td_thing_desc_rm(wot_td_thing_t *thing, wot_td_multi_lang_t *desc);
wot_td_multi_lang_t * wot_td_thing_desc_find_nth(wot_td_thing_t *thing, uint8_t pos);
wot_td_multi_lang_t * wot_td_thing_desc_find_tag(wot_td_thing_t *thing, char *tag);
wot_td_multi_lang_t * wot_td_thing_desc_find_value(wot_td_thing_t *thing, char *value);

int wot_td_thing_type_add(wot_td_thing_t *thing, wot_td_type_t *type);
int wot_td_thing_type_rm(wot_td_thing_t *thing, wot_td_type_t *type);
wot_td_type_t * wot_td_thing_type_find_nth(wot_td_thing_t *thing, uint8_t pos);
wot_td_type_t * wot_td_thing_type_find_value(wot_td_thing_t *thing, char *value);


int wot_td_thing_prop_add(wot_td_thing_t *thing, wot_td_prop_affordance_t *property);
int wot_td_thing_prop_rm(wot_td_thing_t *thing, wot_td_prop_affordance_t *property);
wot_td_prop_affordance_t * wot_td_thing_prop_find_nth(wot_td_thing_t *thing, uint8_t pos);
wot_td_prop_affordance_t * wot_td_thing_prop_find_key(wot_td_thing_t *thing, char *key);


int wot_td_thing_action_add(wot_td_thing_t *thing, wot_td_action_affordance_t *action);
int wot_td_thing_action_rm(wot_td_thing_t *thing, wot_td_action_affordance_t *action);
wot_td_action_affordance_t * wot_td_thing_action_find_nth(wot_td_thing_t *thing, uint8_t pos);
wot_td_action_affordance_t * wot_td_thing_action_find_key(wot_td_thing_t *thing, char *key);


int wot_td_thing_event_add(wot_td_thing_t *thing, wot_td_event_affordance_t *event);
int wot_td_thing_event_rm(wot_td_thing_t *thing, wot_td_event_affordance_t *event);
wot_td_event_affordance_t * wot_td_thing_event_find_nth(wot_td_thing_t *thing, uint8_t pos);
wot_td_event_affordance_t * wot_td_thing_event_find_key(wot_td_thing_t *thing, char *key);


int wot_td_thing_security_add(wot_td_thing_t *thing, wot_td_security_t *security);
int wot_td_thing_security_rm(wot_td_thing_t *thing, wot_td_security_t *security);
wot_td_security_t * wot_td_thing_security_find_nth(wot_td_thing_t *thing, uint8_t pos);


int wot_td_thing_security_def_add(wot_td_thing_t *thing, wot_td_security_definition_t *security_def);
int wot_td_thing_security_def_rm(wot_td_thing_t *thing, wot_td_security_definition_t *security_def);
wot_td_security_t * wot_td_thing_security_def_find_nth(wot_td_thing_t *thing, uint8_t pos);
wot_td_security_t * wot_td_thing_security_def_find_key(wot_td_thing_t *thing, char *key);


int wot_td_thing_link_add(wot_td_thing_t *thing, wot_td_link_t *link);
int wot_td_thing_link_rm(wot_td_thing_t *thing, wot_td_link_t *link);
wot_td_link_t * wot_td_thing_link_find_nth(wot_td_thing_t *thing, uint8_t pos);


int wot_td_thing_form_add(wot_td_thing_t *thing, wot_td_form_t *form);
int wot_td_thing_form_rm(wot_td_thing_t *thing, wot_td_form_t *form);
wot_td_form_t * wot_td_thing_form_find_nth(wot_td_thing_t *thing, uint8_t pos);


int wot_td_form_security_add(wot_td_form_t *form, wot_td_security_t *security);
int wot_td_form_security_rm(wot_td_form_t *form, wot_td_security_t *security);
wot_td_security_t * wot_td_form_security_find_nth(wot_td_form_t *form, uint8_t pos);
wot_td_security_t * wot_td_form_security_find_key(wot_td_form_t *form, char *key);


int wot_td_form_auth_scope_add(wot_td_form_t *form, wot_td_auth_scopes_t *scope);
int wot_td_form_auth_scope_rm(wot_td_form_t *form, wot_td_auth_scopes_t *scope);
wot_td_auth_scopes_t * wot_td_form_auth_scope_find_nth(wot_td_form_t *form, uint8_t pos);
wot_td_auth_scopes_t * wot_td_form_auth_scope_find_value(wot_td_form_t *form, char *value);

int wot_td_form_op_add(wot_td_form_t *form, wot_td_form_op_t *op);
int wot_td_form_op_rm(wot_td_form_t *form, wot_td_form_op_t *op);
wot_td_form_op_t * wot_td_form_op_find_nth(wot_td_form_t *form, uint8_t pos);


int wot_td_affordance_title_add(wot_td_int_affordance_t *affordance, wot_td_multi_lang_t *title);
int wot_td_affordance_title_rm(wot_td_int_affordance_t *affordance, wot_td_multi_lang_t *title);
wot_td_multi_lang_t * wot_td_affordance_title_find_nth(wot_td_int_affordance_t *affordance, uint8_t pos);
wot_td_multi_lang_t * wot_td_affordance_title_find_tag(wot_td_int_affordance_t *affordance, char *tag);
wot_td_multi_lang_t * wot_td_affordance_title_find_value(wot_td_int_affordance_t *affordance, char *value);

int wot_td_affordance_form_add(wot_td_int_affordance_t *affordance, wot_td_form_t *form);
int wot_td_affordance_form_rm(wot_td_int_affordance_t *affordance, wot_td_form_t *form);
wot_td_form_t * wot_td_affordance_form_find_nth(wot_td_int_affordance_t *affordance, uint8_t pos);

int wot_td_security_desc_add(wot_td_sec_scheme_t *security, wot_td_multi_lang_t *desc);
int wot_td_security_desc_rm(wot_td_sec_scheme_t *security, wot_td_multi_lang_t *desc);
wot_td_multi_lang_t * wot_td_security_desc_find_nth(wot_td_sec_scheme_t *security, uint8_t pos);
wot_td_multi_lang_t * wot_td_security_desc_find_tag(wot_td_sec_scheme_t *security, char *tag);
wot_td_multi_lang_t * wot_td_security_desc_find_value(wot_td_sec_scheme_t *security, char *value);


int wot_td_oauth_auth_scope_add(wot_td_oauth2_sec_scheme_t *scheme, wot_td_auth_scopes_t *scope);
int wot_td_oauth_auth_scope_rm(wot_td_oauth2_sec_scheme_t *scheme, wot_td_auth_scopes_t *scope);
wot_td_auth_scopes_t * wot_td_oauth_scope_find_nth(wot_td_oauth2_sec_scheme_t *scheme, uint8_t pos);
wot_td_auth_scopes_t * wot_td_oauth_scope_find_value(wot_td_oauth2_sec_scheme_t *scheme, char *value);


int wot_td_data_schema_desc_add(wot_td_data_schema_t *schema, wot_td_multi_lang_t *desc);
int wot_td_data_schema_desc_rm(wot_td_data_schema_t *schema, wot_td_multi_lang_t *desc);
wot_td_multi_lang_t * wot_td_data_schema_desc_find_nth(wot_td_data_schema_t *schema, uint8_t pos);
wot_td_multi_lang_t * wot_td_data_schema_desc_find_tag(wot_td_data_schema_t *schema, char *tag);
wot_td_multi_lang_t * wot_td_data_schema_desc_find_value(wot_td_data_schema_t *schema, char *value);


int wot_td_data_schema_title_add(wot_td_data_schema_t *schema, wot_td_multi_lang_t *title);
int wot_td_data_schema_title_rm(wot_td_data_schema_t *schema, wot_td_multi_lang_t *title);
wot_td_multi_lang_t * wot_td_data_data_schema_title_find_nth(wot_td_data_schema_t *schema, uint8_t pos);
wot_td_multi_lang_t * wot_td_data_schema_title_find_tag(wot_td_data_schema_t *schema, char *tag);
wot_td_multi_lang_t * wot_td_data_schema_title_find_value(wot_td_data_schema_t *schema, char *value);


int wot_td_data_schema_validation_add(wot_td_data_schema_t *schema, wot_td_data_schemas_t *validator);
int wot_td_data_schema_validation_rm(wot_td_data_schema_t *schema, wot_td_data_schemas_t *validator);
wot_td_data_schemas_t * wot_td_data_data_schema_validator_find_nth(wot_td_data_schema_t *schema, uint8_t pos);


int wot_td_data_schema_enum_add(wot_td_data_schema_t *schema, wot_td_data_enums_t *enumeration);
int wot_td_data_schema_enum_rm(wot_td_data_schema_t *schema, wot_td_data_enums_t *enumeration);
wot_td_data_enums_t * wot_td_data_schema_enum_find_nth(wot_td_data_schema_t *schema, uint8_t pos);
wot_td_data_enums_t * wot_td_data_schema_enum_find_value(wot_td_data_schema_t *schema, char *value);

#ifdef __cplusplus
}
#endif

#endif /* NET_WOT_H */
