#include <Codec/Json.hpp>

const nbl::JsonElement* nbl::JsonObject::get(nbl::StringView key) const {
    usize bucket = key.hash() % bucketCount;
    for(usize i = 0 ; i < buckets[bucket].count ; i++) {
        Entry& ent = entries[i + buckets[bucket].offset];
        if(ent.key == key) {
            return &ent.value;
        }
    }
    return nullptr;
}

