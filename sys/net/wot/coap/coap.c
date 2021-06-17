#include <stddef.h>
#include <string.h>

#include "thread.h"
#include "net/ipv6/addr.h"
#include "net/wot.h"
#include "net/wot/serialization.h"
#include "net/wot/config.h"
#include "net/wot/coap/config.h"
#include "msg.h"

#define MAIN_QUEUE_SIZE     (8)
static msg_t _main_msg_queue[MAIN_QUEUE_SIZE];

static ssize_t _encode_link(const coap_resource_t *resource, char *buf,
                            size_t maxlen, coap_link_encoder_ctx_t *context);

wot_td_thing_t wot_thing;

const char wot_td_coap_schema[] = "coap://";

//Todo: Implement CoAP RDF bindings.
//See: https://github.com/w3c/wot-binding-templates/issues/97
json_ld_context_t wot_td_coap_binding_context = {
        .key = "cov",
        .value = "http://www.example.org/coap-binding#",
};

//static kernel_pid_t wot_td_coap_pid;
//static char wot_td_coap_stack[THREAD_STACKSIZE_DEFAULT + THREAD_EXTRA_STACKSIZE_PRINTF];

const char coap_get_method_name[] = "GET";
const char coap_post_method_name[] = "POST";
const char coap_put_method_name[] = "PUT";

const char wot_td_coap_method_name[] = "methodName";

coap_block_slicer_t _wot_td_coap_slicer;
ssize_t _wot_td_coap_plen;
uint8_t *_wot_td_coap_buf;

void _wot_td_coap_write_string(
        wot_td_serialize_receiver_t receiver, const char * string, size_t length){
    for(size_t i = 0; i < length; i++){
        receiver(&string[i]);
    }
}

void _wot_td_coap_write_cov_method_json_key(wot_td_serialize_receiver_t receiver){
    receiver("\"");
    _wot_td_coap_write_string(
            receiver, wot_td_coap_binding_context.key,
            sizeof(wot_td_coap_binding_context.key)-1);
    receiver(":");
    _wot_td_coap_write_string(
            receiver, wot_td_coap_method_name, sizeof(wot_td_coap_method_name)-1);
    receiver("\"");
    receiver(":");
}

void _wot_td_coap_method_ser(
        wot_td_serialize_receiver_t receiver, const char * name, void * data){
    if(name == wot_td_coap_binding_context.key){
        coap_method_flags_t *methods = (coap_method_flags_t*)data;

        _wot_td_coap_write_cov_method_json_key(receiver);

        receiver("\"");
        if(*methods & COAP_GET){
            _wot_td_coap_write_string(
                    receiver, coap_get_method_name,
                    sizeof(coap_get_method_name)-1);
        }else if(*methods & COAP_POST){
            _wot_td_coap_write_string(
                    receiver, coap_post_method_name,
                    sizeof(coap_post_method_name)-1);
        }else if(*methods & COAP_PUT){
            _wot_td_coap_write_string(
                    receiver, coap_put_method_name,
                    sizeof(coap_put_method_name)-1);
        }
        receiver("\"");
    }
}

void _wot_td_coap_ser_receiver(const char *c){
    _wot_td_coap_plen += coap_blockwise_put_char(&_wot_td_coap_slicer, _wot_td_coap_buf+_wot_td_coap_plen, (char) *c);
}

static int get_base_ip_address(ipv6_addr_t *res) {
    const int MAX_ADRESSES_TO_CHECK = 5;
    netif_t* interface = NULL;
    ipv6_addr_t local_address = {0};
    ipv6_addr_t ula_address = {0};
    bool link_local_found = false;
    bool ula_found = false;

    while ((interface = netif_iter(interface)) != NULL) {
        ipv6_addr_t addresses[MAX_ADRESSES_TO_CHECK];
        int netres = netif_get_opt(interface, NETOPT_IPV6_ADDR, 0, addresses, sizeof(addresses));
        for (unsigned i = 0; i < (netres / sizeof(ipv6_addr_t)); i++)
        {
            ipv6_addr_t* current_address = &addresses[i];

            if (current_address == NULL) {
                break;
            }
            if (ipv6_addr_is_global(current_address)) {
                memcpy(res, current_address, sizeof(ipv6_addr_t));
                return 0;
            }
            else if (ipv6_addr_is_unique_local_unicast(current_address)) {
                memcpy(&ula_address, current_address, sizeof(ipv6_addr_t));
                ula_found = true;
            }
            else if (ipv6_addr_is_link_local(current_address)) {
                memcpy(&local_address, current_address, sizeof(ipv6_addr_t));
                link_local_found = true;
            }
        }
    }

    if (ula_found) {
        memcpy(res, &ula_address, sizeof(ipv6_addr_t));
        return 0;
    }
    else if (link_local_found) {
        memcpy(res, &local_address, sizeof(ipv6_addr_t));
        return 0;
    }
    return -1;
}

static ssize_t _wot_td_coap_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, void *ctx){
    (void)ctx;

    _wot_td_coap_buf = buf;
    memset(&_wot_td_coap_slicer, 0, sizeof(coap_block_slicer_t));
    coap_block2_init(pdu, &_wot_td_coap_slicer);

    gcoap_resp_init(pdu, _wot_td_coap_buf, len, COAP_CODE_CONTENT);
    coap_opt_add_format(pdu, COAP_FORMAT_WOT_TD);
    coap_opt_add_block2(pdu, &_wot_td_coap_slicer, 1);
    _wot_td_coap_plen = coap_opt_finish(pdu, COAP_OPT_FINISH_PAYLOAD);

    wot_td_ser_slicer_t _wot_td_slicer = {
            .start = _wot_td_coap_slicer.start,
            .end = _wot_td_coap_slicer.end,
    };

    char base_address[IPV6_ADDR_MAX_STR_LEN + 2] = "[";
    char address_as_string[IPV6_ADDR_MAX_STR_LEN];
    ipv6_addr_t base_ip_address = {0};
    if (get_base_ip_address(&base_ip_address) < 0) {
        return -1;
    }
    ipv6_addr_to_str(address_as_string, &base_ip_address, sizeof(address_as_string));
    strcat(base_address, address_as_string);
    strcat(base_address, "]");

    wot_td_uri_t _wot_thing_base = {
            .schema = wot_td_coap_schema,
            .value = base_address,
    };

    wot_thing.base = &_wot_thing_base;

    wot_td_serialize_thing((wot_td_serialize_receiver_t) &_wot_td_coap_ser_receiver, &wot_thing, &_wot_td_slicer);

    coap_block2_finish(&_wot_td_coap_slicer);

    return _wot_td_coap_plen;
}

//Todo: Implement WoT-discovery.
//https://w3c.github.io/wot-discovery/
static const coap_resource_t _wot_td_coap_resources[] = {
        { "/.well-known/wot-thing-description", COAP_GET, _wot_td_coap_handler, NULL },
};

static const char *_wot_td_link_params[] = {
    ";rt=\"wot.thing\";ct=432",
};

/* Adds link format params to resource list. TODO: Can probably be streamlined */
static ssize_t _encode_link(const coap_resource_t *resource, char *buf,
                            size_t maxlen, coap_link_encoder_ctx_t *context) {
    ssize_t res = gcoap_encode_link(resource, buf, maxlen, context);
    if (res > 0) {
        if (_wot_td_link_params[context->link_pos]
                && (strlen(_wot_td_link_params[context->link_pos]) < (maxlen - res))) {
            if (buf) {
                memcpy(buf+res, _wot_td_link_params[context->link_pos],
                       strlen(_wot_td_link_params[context->link_pos]));
            }
            return res + strlen(_wot_td_link_params[context->link_pos]);
        }
    }

    return res;
}

gcoap_listener_t _wot_td_gcoap_listener = {
        &_wot_td_coap_resources[0],
        ARRAY_SIZE(_wot_td_coap_resources),
        _encode_link,
        NULL,
        NULL
};

gcoap_listener_t * _find_last_gcoap_listener(void){
    gcoap_listener_t *listener = &_wot_td_gcoap_listener;
    while(listener->next != NULL){
        listener = listener->next;
    }
    return listener;
}

//Todo: Add COAP_MATCH_SUBTREE to href.
//Todo: Possible problem when other module needs to add an additional extension to it. Think about an elegant solution for this.
void _add_method_to_form(wot_td_form_t * form, const coap_resource_t * resource){
    wot_td_extension_t *extension;
    extension = form->extensions;
    extension->name = wot_td_coap_binding_context.key;
    extension->data = &(resource->methods);
    extension->parser = (wot_td_ser_parser_t) &_wot_td_coap_method_ser;
}

void _add_endpoint_to_form(wot_td_form_t * form, const coap_resource_t * resource){
    form->href->value = resource->path;
}

void wot_td_coap_add_forms(wot_td_coap_form_t *coap_forms){
    wot_td_coap_form_t *tmp = coap_forms;
    while(tmp != NULL){
        _add_endpoint_to_form(tmp->form, tmp->coap_resource);
        _add_method_to_form(tmp->form, tmp->coap_resource);
        tmp = tmp->next;
    }
}

void wot_td_coap_server_init(void)
{
    wot_td_thing_context_add(&wot_thing, &wot_td_coap_binding_context);
    msg_init_queue(_main_msg_queue, MAIN_QUEUE_SIZE);

    wot_td_config_init(&wot_thing);
    wot_td_coap_config_init(&wot_thing);

    gcoap_init();
    gcoap_register_listener(&_wot_td_gcoap_listener);
}
