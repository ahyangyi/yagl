///////////////////////////////////////////////////////////////////////////////
// Copyright 2019 Alan Chambers (unicycle.bloke@gmail.com)
//
// This file is part of yagl.
//
// yagl is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// yagl is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with yagl. If not, see <https://www.gnu.org/licenses/>.
///////////////////////////////////////////////////////////////////////////////
#include "Action00AirportTiles.h"
#include "StreamHelpers.h"
#include "Descriptors.h"


namespace {


constexpr const char* str_substitute_tile_id   = "substitute_tile_id";
constexpr const char* str_aiport_tile_override = "aiport_tile_override";
constexpr const char* str_callback_flags       = "callback_flags";
constexpr const char* str_animation_frames     = "animation_frames";
constexpr const char* str_animation_type       = "animation_type";
constexpr const char* str_animation_speed      = "animation_speed";
constexpr const char* str_animation_triggers   = "animation_triggers";


// Properties are only 8 bits. Pad to 16 bits to allow sub-properties to be 
// split out and not ambiguous for the parser. Not all features need this, but
// it's simpler to be consistent.
const std::map<std::string, uint16_t> g_indices =
{
    { str_substitute_tile_id,   0x08'00 },
    { str_aiport_tile_override, 0x09'00 },
    { str_callback_flags,       0x0E'00 },
    { str_animation_frames,     0x0F'00 },
    { str_animation_type,       0x0F'01 },
    { str_animation_speed,      0x10'00 },
    { str_animation_triggers,   0x11'00 },
};


constexpr IntegerDescriptorT<uint8_t> desc_08  = { 0x08, str_substitute_tile_id,   PropFormat::Hex };
constexpr IntegerDescriptorT<uint8_t> desc_09  = { 0x09, str_aiport_tile_override, PropFormat::Hex };
constexpr IntegerDescriptorT<uint8_t> desc_0E  = { 0x0E, str_callback_flags,       PropFormat::Hex };
constexpr IntegerDescriptorT<uint8_t> desc_0F0 = { 0x0F, str_animation_frames,     PropFormat::Hex };
constexpr IntegerDescriptorT<uint8_t> desc_0F1 = { 0x0F, str_animation_type,       PropFormat::Hex };
constexpr IntegerDescriptorT<uint8_t> desc_10  = { 0x10, str_animation_speed,      PropFormat::Hex };
constexpr IntegerDescriptorT<uint8_t> desc_11  = { 0x11, str_animation_triggers,   PropFormat::Hex };


} // namespace {


bool Action00AirportTiles::read_property(std::istream& is, uint8_t property)
{
    switch (property)
    {
        case 0x08: m_08_substitute_tile_id    = read_uint8(is); break;
        case 0x09: m_09_aiport_tile_override  = read_uint8(is); break;
        case 0x0E: m_0E_callback_flags        = read_uint8(is); break;
        // The low byte specifies the number of animation frames minus one, so 00 means 1 frame, 
        // 01 means 2 frames etc. The maximum number of frames is 256, although you can have some 
        // problems if your animation exceeds FD (253) frames. The high byte must be 0 for 
        // on-looping animations and 01 for looping animations. Every other value is reserved for 
        // future use. In addition, if the whole word contains FFFF, animation is turned off for 
        // this tile (this is the default value). 
        case 0x0F: m_0F_animation_frames      = read_uint8(is);
                   m_0F_animation_type        = read_uint8(is); break;
        case 0x10: m_10_animation_speed       = read_uint8(is); break;
        case 0x11: m_11_animation_triggers    = read_uint8(is); break;
        default:   throw RUNTIME_ERROR("Unknown property");
    }

    return true;
}   


bool Action00AirportTiles::write_property(std::ostream& os, uint8_t property) const
{
    switch (property)
    {
        case 0x08: write_uint8(os, m_08_substitute_tile_id); break;
        case 0x09: write_uint8(os, m_09_aiport_tile_override); break;
        case 0x0E: write_uint8(os, m_0E_callback_flags); break;
        // See comments in read().
        case 0x0F: write_uint8(os, (m_0F_animation_type == 0xFF) ? 0xFF : m_0F_animation_frames);
                   write_uint8(os, m_0F_animation_type); break;
        case 0x10: write_uint8(os, m_10_animation_speed); break;
        case 0x11: write_uint8(os, m_11_animation_triggers); break;
        default:   throw RUNTIME_ERROR("Unknown property");
    }

    return true;
}


bool Action00AirportTiles::print_property(std::ostream& os, uint8_t property, uint16_t indent) const 
{
    switch (property)
    {
        case 0x08: desc_08.print(m_08_substitute_tile_id, os, indent); break;
        case 0x09: desc_09.print(m_09_aiport_tile_override, os, indent); break;
        case 0x0E: desc_0E.print(m_0E_callback_flags, os, indent); break;
        case 0x0F: desc_0F0.print(m_0F_animation_frames, os, indent); os << "\n";
                   desc_0F1.print(m_0F_animation_type, os, indent); break;
        case 0x10: desc_10.print(m_10_animation_speed, os, indent); break;
        case 0x11: desc_11.print(m_11_animation_triggers, os, indent); break;
        default:   throw RUNTIME_ERROR("Unknown property");
    }

    os << "\n";
    return true;
}


bool Action00AirportTiles::parse_property(TokenStream& is, const std::string& name, uint8_t& property)
{
    const auto& it = g_indices.find(name);
    if (it != g_indices.end())
    {
        uint16_t index = it->second;
        property = (index >> 8); // The property index is in the high byte.
        switch (index)
        {
            case 0x08'00: desc_08.parse(m_08_substitute_tile_id, is); break;
            case 0x09'00: desc_09.parse(m_09_aiport_tile_override, is); break;
            case 0x0E'00: desc_0E.parse(m_0E_callback_flags, is); break;
            case 0x0F'00: desc_0F0.parse(m_0F_animation_frames, is); break;
            case 0x0F'01: desc_0F1.parse(m_0F_animation_type, is); break;
            case 0x10'00: desc_10.parse(m_10_animation_speed, is); break;
            case 0x11'00: desc_11.parse(m_11_animation_triggers, is); break;
            default:      throw RUNTIME_ERROR("Unknown property");
        }

        return true;
    }

    throw RUNTIME_ERROR("Unknown property");
}
