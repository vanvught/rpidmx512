/**
 * @file json_infos.h
 */

#ifndef HTTP_JSON_INFOS_H_
#define HTTP_JSON_INFOS_H_

#include <cstddef>
#include <cstdint>

namespace json
{

struct Info
{
    constexpr Info(uint32_t (*get_in)(char*, uint32_t), void (*set_in)(const char*, uint32_t), void (*del_in)(const char*, uint32_t), const char* name_in, uint8_t length_in, uint32_t hash_in, const char* label_in) noexcept :
     get(get_in), 
     set(set_in), 
     del(del_in), 
     name(name_in), 
     length(length_in), 
     hash(hash_in), 
     label(label_in) {}

    uint32_t (*get)(char*, uint32_t);
    void (*set)(const char*, uint32_t);
    void (*del)(const char*, uint32_t);
    const char* name;
    uint8_t length;
    uint32_t hash;
    const char* label;
};

template <size_t N> constexpr Info MakeJsonFileInfo(uint32_t (*get)(char*, uint32_t), void (*set)(const char*, uint32_t), void (*del)(const char*, uint32_t), const char (&str)[N], uint32_t hash, const char* label)
{
    return Info{get, set, del, str, static_cast<uint8_t>(N - 1), hash, label};
}

constexpr bool HasUniqueHashes(const Info* entries, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        for (size_t j = i + 1; j < count; ++j)
        {
            if (entries[i].hash == entries[j].hash)
            {
                return false;
            }
        }
    }
    return true;
}

extern const Info kFileInfos[];
extern const size_t kFileInfosSize;

void PrintInfos();
int32_t GetFileIndex(const char* txt_filename);
void CheckHashCollisions();

} // namespace json

#endif  // HTTP_JSON_INFOS_H_
