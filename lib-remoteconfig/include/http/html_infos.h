/**
 * @file html_infos.h
 */

#ifndef HTTP_HTML_INFOS_H_
#define HTTP_HTML_INFOS_H_

#include <cstddef>
#include <cstdint>

namespace html
{

struct Info
{
    constexpr Info(const char* name_in, uint8_t length_in, uint32_t hash_in, const char* label_in) noexcept
        : name(name_in), length(length_in), hash(hash_in), label(label_in)
    {
    }

    const char* name;
    uint8_t length;
    uint32_t hash;
    const char* label;
};

template <size_t N> constexpr Info MakeHtmlInfo(const char (&str)[N], uint32_t hash, const char* label)
{
    return Info{str, static_cast<uint8_t>(N - 1), hash, label};
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

extern const Info kHtmlInfos[];
extern const size_t kHtmlInfosSize;

void PrintInfos();
int32_t GetFileIndex(const char* name);
void CheckHashCollisions();

} // namespace html

#endif  // HTTP_HTML_INFOS_H_
