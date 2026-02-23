#ifndef HTTP_INFO_UTILS_H_
#define HTTP_INFO_UTILS_H_

/*
 * info_utils.h
 */

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <cstdio>

#include "common/utils/utils_hash.h"

template <typename InfoT> int32_t GetFileIndexGeneric(const InfoT* infos, size_t size, const char* name)
{
    const uint32_t kLen = static_cast<uint32_t>(std::strlen(name));
    const uint32_t kHash = Fnv1a32Runtime(name, kLen);

    for (uint32_t i = 0; i < size; ++i)
    {
        if (kHash == infos[i].hash)
        {
            return static_cast<int32_t>(i);
        }
    }

    return -1;
}

template <typename InfoT> void PrintInfosGeneric(const InfoT* infos, size_t size)
{
    for (size_t i = 0; i < size; ++i)
    {
        const auto& entry = infos[i];
        printf("File %2u: %-24s %-24s (length = %2u, hash=0x%.8x)\n", static_cast<unsigned>(i), entry.name, entry.label, static_cast<unsigned>(entry.length), entry.hash);
    }
}

template <typename InfoT> void CheckHashCollisionsGeneric(const InfoT* infos, size_t size)
{
    for (size_t i = 0; i < size; ++i)
    {
        for (size_t j = i + 1; j < size; ++j)
        {
            if (infos[i].hash == infos[j].hash)
            {
                printf("Hash collision between: %s and %s\n", infos[i].name, infos[j].name);
            }
        }
    }
}

#endif  // HTTP_INFO_UTILS_H_
