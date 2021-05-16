#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "net/wot.h"
#include "net/wot/serialization.h"
#include "net/wot/serialization/io.h"
#include "net/wot/serialization/json_writer.h"
#include "net/wot/serialization/json_keys.h"


void _previous_prop_check(wot_td_serialize_receiver_t receiver, bool has_previous_prop, wot_td_ser_slicer_t *slicer){
    if(has_previous_prop){
        _wot_io_send(receiver, ",", 1, slicer);
    }
}

void _writer_context(wot_td_serialize_receiver_t receiver, json_ld_context_t *context, wot_td_ser_slicer_t *slicer){
    if(context->key != NULL){
        _wot_io_send(receiver, "{", 1, slicer);

        _wot_io_send_obj_key(receiver, context->key, strlen(context->key), slicer);
    }

    _wot_io_send_string(receiver, context->value, strlen(context->value), slicer);

    if(context->key != NULL) {
        _wot_io_send(receiver, "}", 1, slicer);
    }
}

void _write_context_list(wot_td_serialize_receiver_t receiver, json_ld_context_t *context, wot_td_ser_slicer_t *slicer){
    json_ld_context_t *tmp_ctx = context;

    while(tmp_ctx != NULL){
        _writer_context(receiver, tmp_ctx, slicer);
        if(tmp_ctx->next != NULL){
            _wot_io_send(receiver, ",", 1, slicer);
        }
        tmp_ctx = tmp_ctx->next;
    }
}

void _write_type_list(wot_td_serialize_receiver_t receiver, wot_td_type_t *type, wot_td_ser_slicer_t *slicer){
    _wot_io_send_obj_key(receiver, _json_key_titles, sizeof(_json_key_titles)-1, slicer);
    wot_td_array_serializer_params_t params = {
        .receiver = receiver,
        .slicer = slicer,
        .data = (wot_td_norm_array_t *)type,
        .writer = (json_writer_t)&_string_writer,
    };
    _json_write_array(&params);
}

void _write_title_list(wot_td_serialize_receiver_t receiver, wot_td_multi_lang_t *titles, char *lang, wot_td_ser_slicer_t *slicer){
    _wot_io_send_obj_key(receiver, _json_key_titles, sizeof(_json_key_titles)-1, slicer);
    wot_td_obj_serializer_params_t params = {
        .receiver = receiver,
        .slicer = slicer,
        .data = (wot_td_norm_obj_t *)titles,
        .writer = (json_writer_t)&_serialize_lang,
    };
    _json_write_obj(&params);

    //Todo: Hopefully TD 2.0 spec doesn't require this anymore.
    wot_td_multi_lang_t *tmp = titles;
    wot_td_multi_lang_t *default_title = NULL;
    while(tmp != NULL && default_title == NULL){
        if(lang != NULL && strcmp(tmp->tag, lang) == 0){
            default_title = tmp;
        }
        if(default_title == NULL){
            tmp = tmp->next;
        }
    }

    if(default_title != NULL){
        _wot_io_send(receiver, ",", 1, slicer);
        _wot_io_send_obj_key(receiver, _json_key_titles, sizeof(_json_key_titles)-2, slicer);
        _wot_io_send_string(receiver, default_title->value, strlen(default_title->value), slicer);
    }
}

void _write_description_list(wot_td_serialize_receiver_t receiver, wot_td_multi_lang_t *desc, char *lang, wot_td_ser_slicer_t *slicer){
    _wot_io_send_obj_key(receiver, _json_key_descriptions, sizeof(_json_key_descriptions)-1, slicer);
    wot_td_obj_serializer_params_t params = {
        .receiver = receiver,
        .slicer = slicer,
        .data = (wot_td_norm_obj_t *)desc,
        .writer = (json_writer_t)&_serialize_lang,
    };
    _json_write_obj(&params);

    //Todo: Same here. Check, if TD 2.0 spec still requires this.
    wot_td_multi_lang_t *tmp = desc;
    wot_td_multi_lang_t *default_description = NULL;
    while(tmp != NULL && default_description == NULL){
        if(lang != NULL && strcmp(tmp->tag, lang) == 0){
            default_description = tmp;
        }
        if(default_description == NULL){
            tmp = tmp->next;
        }
    }

    if(default_description != NULL){
        _wot_io_send(receiver, ",", 1, slicer);
        _wot_io_send_obj_key(receiver, _json_key_descriptions, sizeof(_json_key_descriptions)-2, slicer);
        _wot_io_send_string(receiver, default_description->value, strlen(default_description->value), slicer);
    }
}

//Fixme: Use const vars
void _security_scheme_string(wot_td_serialize_receiver_t receiver, wot_td_sec_scheme_type_t scheme_type, wot_td_ser_slicer_t *slicer){
    switch (scheme_type) {
        case SECURITY_SCHEME_NONE:
            _wot_io_send_string(receiver, "nosec", sizeof("nosec")-1, slicer);
            break;
        case SECURITY_SCHEME_BASIC:
            _wot_io_send_string(receiver, "basic", sizeof("basic")-1, slicer);
            break;
        case SECURITY_SCHEME_DIGEST:
            _wot_io_send_string(receiver, "digest", sizeof("digest")-1, slicer);
            break;
        case SECURITY_SCHEME_API_KEY:
            _wot_io_send_string(receiver, "apikey", sizeof("apikey")-1, slicer);
            break;
        case SECURITY_SCHEME_BEARER:
            _wot_io_send_string(receiver, "bearer", sizeof("bearer")-1, slicer);
            break;
        case SECURITY_SCHEME_PSK:
            _wot_io_send_string(receiver, "psk", sizeof("psk")-1, slicer);
            break;
        case SECURITY_SCHEME_OAUTH2:
            _wot_io_send_string(receiver, "oauth2", sizeof("oauth2")-1, slicer);
            break;
        default:
            _wot_io_send_string(receiver, "nosec", sizeof("nosec")-1, slicer);
            break;
    }
}

void _write_security_schema_in(wot_td_serialize_receiver_t receiver, wot_td_sec_scheme_in_t in, wot_td_ser_slicer_t *slicer){
    switch(in){
        case SECURITY_SCHEME_IN_HEADER:
            _wot_io_send_string(receiver, "header", sizeof("header")-1, slicer);
            break;
        case SECURITY_SCHEME_IN_QUERY:
            _wot_io_send_string(receiver, "query", sizeof("query")-1, slicer);
            break;
        case SECURITY_SCHEME_IN_BODY:
            _wot_io_send_string(receiver, "body", sizeof("body")-1, slicer);
            break;
        case SECURITY_SCHEME_IN_COOKIE:
            _wot_io_send_string(receiver, "cookie", sizeof("cookie")-1, slicer);
            break;
        default:
            _wot_io_send_string(receiver, "header", sizeof("header")-1, slicer);
    }
}

void _write_sec_scheme_basic(wot_td_serialize_receiver_t receiver, wot_td_basic_sec_scheme_t *scheme, wot_td_ser_slicer_t *slicer){
    _wot_io_send_obj_key(receiver, _json_key_in, sizeof(_json_key_in)-1, slicer);
    _write_security_schema_in(receiver, scheme->in, slicer);

    _wot_io_send(receiver, ",", 1, slicer);
    _wot_io_send_obj_key(receiver, _json_key_name, sizeof(_json_key_name)-1, slicer);
    _wot_io_send_string(receiver, scheme->name, strlen(scheme->name), slicer);
}

void _write_security_digest_qop(wot_td_serialize_receiver_t receiver, wot_td_digest_qop_t qop, wot_td_ser_slicer_t *slicer){
    if(qop == SECURITY_DIGEST_QOP_AUTH_INT){
        _wot_io_send_string(receiver, "auth-int", sizeof("auth-int")-1, slicer);
    }else{
        _wot_io_send_string(receiver, "auth", sizeof("auth")-1, slicer);
    }
}

void _write_sec_scheme_digest(wot_td_serialize_receiver_t receiver, wot_td_digest_sec_scheme_t *scheme, wot_td_ser_slicer_t *slicer){
    _wot_io_send_obj_key(receiver, _json_key_qop, sizeof(_json_key_qop)-1, slicer);
    _write_security_digest_qop(receiver, scheme->qop, slicer);

    _wot_io_send(receiver, ",", 1, slicer);
    _wot_io_send_obj_key(receiver, _json_key_in, sizeof(_json_key_in)-1, slicer);
    _write_security_schema_in(receiver, scheme->in, slicer);
    _wot_io_send(receiver, ",", 1, slicer);
    _wot_io_send_obj_key(receiver, _json_key_name, sizeof(_json_key_name)-1, slicer);
    _wot_io_send_string(receiver, scheme->name, strlen(scheme->name), slicer);
}

void _write_sec_scheme_api_key(wot_td_serialize_receiver_t receiver, wot_td_api_key_sec_scheme_t *scheme, wot_td_ser_slicer_t *slicer){
    _wot_io_send_obj_key(receiver, _json_key_in, sizeof(_json_key_in)-1, slicer);
    _write_security_schema_in(receiver, scheme->in, slicer);
    if(scheme->name != NULL){
        _wot_io_send(receiver, ",", 1, slicer);
        _wot_io_send_obj_key(receiver, _json_key_name, sizeof(_json_key_name)-1, slicer);
        _wot_io_send_string(receiver, scheme->name, strlen(scheme->name), slicer);
    }
}

void _write_sec_scheme_bearer(wot_td_serialize_receiver_t receiver, wot_td_bearer_sec_scheme_t *scheme, wot_td_ser_slicer_t *slicer){
    bool has_previous_prop = false;
    if(scheme->authorization != NULL){
        has_previous_prop = true;
        _wot_io_send_obj_key(receiver, _json_key_authorization, sizeof(_json_key_authorization)-1, slicer);
        _wot_io_send_uri(receiver, scheme->authorization, slicer);
    }

    //Todo: Use enum instead
    if(scheme->alg != NULL){
        _previous_prop_check(receiver, has_previous_prop, slicer);
        has_previous_prop = true;
        _wot_io_send_obj_key(receiver, "alg", sizeof("alg")-1, slicer);
        _wot_io_send_string(receiver, scheme->alg, strlen(scheme->alg), slicer);
    }

    if(scheme->format != NULL){
        _previous_prop_check(receiver, has_previous_prop, slicer);
        has_previous_prop = true;
        _wot_io_send_obj_key(receiver, _json_key_format, sizeof(_json_key_format)-1, slicer);
        _wot_io_send_string(receiver, scheme->format, strlen(scheme->format), slicer);
    }

    if(scheme->in != SECURITY_SCHEME_IN_DEFAULT){
        _previous_prop_check(receiver, has_previous_prop, slicer);
        has_previous_prop = true;
        _wot_io_send_obj_key(receiver, _json_key_in, sizeof(_json_key_in)-1, slicer);
        _write_security_schema_in(receiver, scheme->in, slicer);
    }

    if(scheme->name != NULL){
        _previous_prop_check(receiver, has_previous_prop, slicer);
        _wot_io_send_obj_key(receiver, _json_key_name, sizeof(_json_key_name)-1, slicer);
        _wot_io_send_string(receiver, scheme->name, strlen(scheme->name), slicer);
    }
}

void _write_sec_scheme_psk(wot_td_serialize_receiver_t receiver, wot_td_psk_sec_scheme_t *scheme, wot_td_ser_slicer_t *slicer){
    if(scheme->identity != NULL){
        _wot_io_send_obj_key(receiver, _json_key_identity, sizeof(_json_key_identity)-1, slicer);
        _wot_io_send_string(receiver, scheme->identity, strlen(scheme->identity), slicer);
    }
}

void _write_sec_scheme_oauth2(wot_td_serialize_receiver_t receiver, wot_td_oauth2_sec_scheme_t *scheme, wot_td_ser_slicer_t *slicer){
    _wot_io_send_obj_key(receiver, "flow", sizeof("flow")-1, slicer);
    _wot_io_send_string(receiver, scheme->flow, strlen(scheme->flow), slicer);

    if(scheme->authorization != NULL){
        _wot_io_send(receiver, ",", 1, slicer);
        _wot_io_send_obj_key(receiver, _json_key_authorization, sizeof(_json_key_authorization)-1, slicer);
        _wot_io_send_uri(receiver, scheme->authorization, slicer);
    }

    //Todo: Use macro to generate it.
    if(scheme->token != NULL){
        _wot_io_send(receiver, ",", 1, slicer);
        _wot_io_send_obj_key(receiver, "token", sizeof("token")-1, slicer);
        _wot_io_send_uri(receiver, scheme->token, slicer);
    }

    if(scheme->refresh != NULL){
        _wot_io_send(receiver, ",", 1, slicer);
        _wot_io_send_obj_key(receiver, "refresh", sizeof("refresh")-1, slicer);
        _wot_io_send_uri(receiver, scheme->refresh, slicer);
    }

    if(scheme->scopes != NULL){
        _wot_io_send(receiver, ",", 1, slicer);
        _wot_io_send_obj_key(receiver, "scopes", sizeof("scopes")-1, slicer);
        wot_td_array_serializer_params_t params = {
            .receiver = receiver,
            .slicer = slicer,
            .data = (wot_td_norm_array_t *)scheme->scopes,
            .writer = (json_writer_t)&_string_writer,
        };
        _json_write_array(&params);
    }
}

void _write_security_schema(wot_td_serialize_receiver_t receiver, wot_td_sec_scheme_t *security, wot_td_ser_slicer_t *slicer){
    switch (security->scheme_type) {
        default:
            return;
        case SECURITY_SCHEME_BASIC:
            _write_sec_scheme_basic(receiver, (wot_td_basic_sec_scheme_t *) security->scheme, slicer);
            break;
        case SECURITY_SCHEME_DIGEST:
            _write_sec_scheme_digest(receiver, (wot_td_digest_sec_scheme_t *) security->scheme, slicer);
            break;
        case SECURITY_SCHEME_API_KEY:
            _write_sec_scheme_api_key(receiver, (wot_td_api_key_sec_scheme_t *) security->scheme, slicer);
            break;
        case SECURITY_SCHEME_BEARER:
            _write_sec_scheme_bearer(receiver, (wot_td_bearer_sec_scheme_t *) security->scheme, slicer);
            break;
        case SECURITY_SCHEME_PSK:
            _write_sec_scheme_psk(receiver, (wot_td_psk_sec_scheme_t *) security->scheme, slicer);
            break;
        case SECURITY_SCHEME_OAUTH2:
            _write_sec_scheme_oauth2(receiver, (wot_td_oauth2_sec_scheme_t *) security->scheme, slicer);
            break;
    }
}

void _write_security_def_list(wot_td_serialize_receiver_t receiver, wot_td_security_definition_t *security_def, char *lang, wot_td_ser_slicer_t *slicer){
    _wot_io_send_obj_key(receiver, _json_key_sec_def, sizeof(_json_key_sec_def)-1, slicer);
    wot_td_obj_serializer_params_t params = {
        .receiver = receiver,
        .slicer = slicer,
        .data = (wot_td_norm_obj_t *)security_def,
        .lang = lang,
        .writer = (json_writer_t)&_write_security,
    };
    _json_write_obj(&params);    
}

void _serialize_security_array(wot_td_serialize_receiver_t receiver, wot_td_security_t *security, wot_td_ser_slicer_t *slicer){
   _wot_io_send_obj_key(receiver, _json_key_sec_def, 8, slicer);
    wot_td_array_serializer_params_t params = {
        .receiver = receiver,
        .slicer = slicer,
        .data = (wot_td_norm_array_t *)security,
        .writer = (json_writer_t)&_serialize_sec_def_string,
    };
    _json_write_array(&params);
}

//Fixme: Use const vars
void _write_op_type(wot_td_serialize_receiver_t receiver, wot_td_ser_slicer_t *slicer, const char *lang, void *data){
    (void)lang;
    wot_td_form_op_type_t *op_type = (wot_td_form_op_type_t *)data;
    switch (*op_type) {
        case FORM_OP_READ_PROPERTY:
            _wot_io_send_string(receiver, "readproperty", sizeof("readproperty")-1, slicer);
            break;
        case FORM_OP_WRITE_PROPERTY:
            _wot_io_send_string(receiver, "writeproperty", sizeof("writeproperty")-1, slicer);
            break;
        case FORM_OP_OBSERVE_PROPERTY:
            _wot_io_send_string(receiver, "observeproperty", sizeof("observeproperty")-1, slicer);
            break;
        case FORM_OP_UNOBSERVE_PROPERTY:
            //Todo: Implement multi const char write function. e.g. const char unobserve + const char property
            _wot_io_send_string(receiver, "unobserveproperty", sizeof("unobserveproperty")-1, slicer);
            break;
        case FORM_OP_INVOKE_ACTION:
            _wot_io_send_string(receiver, "invokeaction", sizeof("invokeaction")-1, slicer);
            break;
        case FORM_OP_SUBSCRIBE_EVENT:
            _wot_io_send_string(receiver, "subscribeevent", sizeof("subscribeevent")-1, slicer);
            break;
        case FORM_OP_UNSUBSCRIBE_EVENT:
            _wot_io_send_string(receiver, "unsubscribeevent", sizeof("unsubscribeevent")-1, slicer);
            break;
        case FORM_OP_READ_ALL_PROPERTIES:
            _wot_io_send_string(receiver, "readallproperties", sizeof("readallproperties")-1, slicer);
            break;
        case FORM_OP_WRITE_ALL_PROPERTIES:
            _wot_io_send_string(receiver, "writeallproperties", sizeof("writeallproperties")-1, slicer);
            break;
        case FORM_OP_READ_MULTIPLE_PROPERTIES:
            _wot_io_send_string(receiver, "readmultipleproperties", sizeof("readmultipleproperties")-1, slicer);
            break;
        case FORM_OP_WRITE_MULTIPLE_PROPERTIES:
            _wot_io_send_string(receiver, "writemultipleproperties", sizeof("writemultipleproperties")-1, slicer);
            break;
        default:
            _wot_io_send_string(receiver, " ", sizeof(" "), slicer);
            break;
    }
}

void _write_op_list(wot_td_serialize_receiver_t receiver, wot_td_form_op_t *op, wot_td_ser_slicer_t *slicer){
    _wot_io_send_obj_key(receiver, _json_key_sec_def, 8, slicer);
    wot_td_array_serializer_params_t params = {
        .receiver = receiver,
        .slicer = slicer,
        .data = (wot_td_norm_array_t *)op,
        .writer = (json_writer_t)&_write_op_type,
    };
    _json_write_array(&params);
}

void _write_media_type(wot_td_serialize_receiver_t receiver, wot_td_media_type_t media_type, wot_td_ser_slicer_t *slicer){
    switch (media_type) {
        case MEDIA_TYPE_JSON:
            _wot_io_send(receiver, "application/json", sizeof("application/json")-1, slicer);
            break;
        case MEDIA_TYPE_TEXT_PLAIN:
            _wot_io_send(receiver, "text/plain", sizeof("text/plain")-1, slicer);
            break;
        case MEDIA_TYPE_JSON_LD:
            _wot_io_send(receiver, "application/ld+json", sizeof("application/ld+json")-1, slicer);
            break;
        case MEDIA_TYPE_CSV:
            _wot_io_send(receiver, "text/csv", sizeof("text/csv")-1, slicer);
            break;
        default:
            _wot_io_send_string(receiver, "", sizeof("")-1, slicer);
            break;
    }
}

void _write_content_type(wot_td_serialize_receiver_t receiver, wot_td_content_type_t *content_type, wot_td_ser_slicer_t *slicer){
    _wot_io_send(receiver, "\"", 1, slicer);
    _write_media_type(receiver, content_type->media_type, slicer);
    if(content_type->media_type_parameter != NULL){
        _wot_io_send(receiver, ";", 1, slicer);
        wot_td_media_type_parameter_t *tmp = content_type->media_type_parameter;
        while(tmp != NULL){
            _wot_io_send(receiver, tmp->key, strlen(tmp->key), slicer);
            _wot_io_send(receiver, "=", sizeof("=")-1, slicer);
            _wot_io_send(receiver, tmp->value, strlen(tmp->value), slicer);
            tmp = tmp->next;
        }
    }
    _wot_io_send(receiver, "\"", 1, slicer);
}

void _write_content_encoding(wot_td_serialize_receiver_t receiver, wot_td_content_encoding_type_t encoding, wot_td_ser_slicer_t *slicer){
    switch (encoding) {
        case CONTENT_ENCODING_GZIP:
            _wot_io_send_string(receiver, "gzip", sizeof("gzip")-1, slicer);
            break;
        case CONTENT_ENCODING_COMPRESS:
            _wot_io_send_string(receiver, "compress", sizeof("compress")-1, slicer);
            break;
        case CONTENT_ENCODING_DEFLATE:
            _wot_io_send_string(receiver, "deflate", sizeof("deflate")-1, slicer);
            break;
        case CONTENT_ENCODING_IDENTITY:
            _wot_io_send_string(receiver, _json_key_identity, sizeof(_json_key_identity)-1, slicer);
            break;
        case CONTENT_ENCODING_BROTLI:
            _wot_io_send_string(receiver, "br", sizeof("br")-1, slicer);
            break;
        default:
            _wot_io_send_string(receiver, "", sizeof("")-1, slicer);
            break;
    }
}

void _write_expected_response(wot_td_serialize_receiver_t receiver, wot_td_expected_res_t *res, wot_td_ser_slicer_t *slicer){
    //Todo: use _wot_td_serialize_json_obj
    _wot_io_send(receiver, "{", 1, slicer);
    _wot_io_send_obj_key(receiver, "contentType", sizeof("contentType")-1, slicer);
    _write_content_type(receiver, res->content_type, slicer);
    _wot_io_send(receiver, "}", 1, slicer);
}

void _write_form_list(wot_td_serialize_receiver_t receiver, wot_td_form_t *form, wot_td_ser_slicer_t *slicer){
    //Todo: Use generic array and obj functions for it.
    wot_td_form_t *tmp = form;

    _wot_io_send(receiver, "[", 1, slicer);
    while (tmp != NULL){
        _wot_io_send(receiver, "{", 1, slicer);
        bool has_previous_prop = false;

        if(tmp->op != NULL){
            has_previous_prop = true;
            _wot_io_send_obj_key(receiver, _json_key_op, sizeof(_json_key_op)-1, slicer);
            _write_op_list(receiver, tmp->op, slicer);
        }

        if(tmp->href != NULL && tmp->href->value != NULL){
            _previous_prop_check(receiver, has_previous_prop, slicer);
            has_previous_prop = true;
            _wot_io_send_obj_key(receiver, _json_key_href, sizeof(_json_key_href)-1, slicer);
            _wot_io_send_uri(receiver, tmp->href, slicer);
        }

        if(tmp->content_type != NULL){
            _previous_prop_check(receiver, has_previous_prop, slicer);
            has_previous_prop = true;
            _wot_io_send_obj_key(receiver, _json_key_content_type, sizeof(_json_key_content_type)-1, slicer);
            _write_content_type(receiver, tmp->content_type, slicer);
        }

        if(tmp->content_encoding != CONTENT_ENCODING_NONE){
            _previous_prop_check(receiver, has_previous_prop, slicer);
            has_previous_prop = true;
            _wot_io_send_obj_key(receiver, _json_key_content_coding, sizeof(_json_key_content_coding)-1, slicer);
            _write_content_encoding(receiver, tmp->content_encoding, slicer);
        }

        if(tmp->sub_protocol != NULL){
            _previous_prop_check(receiver, has_previous_prop, slicer);
            has_previous_prop = true;
            _wot_io_send_obj_key(receiver, _json_key_subprotocol, sizeof(_json_key_subprotocol)-1, slicer);
            _wot_io_send_string(receiver, tmp->sub_protocol, strlen(tmp->sub_protocol), slicer);
        }

        if(tmp->security != NULL){
            _previous_prop_check(receiver, has_previous_prop, slicer);
            has_previous_prop = true;

            _wot_io_send_obj_key(receiver, _json_key_sec_def, sizeof(_json_key_sec_def)-9, slicer);
            wot_td_array_serializer_params_t params = {
                .receiver = receiver,
                .slicer = slicer,
                .data = (wot_td_norm_array_t *)tmp->security,
                .writer = (json_writer_t)&_wot_td_security_writer,
            };
            _json_write_array(&params);
        }

        if(tmp->scopes != NULL){
            _previous_prop_check(receiver, has_previous_prop, slicer);
            has_previous_prop = true;
            _wot_io_send_obj_key(receiver, _json_key_scopes, sizeof(_json_key_scopes)-1, slicer);
            wot_td_auth_scopes_t *scope = tmp->scopes;
            _wot_io_send(receiver, "[", 1, slicer);
            while(scope != NULL){
                _wot_io_send_string(receiver, scope->value, strlen(scope->value), slicer);
                scope = scope->next;
            }
            _wot_io_send(receiver, "]", 1, slicer);
        }

        if(tmp->expected_response != NULL){
            _previous_prop_check(receiver, has_previous_prop, slicer);
            has_previous_prop = true;
            _wot_io_send_obj_key(receiver, "response", sizeof("response")-1, slicer);
        }

        if(tmp->extensions != NULL){
            _previous_prop_check(receiver, has_previous_prop, slicer);
            has_previous_prop = true;
            wot_td_extension_t *extension = tmp->extensions;
            while(extension != NULL){
                wot_td_ser_parser_t parser = extension->parser;
                parser(receiver, extension->name, extension->data);
                extension = extension->next;
            }
        }

        _wot_io_send(receiver, "}", 1, slicer);
        if(tmp->next != NULL){
            _wot_io_send(receiver, ",", 1, slicer);
        }
        tmp = tmp->next;
    }
    _wot_io_send(receiver, "]", 1, slicer);
}

void _write_data_schema(wot_td_serialize_receiver_t receiver, wot_td_data_schema_t *data_schema, char *lang, bool as_obj, wot_td_ser_slicer_t *slicer);

void _write_int_aff(wot_td_serialize_receiver_t receiver, wot_td_int_affordance_t *int_aff, char *lang, wot_td_ser_slicer_t *slicer){

    _wot_io_send_obj_key(receiver, _json_key_forms, sizeof(_json_key_forms)-1, slicer);
    _write_form_list(receiver, int_aff->forms, slicer);
    if(int_aff->titles != NULL){
        _wot_io_send(receiver, ",", 1, slicer);
        _write_title_list(receiver, int_aff->titles, lang, slicer);
    }

    if(int_aff->descriptions != NULL){
        _wot_io_send(receiver, ",", 1, slicer);
        _write_description_list(receiver, int_aff->descriptions, lang, slicer);
    }

    if(int_aff->uri_variables != NULL){
        _wot_io_send(receiver, ",", 1, slicer);
        _wot_io_send_obj_key(receiver, "uriVariables", sizeof("uriVariables")-1, slicer);
        wot_td_data_schema_map_t *data_map = int_aff->uri_variables;
        _wot_io_send(receiver, "{", 1, slicer);
        while (data_map != NULL){
            _wot_io_send_obj_key(receiver, data_map->key, strlen(data_map->key), slicer);
            _write_data_schema(receiver, data_map->value, lang, true, slicer);
            data_map = data_map->next;
        }
        _wot_io_send(receiver, "}", 1, slicer);
    }

    if(int_aff->type != NULL){
        _wot_io_send(receiver, ",", 1, slicer);
        wot_td_type_t *type = int_aff->type;
        _write_type_list(receiver, type, slicer);
    }

}

void _write_prop_aff(wot_td_serialize_receiver_t receiver, wot_td_prop_affordance_t *prop_aff, char *lang, wot_td_ser_slicer_t *slicer){
    _wot_io_send(receiver, "{", 1, slicer);
    _wot_io_send_obj_key(receiver, _json_key_observable, sizeof(_json_key_observable)-1, slicer);
    _wot_io_send_bool(receiver, prop_aff->observable, slicer);
    _wot_io_send(receiver, ",", 1, slicer);
    if(prop_aff->data_schema != NULL){
        _write_data_schema(receiver, prop_aff->data_schema, lang, false, slicer);
        _wot_io_send(receiver, ",", 1, slicer);
    }
    _write_int_aff(receiver, prop_aff->int_affordance, lang, slicer);
    _wot_io_send(receiver, "}", 1, slicer);
}

void _write_prop_aff_list(wot_td_serialize_receiver_t receiver, wot_td_prop_affordance_t *prop_aff, char *lang, wot_td_ser_slicer_t *slicer){
    wot_td_prop_affordance_t *tmp = prop_aff;

    _wot_io_send_obj_key(receiver, _json_key_properties, sizeof(_json_key_properties)-1, slicer);
    _wot_io_send(receiver, "{", 1, slicer);
    while(tmp != NULL){
        _wot_io_send_obj_key(receiver, tmp->key, strlen(tmp->key), slicer);
        _write_prop_aff(receiver, tmp, lang, slicer);
        if(tmp->next != NULL){
            _wot_io_send(receiver, ",", 1, slicer);
        }
        tmp = tmp->next;
    }

    _wot_io_send(receiver, "}", 1, slicer);
}


void _write_data_schema_object(wot_td_serialize_receiver_t receiver, wot_td_object_schema_t *schema, char *lang, wot_td_ser_slicer_t *slicer){
    _wot_io_send_obj_key(receiver, _json_key_properties, sizeof(_json_key_properties)-1, slicer);
    wot_td_data_schema_map_t *property = schema->properties;
    _wot_io_send(receiver, "{", 1, slicer);
    while (property != NULL){
        _wot_io_send_obj_key(receiver, property->key, strlen(property->key), slicer);
        _write_data_schema(receiver, property->value, lang, true, slicer);
        if(property->next != NULL){
            _wot_io_send(receiver, ",", 1, slicer);
        }
        property = property->next;
    }
    _wot_io_send(receiver, "}", 1, slicer);
    _wot_io_send(receiver, ",", 1, slicer);
    _wot_io_send_obj_key(receiver, _json_key_required, sizeof(_json_key_required)-1, slicer);
    wot_td_object_required_t *required = schema->required;
    _wot_io_send(receiver, "[", 1, slicer);
    while (required != NULL){
        _wot_io_send_string(receiver, required->value, strlen(required->value), slicer);
        required = required->next;
    }
    _wot_io_send(receiver, "]", 1, slicer);
}

void _write_data_schema_list(wot_td_serialize_receiver_t receiver, wot_td_array_schema_t *schema, char *lang, wot_td_ser_slicer_t *slicer){
    if(schema->items != NULL){
        _wot_io_send(receiver, ",", 1, slicer);
        _wot_io_send_obj_key(receiver, _json_key_items, sizeof(_json_key_items)-1, slicer);
        wot_td_data_schemas_t *item = schema->items;
        _wot_io_send(receiver, "[", 1, slicer);
        while (item != NULL){
            _write_data_schema(receiver, item->value, lang, true, slicer);
            if(item->next != NULL){
                _wot_io_send(receiver, ",", 1, slicer);
            }
            item = item->next;
        }
        _wot_io_send(receiver, "]", 1, slicer);
    }

    if(schema->min_items != NULL){
        if(schema->items != NULL){
            _wot_io_send(receiver, ",", 1, slicer);
        }
        _wot_io_send_obj_key(receiver, _json_key_min_items, sizeof(_json_key_min_items) - 1, slicer);
        char min_items_output[32];
        _itoa(*schema->min_items, min_items_output);
        _wot_io_send(receiver, min_items_output, strlen(min_items_output), slicer);
    }

    if(schema->max_items != NULL){
        if(schema->min_items != NULL || schema->items != NULL){
            _wot_io_send(receiver, ",", 1, slicer);
        }
        _wot_io_send_obj_key(receiver, _json_key_max_items, sizeof(_json_key_max_items)-1, slicer);
        char max_items_output[32];
        _itoa(*schema->max_items, max_items_output);
        _wot_io_send(receiver, max_items_output, strlen(max_items_output), slicer);
    }
}

void _serialize_data_schema_number(wot_td_serialize_receiver_t receiver, wot_td_number_schema_t *schema, wot_td_ser_slicer_t *slicer){
    char buf[16];
    bool has_minimum = false;
    if(schema->minimum != NULL){
        has_minimum = true;
        _wot_io_send_obj_key(receiver, _json_key_minimum, sizeof(_json_key_minimum)-1, slicer);
        sprintf(buf, "%f", *schema->minimum);
    }
    if(schema->maximum != NULL){
        if(has_minimum){
            _wot_io_send(receiver, ",", 1, slicer);
        }

        _wot_io_send_obj_key(receiver, _json_key_maximum, sizeof(_json_key_maximum)-1, slicer);
        memset(buf, 0, 16);
        sprintf(buf, "%f", *schema->maximum);
    }
}

//Todo: Use const vars
void _serialize_data_schema_int(wot_td_serialize_receiver_t receiver, wot_td_integer_schema_t *schema, wot_td_ser_slicer_t *slicer){
    if(schema->minimum != NULL){
        _wot_io_send_obj_key(receiver, _json_key_minimum, sizeof(_json_key_minimum)-1, slicer);
        char min_output[23];
        _itoa(*schema->minimum, min_output);
        _wot_io_send(receiver, min_output, strlen(min_output), slicer);
        _wot_io_send(receiver, ",", 1, slicer);

        _wot_io_send_obj_key(receiver, _json_key_maximum, sizeof(_json_key_maximum)-1, slicer);
        char max_output[23];
        _itoa(*schema->minimum, max_output);
        _wot_io_send(receiver, max_output, strlen(max_output), slicer);
    }
}

void _serialize_data_schema_subclass(wot_td_serialize_receiver_t receiver, wot_td_data_schema_t *data_schema, char *lang, wot_td_ser_slicer_t *slicer){
    switch (data_schema->json_type) {
        case JSON_TYPE_OBJECT:
            _write_data_schema_object(receiver, (wot_td_object_schema_t *) data_schema->schema, lang, slicer);
            break;
        case JSON_TYPE_ARRAY:
            _write_data_schema_list(receiver, (wot_td_array_schema_t *) data_schema->schema, lang, slicer);
            break;
        case JSON_TYPE_NUMBER:
            _serialize_data_schema_number(receiver, (wot_td_number_schema_t *) data_schema->schema, slicer);
            break;
        case JSON_TYPE_INTEGER:
            _serialize_data_schema_int(receiver, (wot_td_integer_schema_t *) data_schema->schema, slicer);
            break;
        default:
            return;
    }
}

void _serialize_json_type(wot_td_serialize_receiver_t receiver, wot_td_json_type_t json_type, wot_td_ser_slicer_t *slicer) {
    _wot_io_send_obj_key(receiver, _obj_key_at_type+1, sizeof(_obj_key_at_type)-2, slicer);
    switch(json_type){
        case JSON_TYPE_ARRAY:
            _wot_io_send_string(receiver, "array", sizeof("array")-1, slicer);
            break;
        case JSON_TYPE_OBJECT:
            _wot_io_send_string(receiver, "object", sizeof("object")-1, slicer);
            break;
        case JSON_TYPE_NUMBER:
            _wot_io_send_string(receiver, "number", sizeof("number")-1, slicer);
            break;
        case JSON_TYPE_INTEGER:
            _wot_io_send_string(receiver, "integer", sizeof("integer")-1, slicer);
            break;
        case JSON_TYPE_NULL:
            _wot_io_send_string(receiver, "null", sizeof("null")-1, slicer);
            break;
        case JSON_TYPE_BOOLEAN:
            _wot_io_send_string(receiver, "boolean", sizeof("boolean")-1, slicer);
            break;
        case JSON_TYPE_STRING:
            _wot_io_send_string(receiver, "string", sizeof("string")-1, slicer);
            break;
        default:
            return;
    }
}

void _write_data_schema(wot_td_serialize_receiver_t receiver, wot_td_data_schema_t *data_schema, char *lang, bool as_obj, wot_td_ser_slicer_t *slicer){
    bool has_previous_prop = false;
    if(as_obj){
        _wot_io_send(receiver, "{", 1, slicer);
    }

    if(data_schema->type != NULL){
        has_previous_prop = true;
        wot_td_type_t *type = data_schema->type;
        _write_type_list(receiver, type, slicer);
    }

    if(data_schema->titles != NULL){
        _previous_prop_check(receiver, has_previous_prop, slicer);
        has_previous_prop = true;
        _write_title_list(receiver, data_schema->titles, lang, slicer);
    }

    if(data_schema->descriptions != NULL){
        _previous_prop_check(receiver, has_previous_prop, slicer);
        has_previous_prop = true;
        _write_description_list(receiver, data_schema->descriptions, lang, slicer);
    }

    if(data_schema->json_type != JSON_TYPE_NONE){
        _previous_prop_check(receiver, has_previous_prop, slicer);
        has_previous_prop = true;
        _serialize_json_type(receiver, data_schema->json_type, slicer);
        _serialize_data_schema_subclass(receiver, data_schema, lang, slicer);
    }

    if(data_schema->constant != NULL){
        _previous_prop_check(receiver, has_previous_prop, slicer);
        has_previous_prop = true;
        _wot_io_send_obj_key(receiver, _json_key_const, sizeof(_json_key_const)-1, slicer);
        _wot_io_send_string(receiver, data_schema->constant, strlen(data_schema->constant), slicer);
    }

    if(data_schema->unit != NULL){
        _previous_prop_check(receiver, has_previous_prop, slicer);
        has_previous_prop = true;
        _wot_io_send_obj_key(receiver, _json_key_unit, sizeof(_json_key_unit)-1, slicer);
        _wot_io_send_string(receiver, data_schema->unit, strlen(data_schema->unit), slicer);
    }

    if(data_schema->one_of != NULL){
        _previous_prop_check(receiver, has_previous_prop, slicer);
        has_previous_prop = true;
        _wot_io_send_obj_key(receiver, _json_key_one_of, sizeof(_json_key_one_of)-1, slicer);
        wot_td_data_schemas_t *tmp = data_schema->one_of;
        _wot_io_send(receiver, "[", 1, slicer);
        while (tmp != NULL){
            _write_data_schema(receiver, tmp->value, lang, true, slicer);
            tmp = tmp->next;
        }
        _wot_io_send(receiver, "]", 1, slicer);
    }

    if(data_schema->enumeration != NULL){
        _previous_prop_check(receiver, has_previous_prop, slicer);
        has_previous_prop = true;
        _wot_io_send_obj_key(receiver, _json_key_enum, sizeof(_json_key_enum)-1, slicer);
        wot_td_data_enums_t *tmp = data_schema->enumeration;
        _wot_io_send(receiver, "[", 1, slicer);
        while (tmp != NULL){
            _wot_io_send_string(receiver, tmp->value, strlen(tmp->value), slicer);
            if (tmp->next != NULL){
                _wot_io_send(receiver, ",", 1, slicer);
            }
            tmp = tmp->next;
        }
        _wot_io_send(receiver, "]", 1, slicer);
    }

    _previous_prop_check(receiver, has_previous_prop, slicer);
    _wot_io_send_obj_key(receiver, _json_key_read_only, sizeof(_json_key_read_only)-1, slicer);
    _wot_io_send_bool(receiver, data_schema->read_only, slicer);

    _wot_io_send(receiver, ",", 1, slicer);
    _wot_io_send_obj_key(receiver, _json_key_write_only, sizeof(_json_key_write_only)-1, slicer);
    _wot_io_send_bool(receiver, data_schema->write_only, slicer);

    if(data_schema->format != NULL){
        _wot_io_send_string(receiver, ",", 1, slicer);
        _wot_io_send_obj_key(receiver, _json_key_format, sizeof(_json_key_format)-1, slicer);
        _wot_io_send_string(receiver, data_schema->format, strlen(data_schema->format), slicer);
    }

    if(as_obj){
        _wot_io_send(receiver, "}", 1, slicer);
    }
}

void _serialize_action_aff_array(wot_td_serialize_receiver_t receiver, wot_td_action_affordance_t *action_aff, char *lang, wot_td_ser_slicer_t *slicer){
    wot_td_action_affordance_t *tmp = action_aff;

    _wot_io_send_obj_key(receiver, _json_key_actions, sizeof(_json_key_actions)-1, slicer);
    _wot_io_send(receiver, "{", 1, slicer);
    while (tmp != NULL){
        _wot_io_send_obj_key(receiver, tmp->key, strlen(tmp->key), slicer);
        _wot_io_send(receiver, "{", 1, slicer);

        _wot_io_send_obj_key(receiver, _json_key_safe, sizeof(_json_key_safe)-1, slicer);
        _wot_io_send_bool(receiver, tmp->safe, slicer);
        _wot_io_send(receiver, ",", 1, slicer);
        _wot_io_send_obj_key(receiver, _json_key_idempotent, sizeof(_json_key_idempotent)-1, slicer);
        _wot_io_send_bool(receiver, tmp->idempotent, slicer);

        if(tmp->input != NULL){
            _wot_io_send(receiver, ",", 1, slicer);
            _wot_io_send_obj_key(receiver, _json_key_input, sizeof(_json_key_input)-1, slicer);
            _write_data_schema(receiver, tmp->input, lang, true, slicer);
        }

        if(tmp->output != NULL){
            _wot_io_send(receiver, ",", 1, slicer);
            _wot_io_send_obj_key(receiver, _json_key_output, sizeof(_json_key_output)-1, slicer);
            _write_data_schema(receiver,tmp->output, lang, true, slicer);
        }

        _wot_io_send(receiver, ",", 1, slicer);
        _write_int_aff(receiver, tmp->int_affordance, lang, slicer);
        _wot_io_send(receiver, "}", 1, slicer);
        if(tmp->next != NULL){
            _wot_io_send(receiver, ",", 1, slicer);
        }
        tmp = tmp->next;
    }

    _wot_io_send(receiver, "}", 1, slicer);
}

void _serialize_event_aff_array(wot_td_serialize_receiver_t receiver, wot_td_event_affordance_t *event_aff, char *lang, wot_td_ser_slicer_t *slicer){
    wot_td_event_affordance_t *tmp = event_aff;
    bool has_previous_prop = false;

    _wot_io_send_obj_key(receiver, _json_key_events, sizeof(_json_key_events)-1, slicer);
    _wot_io_send(receiver, "{", 1, slicer);
    while (tmp != NULL){
        _wot_io_send_obj_key(receiver, tmp->key, strlen(tmp->key), slicer);
        _wot_io_send(receiver, "{", 1, slicer);
        if(tmp->subscription != NULL){
            has_previous_prop = true;
            _wot_io_send_obj_key(receiver, _json_key_subscription, sizeof(_json_key_subscription)-1, slicer);
            _write_data_schema(receiver, tmp->subscription, lang, true, slicer);
        }
        if(tmp->data != NULL){
            _previous_prop_check(receiver, has_previous_prop, slicer);
            has_previous_prop = true;
            _wot_io_send_obj_key(receiver, _json_key_data, sizeof(_json_key_data)-1, slicer);
            _write_data_schema(receiver, tmp->data, lang, true, slicer);
        }
        if(tmp->cancellation != NULL){
            _previous_prop_check(receiver, has_previous_prop, slicer);
            has_previous_prop = true;
            _wot_io_send_obj_key(receiver, _json_key_cancellation, sizeof(_json_key_cancellation)-1, slicer);
            _write_data_schema(receiver, tmp->cancellation, lang, true, slicer);
        }
        _previous_prop_check(receiver, has_previous_prop, slicer);
        _write_int_aff(receiver, tmp->int_affordance, lang, slicer);
        _wot_io_send(receiver, "}", 1, slicer);
        if(tmp->next != NULL){
            _wot_io_send(receiver, ",", 1, slicer);
        }
        tmp = tmp->next;
    }
    _wot_io_send(receiver, "}", 1, slicer);
}

void _serialize_link_array(wot_td_serialize_receiver_t receiver, wot_td_link_t *links, wot_td_ser_slicer_t *slicer){
    wot_td_link_t *tmp = links;
    _wot_io_send(receiver, "[", 1, slicer);

    while(tmp != NULL){
        _wot_io_send(receiver, "{", 1, slicer);
        _wot_io_send_obj_key(receiver, _json_key_href, sizeof(_json_key_href)-1, slicer);
        _wot_io_send_uri(receiver, tmp->href, slicer);

        if(tmp->type != MEDIA_TYPE_NONE){
            _wot_io_send(receiver, "{", 1, slicer);
            _wot_io_send_obj_key(receiver, "type", sizeof("type")-1, slicer);
            _write_media_type(receiver, tmp->type, slicer);
            _wot_io_send(receiver, "}", 1, slicer);
        }

        if(tmp->rel != NULL){
            _wot_io_send(receiver, ",", 1, slicer);
            _wot_io_send_obj_key(receiver, _json_key_rel, sizeof(_json_key_rel)-1, slicer);
            _wot_io_send_string(receiver, tmp->rel, strlen(tmp->rel), slicer);
        }

        if(tmp->anchor != NULL){
            _wot_io_send(receiver, ",", 1, slicer);
            _wot_io_send_obj_key(receiver, _json_key_anchor, sizeof(_json_key_anchor)-1, slicer);
            _wot_io_send_uri(receiver, tmp->anchor, slicer);
        }

        _wot_io_send(receiver, "}", 1, slicer);
        if(tmp->next != NULL){
            _wot_io_send(receiver, ",", 1, slicer);
        }
        tmp = tmp->next;
    }

    _wot_io_send(receiver, "]", 1, slicer);
}

int wot_td_serialize_thing(wot_td_serialize_receiver_t receiver, wot_td_thing_t *thing, wot_td_ser_slicer_t *slicer){
    //Todo: Check for all necessary properties, before continue processing

    _wot_io_send(receiver, "{", 1, slicer);

    bool has_previous_prop = true;
    _wot_io_send_obj_key(receiver, _json_key_at_context, sizeof(_json_key_at_context)-1, slicer);

    if(thing->context != NULL){
        _wot_io_send(receiver, "[", 1, slicer);
        _wot_io_send_string(receiver, _wot_td_url, sizeof(_wot_td_url)-1, slicer);
        _wot_io_send(receiver, ",", 1, slicer);
        _write_context_list(receiver, thing->context, slicer);
        _wot_io_send(receiver, "]", 1, slicer);
    }else{
        _wot_io_send_string(receiver, _wot_td_url, sizeof(_wot_td_url)-1, slicer);
    }

    if(thing->security_def != NULL){
        _previous_prop_check(receiver, has_previous_prop, slicer);
        has_previous_prop = true;
        _write_security_def_list(receiver, thing->security_def, thing->default_language_tag, slicer);
    }else{
        return 1;
    }

    if(thing->security != NULL){
        _previous_prop_check(receiver, has_previous_prop, slicer);
        has_previous_prop = true;
        _serialize_security_array(receiver, thing->security, slicer);
    }else{
        return 1;
    }

    if(thing->type != NULL){
        _previous_prop_check(receiver, has_previous_prop, slicer);
        has_previous_prop = true;
        wot_td_type_t *type = thing->type;
        _write_type_list(receiver, type, slicer);
    }

    if(thing->id != NULL){
        _previous_prop_check(receiver, has_previous_prop, slicer);
        has_previous_prop = true;
        _wot_io_send_obj_key(receiver, _json_key_id, sizeof(_json_key_id)-1, slicer);
        _wot_io_send_uri(receiver, thing->id, slicer);
    }

    if(thing->titles != NULL){
        _wot_io_send(receiver, ",", 1, slicer);
        has_previous_prop = true;
        _write_title_list(receiver, thing->titles, thing->default_language_tag, slicer);
    }

    if(thing->descriptions != NULL){
        _wot_io_send(receiver, ",", 1, slicer);
        has_previous_prop = true;
        _write_description_list(receiver, thing->descriptions, thing->default_language_tag, slicer);
    }

    if(thing->properties != NULL){
        _wot_io_send(receiver, ",", 1, slicer);
        _write_prop_aff_list(receiver, thing->properties, thing->default_language_tag, slicer);
    }

    if(thing->actions != NULL){
        _wot_io_send(receiver, ",", 1, slicer);
        _serialize_action_aff_array(receiver, thing->actions, thing->default_language_tag, slicer);
    }

    if(thing->events != NULL){
        _wot_io_send(receiver, ",", 1, slicer);
        _serialize_event_aff_array(receiver, thing->events, thing->default_language_tag, slicer);
    }

    if(thing->links != NULL){
        _previous_prop_check(receiver, has_previous_prop, slicer);
        _wot_io_send_obj_key(receiver, _json_links, sizeof(_json_links)-1, slicer);
        _serialize_link_array(receiver, thing->links, slicer);
    }

    assert(thing->base != NULL);
    _previous_prop_check(receiver, has_previous_prop, slicer);
    _wot_io_send_obj_key(receiver, _json_key_base, sizeof(_json_key_base)-1, slicer);
    _wot_io_send_uri(receiver, thing->base, slicer);

    if(thing->support != NULL){
        _wot_io_send(receiver, ",", 1, slicer);
        _wot_io_send_obj_key(receiver, _json_key_support, sizeof(_json_key_support) - 1, slicer);
        _wot_io_send_uri(receiver, thing->support, slicer);
    }

    if(thing->version != NULL){
        _wot_io_send(receiver, ",", 1, slicer);
        _wot_io_send_obj_key(receiver, _json_key_version, sizeof(_json_key_version)-1, slicer);
        _wot_io_send(receiver, "{", 1, slicer);
        _wot_io_send_obj_key(receiver, _json_key_instance, sizeof(_json_key_instance)-1, slicer);
        _wot_io_send_string(receiver, thing->version->instance, strlen(thing->version->instance), slicer);
        _wot_io_send(receiver, "}", 1, slicer);
    }

    if(thing->forms != NULL){
        _wot_io_send(receiver, ",", 1, slicer);
        _wot_io_send_obj_key(receiver, _json_key_forms, sizeof(_json_key_forms)-1, slicer);
        _write_form_list(receiver, thing->forms, slicer);
    }

    if(thing->created != NULL){
        _wot_io_send(receiver, ",", 1, slicer);
        _wot_io_send_obj_key(receiver, _json_key_created, sizeof(_json_key_created)-1, slicer);
        _wot_io_send_date(receiver, thing->created, slicer);
    }

    if(thing->modified != NULL){
        _wot_io_send(receiver, ",", 1, slicer);
        _wot_io_send_obj_key(receiver, _json_key_modified, sizeof(_json_key_modified)-1, slicer);
        _wot_io_send_date(receiver, thing->modified, slicer);
    }

    _wot_io_send(receiver, "}", 1, slicer);

    return 0;
}
