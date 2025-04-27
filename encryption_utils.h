#pragma once

#include "libralink.pb.h"
#include <pb_encode.h>
#include <pb_decode.h>
#include <Arduino.h>

// Fake signature data
static const char* fakeSignature = "signed-by-device";

// Encoding function
inline bool encode_signature_callback(pb_ostream_t *stream, const pb_field_t *field, void *const *arg) {
    const char* sig = (const char*)(*arg);
    size_t len = strlen(sig);

    if (!pb_encode_tag_for_field(stream, field)) return false;
    return pb_encode_string(stream, (const pb_byte_t*)sig, len);
}

// Signing function
inline io_libralink_client_payment_proto_Envelope signEnvelope(const io_libralink_client_payment_proto_Envelope& envelope) {
    io_libralink_client_payment_proto_Envelope signedEnvelope = envelope; // Copy

    signedEnvelope.sig.funcs.encode = encode_signature_callback;
    signedEnvelope.sig.arg = (void*)fakeSignature;

    return signedEnvelope;
}

// Verification function
inline bool verifyEnvelope(const io_libralink_client_payment_proto_Envelope& envelope) {
    if (!envelope.sig.arg) return false;

    const char* sig = (const char*)(envelope.sig.arg);
    return (strcmp(sig, "signed-by-device") == 0);
}