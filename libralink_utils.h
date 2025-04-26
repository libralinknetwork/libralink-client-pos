#pragma once

#include "libralink.pb.h"
#include <pb_encode.h>
#include <base64.h>
#include <Arduino.h>
#include <time.h>

// Encodes a string field for nanopb
inline bool encode_string_callback(pb_ostream_t *stream, const pb_field_t *field, void *const *arg) {
    const char *str = (const char *)(*arg);
    if (!pb_encode_tag_for_field(stream, field))
        return false;
    return pb_encode_string(stream, (const pb_byte_t *)str, strlen(str));
}

// Encodes a varint field for nanopb
inline bool encode_varint_callback(pb_ostream_t *stream, const pb_field_t *field, void *const *arg) {
    uint64_t value = *(uint64_t*)(*arg);
    if (!pb_encode_tag_for_field(stream, field))
        return false;
    return pb_encode_varint(stream, value);
}

// Encodes a nested message (submessage) for nanopb
inline bool encode_submessage_callback(pb_ostream_t *stream, const pb_field_t *field, void *const *arg) {
    if (!pb_encode_tag_for_field(stream, field))
        return false;
    return pb_encode_submessage(stream, field->submsg_desc, *arg);
}

// Generates a PaymentRequest, wraps it inside EnvelopeContent, wraps that inside Envelope, and returns base64-encoded result
inline String generatePaymentRequestBase64(const String& amount, const String& from, const String& to, const String& processor, const String& correlationId) {
    io_libralink_client_payment_proto_PaymentRequest paymentRequest = io_libralink_client_payment_proto_PaymentRequest_init_zero;

    paymentRequest.amount.funcs.encode = &encode_string_callback;
    paymentRequest.amount.arg = (void*) amount.c_str();

    paymentRequest.from.funcs.encode = &encode_string_callback;
    paymentRequest.from.arg = (void*) from.c_str();

    paymentRequest.fromProc.funcs.encode = &encode_string_callback;
    paymentRequest.fromProc.arg = (void*) processor.c_str();

    paymentRequest.to.funcs.encode = &encode_string_callback;
    paymentRequest.to.arg = (void*) to.c_str();

    paymentRequest.toProc.funcs.encode = &encode_string_callback;
    paymentRequest.toProc.arg = (void*) processor.c_str();

    paymentRequest.currency.funcs.encode = &encode_string_callback;
    paymentRequest.currency.arg = (void*) "USDC";

    paymentRequest.created_at = (int64_t)time(NULL);

    paymentRequest.note.funcs.encode = &encode_string_callback;
    paymentRequest.note.arg = (void*) "Order note";

    paymentRequest.correlationId.funcs.encode = &encode_string_callback;
    paymentRequest.correlationId.arg = (void*) correlationId.c_str();

    io_libralink_client_payment_proto_EnvelopeContent envelopeContent = io_libralink_client_payment_proto_EnvelopeContent_init_zero;

    envelopeContent.address.funcs.encode = &encode_string_callback;
    envelopeContent.address.arg = (void*)"0xToAddress";

    envelopeContent.algorithm.funcs.encode = &encode_string_callback;
    envelopeContent.algorithm.arg = (void*)"secp256k1";

    static uint64_t reason_value = io_libralink_client_payment_proto_SignatureReason_CONFIRM;
    envelopeContent.reason.funcs.encode = &encode_varint_callback;
    envelopeContent.reason.arg = &reason_value;

    envelopeContent.which_entity = io_libralink_client_payment_proto_EnvelopeContent_paymentRequest_tag;
    envelopeContent.entity.paymentRequest.funcs.encode = &encode_submessage_callback;
    envelopeContent.entity.paymentRequest.arg = &paymentRequest;

    io_libralink_client_payment_proto_Envelope envelope = io_libralink_client_payment_proto_Envelope_init_zero;
    envelope.id.funcs.encode = &encode_string_callback;
    envelope.id.arg = (void*)"unique-envelope-id-123";

    envelope.has_content = true;
    envelope.content = envelopeContent;

    envelope.sig.funcs.encode = &encode_string_callback;
    envelope.sig.arg = (void*)"";

    uint8_t buffer[1024];
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

    if (!pb_encode(&stream, io_libralink_client_payment_proto_Envelope_fields, &envelope)) {
        Serial.println("Encoding Envelope failed");
        return "";
    }

    return base64::encode(buffer, stream.bytes_written);
}
