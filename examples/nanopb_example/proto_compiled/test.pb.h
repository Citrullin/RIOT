/* Automatically generated nanopb header */
/* Generated by nanopb-0.4.0-dev */

#ifndef PB_TEST_PB_H_INCLUDED
#define PB_TEST_PB_H_INCLUDED
#include <pb.h>

/* @@protoc_insertion_point(includes) */
#if PB_PROTO_HEADER_VERSION != 40
#error Regenerate this file with the current version of nanopb generator.
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Struct definitions */
typedef struct _TestMessage {
    int32_t test;
/* @@protoc_insertion_point(struct:TestMessage) */
} TestMessage;


/* Initializer values for message structs */
#define TestMessage_init_default                 {0}
#define TestMessage_init_zero                    {0}

/* Field tags (for use in manual encoding/decoding) */
#define TestMessage_test_tag                     1

/* Struct field encoding specification for nanopb */
#define TestMessage_FIELDLIST(X, a) \
X(a, STATIC, REQUIRED, INT32, test, 1)
#define TestMessage_CALLBACK NULL
#define TestMessage_DEFAULT NULL

extern const pb_msgdesc_t TestMessage_msg;

/* Defines for backwards compatibility with code written before nanopb-0.4.0 */
#define TestMessage_fields &TestMessage_msg

/* Maximum encoded size of messages (where known) */
#define TestMessage_size                         11

#ifdef __cplusplus
} /* extern "C" */
#endif
/* @@protoc_insertion_point(eof) */

#endif
