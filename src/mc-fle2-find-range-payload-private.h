/*
 * Copyright 2022-present MongoDB, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MC_FLE2_FIND_RANGE_PAYLOAD_PRIVATE_H
#define MC_FLE2_FIND_RANGE_PAYLOAD_PRIVATE_H

#include "mongocrypt.h"
#include "mongocrypt-private.h"
#include "mongocrypt-buffer-private.h"

#include "mc-array-private.h"

/**
 * FLE2FindRangePayload represents an FLE2 payload of a range indexed field to
 * query. It is created client side.
 *
 * FLE2FindRangePayload has the following data layout:
 *
 * struct {
 *   uint8_t fle_blob_subtype = 10;
 *   uint8_t bson[];
 * } FLE2FindRangePayload;
 *
 * bson is a BSON document of this form:
 * g: array<EdgeFindTokenSet> // Array of Edges
 * e: <binary> // ServerDataEncryptionLevel1Token
 * cm: <int64> // Queryable Encryption max counter
 */
typedef struct {
   mc_array_t edgeFindTokenSetArray;           // g
   _mongocrypt_buffer_t serverEncryptionToken; // e
   int64_t maxContentionCounter;               // cm
} mc_FLE2FindRangePayload_t;

/**
 * EdgeFindTokenSet is the following BSON document:
 * d: <binary> // EDCDerivedFromDataTokenAndCounter
 * s: <binary> // ESCDerivedFromDataTokenAndCounter
 * c: <binary> // ECCDerivedFromDataTokenAndCounter
 *
 * Instances of mc_EdgeFindTokenSet_t are expected to be owned by
 * mc_FLE2FindRangePayload_t and are freed in
 * mc_FLE2FindRangePayload_cleanup.
 */
typedef struct {
   _mongocrypt_buffer_t edcDerivedToken; // d
   _mongocrypt_buffer_t escDerivedToken; // s
   _mongocrypt_buffer_t eccDerivedToken; // c
} mc_EdgeFindTokenSet_t;

void
mc_FLE2FindRangePayload_init (mc_FLE2FindRangePayload_t *payload);

bool
mc_FLE2FindRangePayload_serialize (const mc_FLE2FindRangePayload_t *payload,
                                   bson_t *out);

void
mc_FLE2FindRangePayload_cleanup (mc_FLE2FindRangePayload_t *payload);

#endif /* MC_FLE2_FIND_RANGE_PAYLOAD_PRIVATE_H */
