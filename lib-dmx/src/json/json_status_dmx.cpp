/**
 * @file json_status_dmx.cpp
 */
/* Copyright (C) 2025 by Arjan van Vught mailto:info@gd32-dmx.org */ 

#include <cstdint>
#include <cstdio>

#include "dmx.h"
#include "dmxnode.h"

namespace json::status
{
static uint32_t PortInfo(char* out_buffer, uint32_t out_buffer_size, uint32_t port_index)
{
    const auto kDirection = Dmx::Get()->GetPortDirection(port_index) == ::dmx::PortDirection::kInput ? ::dmxnode::PortDirection::kInput : ::dmxnode::PortDirection::kOutput;
    auto length = static_cast<uint32_t>(snprintf(out_buffer, out_buffer_size, 
    "{\"port\":\"%c\",\"direction\":\"%s\"},", 
    static_cast<char>('A' + port_index), 
    dmxnode::GetPortDirection(kDirection)));

    return length;
}

uint32_t Dmx(char* out_buffer, uint32_t out_buffer_size) {
    out_buffer[0] = '[';
    uint32_t length = 1;

    for (uint32_t port_index = 0; port_index < ::dmx::config::max::PORTS; port_index++)
    {
        length += PortInfo(&out_buffer[length], out_buffer_size - length, port_index);
    }

    out_buffer[length - 1] = ']';

    return length;	
}
uint32_t Dmx(char* out_buffer, uint32_t out_buffer_size, uint32_t port_index) {
    if (port_index < ::dmx::config::max::PORTS)
    {
        auto& statistics = Dmx::Get()->GetTotalStatistics(port_index);
        auto length = static_cast<uint32_t>(snprintf(out_buffer, out_buffer_size,
         "{\"port\":\"%c\","
         "\"dmx\":{\"sent\":\"%u\",\"received\":\"%u\"},"
         "\"rdm\":{\"sent\":{\"class\":\"%u\",\"discovery\":\"%u\"},\"received\":{\"good\":\"%u\",\"bad\":\"%u\",\"discovery\":\"%u\"}}}",
         static_cast<char>('A' + port_index), static_cast<unsigned int>(statistics.dmx.sent), static_cast<unsigned int>(statistics.dmx.received),
         static_cast<unsigned int>(statistics.rdm.sent.classes), static_cast<unsigned int>(statistics.rdm.sent.discovery_response), static_cast<unsigned int>(statistics.rdm.received.good),
         static_cast<unsigned int>(statistics.rdm.received.bad), static_cast<unsigned int>(statistics.rdm.received.discovery_response)));

        return length;
    }

    return 0;	
}
}  // namespace json::status
