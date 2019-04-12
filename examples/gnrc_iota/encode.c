#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#include "pb_common.h"
#include "pb_decode.h"
#include "pb_encode.h"

#include "iota/transfers.h"

#include "proto_compiled/iota-transaction.pb.h"

uint8_t encode_buffer[5000];
size_t encode_message_length;
bool encode_status;

extern iota_wallet_tx_object_t * wallet_tx_object;

bool write_signature_message_fragment(pb_ostream_t *stream, const pb_field_t *field, void * const *arg)
{
    (void) arg;
    char *str = wallet_tx_object->signatureMessageFragment;
    if (!pb_encode_tag_for_field(stream, field))
        return false;

    return pb_encode_string(stream, (uint8_t*)str, NUM_SIG_MSG_TRYTES);
}

bool write_address(pb_ostream_t *stream, const pb_field_t *field, void * const *arg)
{
    (void) arg;
    char *str = wallet_tx_object->address;
    if (!pb_encode_tag_for_field(stream, field))
        return false;

    return pb_encode_string(stream, (uint8_t*)str, NUM_ADDR_TRYTES);
}

bool write_obsoleteTag(pb_ostream_t *stream, const pb_field_t *field, void * const *arg)
{
    (void) arg;
    char *str = wallet_tx_object->obsoleteTag;
    if (!pb_encode_tag_for_field(stream, field))
        return false;

    return pb_encode_string(stream, (uint8_t*)str, NUM_TAG_TRYTES);
}

int encode_iota_transaction_message(IotaTransactionMessage * message_ptr) {
    pb_callback_t address_callback;
    address_callback.funcs.encode = &write_address;
    message_ptr->address = address_callback;

    pb_callback_t signature_message_fragment_callback;
    signature_message_fragment_callback.funcs.encode = &write_signature_message_fragment;
    message_ptr->signatureMessageFragment = signature_message_fragment_callback;

    pb_callback_t obsoleteTag_callback;
    obsoleteTag_callback.funcs.encode = &write_obsoleteTag;
    message_ptr->obsoleteTag = obsoleteTag_callback;

    pb_ostream_t stream = pb_ostream_from_buffer(encode_buffer, sizeof(encode_buffer));

    encode_status = pb_encode(&stream, IotaTransactionMessage_fields, message_ptr);
    encode_message_length = stream.bytes_written;

    if (!encode_status)
    {
        printf("Encoding failed: %s\n", PB_GET_ERROR(&stream));
        return 1;
    }

    return 0;
}

int send_iota_transaction_message(
        IotaTransactionMessage * message_ptr, int socket, struct sockaddr_in6 * client_addr_ptr){
    encode_iota_transaction_message(message_ptr);
    sendto(
            socket, encode_buffer, sizeof(encode_buffer), 0,
            (struct sockaddr *)client_addr_ptr, sizeof(struct sockaddr_in6));
    return 0;
}
