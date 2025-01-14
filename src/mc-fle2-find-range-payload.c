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

#include <bson/bson.h>

#include "mc-fle2-find-range-payload-private.h"
#include "mongocrypt.h"
#include "mongocrypt-buffer-private.h"

void
mc_FLE2FindRangePayload_init (mc_FLE2FindRangePayload_t *payload)
{
   BSON_ASSERT_PARAM (payload);
   *payload = (mc_FLE2FindRangePayload_t){{0}};
   _mc_array_init (&payload->edgeFindTokenSetArray,
                   sizeof (mc_EdgeFindTokenSet_t));
}

static void
mc_EdgeFindTokenSet_cleanup (mc_EdgeFindTokenSet_t *etc)
{
   if (!etc) {
      return;
   }
   _mongocrypt_buffer_cleanup (&etc->edcDerivedToken);
   _mongocrypt_buffer_cleanup (&etc->escDerivedToken);
   _mongocrypt_buffer_cleanup (&etc->eccDerivedToken);
}

void
mc_FLE2FindRangePayload_cleanup (mc_FLE2FindRangePayload_t *payload)
{
   if (!payload) {
      return;
   }
   _mongocrypt_buffer_cleanup (&payload->serverEncryptionToken);
   // Free all EdgeFindTokenSet entries.
   for (size_t i = 0; i < payload->edgeFindTokenSetArray.len; i++) {
      mc_EdgeFindTokenSet_t entry = _mc_array_index (
         &payload->edgeFindTokenSetArray, mc_EdgeFindTokenSet_t, i);
      mc_EdgeFindTokenSet_cleanup (&entry);
   }
   _mc_array_destroy (&payload->edgeFindTokenSetArray);
}

#define APPEND_BINDATA(out, name, subtype, value)           \
   if (!_mongocrypt_buffer_append (                         \
          &(value), out, name, (uint32_t) strlen (name))) { \
      return false;                                         \
   }

bool
mc_FLE2FindRangePayload_serialize (const mc_FLE2FindRangePayload_t *payload,
                                   bson_t *out)
{
   BSON_ASSERT_PARAM (out);
   BSON_ASSERT_PARAM (payload);
   // Append "g" array of EdgeTokenSets.
   bson_t g_bson;
   if (!BSON_APPEND_ARRAY_BEGIN (out, "g", &g_bson)) {
      return false;
   }

   uint32_t g_index = 0;
   for (size_t i = 0; i < payload->edgeFindTokenSetArray.len; i++) {
      mc_EdgeFindTokenSet_t etc = _mc_array_index (
         &payload->edgeFindTokenSetArray, mc_EdgeFindTokenSet_t, i);
      bson_t etc_bson;

      const char *g_index_string;
      char storage[16];
      bson_uint32_to_string (
         g_index, &g_index_string, storage, sizeof (storage));

      if (!BSON_APPEND_DOCUMENT_BEGIN (&g_bson, g_index_string, &etc_bson)) {
         return false;
      }

      APPEND_BINDATA (&etc_bson, "d", BSON_SUBTYPE_BINARY, etc.edcDerivedToken);
      APPEND_BINDATA (&etc_bson, "s", BSON_SUBTYPE_BINARY, etc.escDerivedToken);
      APPEND_BINDATA (&etc_bson, "c", BSON_SUBTYPE_BINARY, etc.eccDerivedToken);

      if (!bson_append_document_end (&g_bson, &etc_bson)) {
         return false;
      }
      g_index++;
   }

   if (!bson_append_array_end (out, &g_bson)) {
      return false;
   }

   APPEND_BINDATA (
      out, "e", BSON_SUBTYPE_BINARY, payload->serverEncryptionToken);
   if (!BSON_APPEND_INT64 (out, "cm", payload->maxContentionCounter)) {
      return false;
   }

   return true;
}
#undef APPEND_BINDATA
