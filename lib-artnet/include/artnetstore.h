/**
 * @file artnetstore.h
 *
 */
/* Copyright (C) 2018-2025 by Arjan van Vught mailto:info@gd32-dmx.org
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef ARTNETSTORE_H_
#define ARTNETSTORE_H_

#include <cstdint>

#include "artnet.h"
#include "artnetnode.h"
#include "dmxnode.h"
#include "dmxnode_utils.h"
#include "configstore.h"
 #include "firmware/debug/debug_debug.h"

namespace artnet::store
{
namespace internal
{
inline void SaveUniverse(uint32_t port_index)
{
    DEBUG_ENTRY();
    DEBUG_PRINTF("port_index=%u", port_index);

    uint16_t universe;

    if (ArtNetNode::Get()->GetPortAddress(port_index, universe))
    {
#if defined(CONFIG_DMXNODE_DMX_PORT_OFFSET)
        if (port_index >= CONFIG_DMXNODE_DMX_PORT_OFFSET)
        {
            port_index -= CONFIG_DMXNODE_DMX_PORT_OFFSET;
        }
        else
        {
            DEBUG_EXIT();
            return;
        }
#endif

        DEBUG_PRINTF("port_index=%u", port_index);

        if (port_index >= artnet::kPorts)
        {
            DEBUG_EXIT();
            return;
        }

#if defined(__GNUC__) && !defined(__clang__) && (__GNUC__ >= 13)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdangling-pointer"
#endif

        DEBUG_PRINTF("port_index=%u, universe=%u", port_index, universe);

#if defined(__GNUC__) && !defined(__clang__) && (__GNUC__ >= 13)
#pragma GCC diagnostic pop
#endif

        ConfigStore::Instance().DmxNodeUpdateIndexed(&common::store::DmxNode::universe, port_index, universe);
    }

    DEBUG_EXIT();
}
} // namespace internal

inline void SaveLongName(const char* long_name)
{
    DEBUG_ENTRY();
    ConfigStore::Instance().DmxNodeUpdateArray(&common::store::DmxNode::long_name, long_name, artnet::kLongNameLength);
    DEBUG_EXIT();
}

inline void SaveShortName(uint32_t port_index, const char* short_name)
{
    DEBUG_ENTRY();
    DEBUG_PRINTF("%u, %s", port_index, short_name);

#if defined(CONFIG_DMXNODE_DMX_PORT_OFFSET)
    if (port_index >= CONFIG_DMXNODE_DMX_PORT_OFFSET)
    {
        port_index -= CONFIG_DMXNODE_DMX_PORT_OFFSET;
    }
    else
    {
        DEBUG_EXIT();
        return;
    }
#endif

    DEBUG_PRINTF("port_index=%u", port_index);

    if (port_index >= artnet::kPorts)
    {
        DEBUG_EXIT();
        return;
    }

    ConfigStore::Instance().DmxNodeUpdateLabel(&common::store::DmxNode::label, port_index, short_name, artnet::kShortNameLength);

    DEBUG_EXIT();
}

inline void SaveSwitch(uint32_t port_index, [[maybe_unused]] uint8_t sw)
{
    DEBUG_ENTRY();
    internal::SaveUniverse(port_index);
    DEBUG_EXIT();
}

inline void SaveDirection(uint32_t port_index, dmxnode::PortDirection direction)
{
    DEBUG_ENTRY();
    DEBUG_PRINTF("%u, %u", port_index, static_cast<uint32_t>(direction));

#if defined(CONFIG_DMXNODE_DMX_PORT_OFFSET)
    if (port_index >= CONFIG_DMXNODE_DMX_PORT_OFFSET)
    {
        port_index -= CONFIG_DMXNODE_DMX_PORT_OFFSET;
    }
    else
    {
        DEBUG_EXIT();
        return;
    }
#endif

    DEBUG_PRINTF("port_index=%u", port_index);

    if (port_index >= artnet::kPorts)
    {
        DEBUG_EXIT();
        return;
    }

    auto direction_store = ConfigStore::Instance().DmxNodeGet(&common::store::DmxNode::direction);

    json::PortSet<dmxnode::PortDirection>(port_index, direction, direction_store);

    ConfigStore::Instance().DmxNodeUpdate(&common::store::DmxNode::direction, direction_store);

    DEBUG_EXIT();
}

inline void SaveMergeMode(uint32_t port_index, dmxnode::MergeMode merge_mode)
{
    DEBUG_ENTRY();
    DEBUG_PRINTF("%u, %u", port_index, static_cast<uint32_t>(merge_mode));

#if defined(CONFIG_DMXNODE_DMX_PORT_OFFSET)
    if (port_index >= CONFIG_DMXNODE_DMX_PORT_OFFSET)
    {
        port_index -= CONFIG_DMXNODE_DMX_PORT_OFFSET;
    }
    else
    {
        DEBUG_EXIT();
        return;
    }
#endif

    DEBUG_PRINTF("port_index=%u", port_index);

    if (port_index >= artnet::kPorts)
    {
        DEBUG_EXIT();
        return;
    }

    auto merge_mode_store = ConfigStore::Instance().DmxNodeGet(&common::store::DmxNode::merge_mode);

    json::PortSet<dmxnode::MergeMode>(port_index, merge_mode, merge_mode_store);

    ConfigStore::Instance().DmxNodeUpdate(&common::store::DmxNode::merge_mode, merge_mode_store);

    DEBUG_EXIT();
}

inline void SaveProtocol(uint32_t port_index, artnet::PortProtocol port_protocol)
{
    DEBUG_ENTRY();
    DEBUG_PRINTF("port_index=%u, portProtocol=%u", port_index, static_cast<uint32_t>(port_protocol));

#if defined(CONFIG_DMXNODE_DMX_PORT_OFFSET)
    if (port_index >= CONFIG_DMXNODE_DMX_PORT_OFFSET)
    {
        port_index -= CONFIG_DMXNODE_DMX_PORT_OFFSET;
    }
    else
    {
        DEBUG_EXIT();
        return;
    }
#endif

    DEBUG_PRINTF("port_index=%u", port_index);

    if (port_index >= artnet::kPorts)
    {
        DEBUG_EXIT();
        return;
    }

    uint16_t protocol_store = ConfigStore::Instance().DmxNodeGet(&common::store::DmxNode::protocol);

    json::PortSet<artnet::PortProtocol>(port_index, port_protocol, protocol_store);

    ConfigStore::Instance().DmxNodeUpdate(&common::store::DmxNode::protocol, protocol_store);

    DEBUG_EXIT();
}

inline void SaveOutputStyle(uint32_t port_index, dmxnode::OutputStyle output_style)
{
    DEBUG_ENTRY();
    DEBUG_PRINTF("port_index=%u, output_style=%u", port_index, static_cast<uint32_t>(output_style));

#if defined(CONFIG_DMXNODE_DMX_PORT_OFFSET)
    if (port_index >= CONFIG_DMXNODE_DMX_PORT_OFFSET)
    {
        port_index -= CONFIG_DMXNODE_DMX_PORT_OFFSET;
    }
    else
    {
        DEBUG_EXIT();
        return;
    }
#endif

    DEBUG_PRINTF("port_index=%u", port_index);

    if (port_index >= artnet::kPorts)
    {
        DEBUG_EXIT();
        return;
    }

    auto output_style_store = ConfigStore::Instance().DmxNodeGet(&common::store::DmxNode::output_style);

    if (output_style == dmxnode::OutputStyle::kConstant)
    {
        output_style_store |= static_cast<uint8_t>(1U << port_index);
    }
    else
    {
        output_style_store &= static_cast<uint8_t>(~(1U << port_index));
    }

    ConfigStore::Instance().DmxNodeUpdate(&common::store::DmxNode::output_style, output_style_store);

    DEBUG_EXIT();
}

inline void SaveRdmEnabled(uint32_t port_index, bool is_enabled)
{
    DEBUG_ENTRY();
    DEBUG_PRINTF("port_index=%u, is_enabled=%d", port_index, is_enabled);

#if defined(CONFIG_DMXNODE_DMX_PORT_OFFSET)
    if (port_index >= CONFIG_DMXNODE_DMX_PORT_OFFSET)
    {
        port_index -= CONFIG_DMXNODE_DMX_PORT_OFFSET;
    }
    else
    {
        DEBUG_EXIT();
        return;
    }
#endif

    DEBUG_PRINTF("port_index=%u", port_index);

    if (port_index >= artnet::kPorts)
    {
        DEBUG_EXIT();
        return;
    }

    auto rdm_store = ConfigStore::Instance().DmxNodeGet(&common::store::DmxNode::rdm);

    json::PortSet<dmxnode::Rdm>(port_index, static_cast<dmxnode::Rdm>(is_enabled), rdm_store);

    ConfigStore::Instance().DmxNodeUpdate(&common::store::DmxNode::rdm, rdm_store);

    DEBUG_EXIT();
}

inline void SaveFailSafe(uint8_t fail_safe)
{
    DEBUG_ENTRY();

    ConfigStore::Instance().DmxNodeUpdate(&common::store::DmxNode::fail_safe, fail_safe);

    DEBUG_EXIT();
}
} // namespace artnet::store

#endif  // ARTNETSTORE_H_
