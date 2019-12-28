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
#include "Action00Houses.h"
#include "StreamHelpers.h"
#include "Descriptors.h"


namespace {


constexpr const char* str_substitute_building_id =  "substitute_building_id";
constexpr const char* str_building_flags =          "building_flags";
constexpr const char* str_first_year_available =    "first_year_available";
constexpr const char* str_last_year_available =     "last_year_available";
constexpr const char* str_population =              "population";
constexpr const char* str_mail_multiplier =         "mail_multiplier";
constexpr const char* str_passenger_acceptance =    "passenger_acceptance";
constexpr const char* str_mail_acceptance =         "mail_acceptance";
constexpr const char* str_goods_etc_acceptance =    "goods_etc_acceptance";
constexpr const char* str_accepts_goods =           "accepts_goods";
constexpr const char* str_la_rating_decrease =      "la_rating_decrease";
constexpr const char* str_removal_cost_multiplier = "removal_cost_multiplier";
constexpr const char* str_building_name_id =        "building_name_id";
constexpr const char* str_availability_mask =       "availability_mask";
constexpr const char* str_callback_flags =          "callback_flags";
constexpr const char* str_override_byte =           "override_byte";
constexpr const char* str_refresh_multiplier =      "refresh_multiplier";
constexpr const char* str_four_random_colours =     "four_random_colours";
constexpr const char* str_appearance_probability =  "appearance_probability";
constexpr const char* str_extra_flags =             "extra_flags";
constexpr const char* str_animation_frames =        "animation_frames";
constexpr const char* str_animation_loops =         "animation_loops";
constexpr const char* str_animation_speed =         "animation_speed";
constexpr const char* str_building_class =          "building_class";
constexpr const char* str_callback_flags_2 =        "callback_flags_2";
constexpr const char* str_accepted_cargo_types=     "accepted_cargo_types";
constexpr const char* str_minimum_life_years =      "minimum_life_years";
constexpr const char* str_accepted_cargo_list =     "accepted_cargo_list";
constexpr const char* str_long_minimum_year =       "long_minimum_year";
constexpr const char* str_long_maximum_year =       "long_maximum_year";


// Properties are only 8 bits. Pad to 16 bits to allow sub-properties to be 
// split out and not ambiguous for the parser. Not all features need this, but
// it's simpler to be consistent.
const std::map<std::string, uint16_t> g_indices =
{
    { str_substitute_building_id,  0x08'00 },
    { str_building_flags,          0x09'00 },
    { str_first_year_available,    0x0A'00 },
    { str_last_year_available,     0x0A'01 },
    { str_population,              0x0B'00 },
    { str_mail_multiplier,         0x0C'00 },
    { str_passenger_acceptance,    0x0D'00 },
    { str_mail_acceptance,         0x0E'00 },
    { str_goods_etc_acceptance,    0x0F'00 },
    { str_accepts_goods,           0x0F'01 },
    { str_la_rating_decrease,      0x10'00 },
    { str_removal_cost_multiplier, 0x11'00 },
    { str_building_name_id,        0x12'00 },
    { str_availability_mask,       0x13'00 },
    { str_callback_flags,          0x14'00 },
    { str_override_byte,           0x15'00 },
    { str_refresh_multiplier,      0x16'00 },
    { str_four_random_colours,     0x17'00 },
    { str_appearance_probability,  0x18'00 },
    { str_extra_flags,             0x19'00 },
    { str_animation_frames,        0x1A'00 },
    { str_animation_loops,         0x1A'01 },
    { str_animation_speed,         0x1B'00 },
    { str_building_class,          0x1C'00 },
    { str_callback_flags_2,        0x1D'00 },
    { str_accepted_cargo_types,    0x1E'00 },
    { str_minimum_life_years,      0x1F'00 },
    { str_accepted_cargo_list,     0x20'00 },
    { str_long_minimum_year,       0x21'00 },
    { str_long_maximum_year,       0x22'00 },
};


constexpr IntegerDescriptorT<uint8_t>  desc_08  = { 0x08, str_substitute_building_id, PropFormat::Hex };
constexpr IntegerDescriptorT<uint8_t>  desc_09  = { 0x09, str_building_flags,         PropFormat::Hex };
constexpr IntegerDescriptorT<uint8_t>  desc_0A0 = { 0x0A, str_first_year_available,   PropFormat::Hex };
constexpr IntegerDescriptorT<uint8_t>  desc_0A1 = { 0x0A, str_last_year_available,    PropFormat::Hex };
constexpr IntegerDescriptorT<uint8_t>  desc_0B  = { 0x0B, str_population,             PropFormat::Hex };
constexpr IntegerDescriptorT<uint8_t>  desc_0C  = { 0x0C, str_mail_multiplier,        PropFormat::Hex };
constexpr IntegerDescriptorT<uint8_t>  desc_0D  = { 0x0D, str_passenger_acceptance,   PropFormat::Hex };
constexpr IntegerDescriptorT<uint8_t>  desc_0E  = { 0x0E, str_mail_acceptance,        PropFormat::Hex };
constexpr BooleanDescriptor            desc_0F0 = { 0x0F, str_accepts_goods };
constexpr IntegerDescriptorT<uint8_t>  desc_0F1 = { 0x0F, str_goods_etc_acceptance,   PropFormat::Hex };
constexpr IntegerDescriptorT<uint16_t> desc_10  = { 0x10, str_la_rating_decrease,     PropFormat::Hex };
constexpr IntegerDescriptorT<uint8_t>  desc_11  = { 0x11, str_removal_cost_multiplier,PropFormat::Hex };
constexpr IntegerDescriptorT<uint16_t> desc_12  = { 0x12, str_building_name_id,       PropFormat::Hex };
constexpr IntegerDescriptorT<uint16_t> desc_13  = { 0x13, str_availability_mask,      PropFormat::Hex };
constexpr IntegerDescriptorT<uint8_t>  desc_14  = { 0x14, str_callback_flags,         PropFormat::Hex };
constexpr IntegerDescriptorT<uint8_t>  desc_15  = { 0x15, str_override_byte,          PropFormat::Hex };
constexpr IntegerDescriptorT<uint8_t>  desc_16  = { 0x16, str_refresh_multiplier,     PropFormat::Hex };
constexpr ArrayDescriptorT<uint8_t, 4> desc_17  = { 0x17, str_four_random_colours,    PropFormat::Hex };
constexpr IntegerDescriptorT<uint8_t>  desc_18  = { 0x18, str_appearance_probability, PropFormat::Hex };
constexpr IntegerDescriptorT<uint8_t>  desc_19  = { 0x19, str_extra_flags,            PropFormat::Hex };
constexpr IntegerDescriptorT<uint8_t>  desc_1A0 = { 0x1A, str_animation_frames,       PropFormat::Hex };
constexpr BooleanDescriptor            desc_1A1 = { 0x1A, str_animation_loops };
constexpr IntegerDescriptorT<uint8_t>  desc_1B  = { 0x1B, str_animation_speed,        PropFormat::Hex };
constexpr IntegerDescriptorT<uint8_t>  desc_1C  = { 0x1C, str_building_class,         PropFormat::Hex };
constexpr IntegerDescriptorT<uint8_t>  desc_1D  = { 0x1D, str_callback_flags_2,       PropFormat::Hex };
constexpr ArrayDescriptorT<uint8_t, 4> desc_1E  = { 0x1E, str_accepted_cargo_types,   PropFormat::Hex };
constexpr IntegerDescriptorT<uint16_t> desc_1F  = { 0x1F, str_minimum_life_years,     PropFormat::Hex };
constexpr CargoListDescriptor          desc_20  = { 0x20, str_accepted_cargo_list };
constexpr IntegerDescriptorT<uint16_t> desc_21  = { 0x21, str_long_minimum_year,      PropFormat::Hex };
constexpr IntegerDescriptorT<uint16_t> desc_22  = { 0x22, str_long_maximum_year,      PropFormat::Hex };


} // namespace {


bool Action00Houses::read_property(std::istream& is, uint8_t property)
{
    switch (property)
    {
        case 0x08: m_08_substitute_building_id  = read_uint8(is); break;
        case 0x09: m_09_building_flags          = read_uint8(is); break;
        case 0x0A: m_0A_first_year_available    = read_uint8(is);
                   m_0A_last_year_available     = read_uint8(is); break;
        case 0x0B: m_0B_population              = read_uint8(is); break;
        case 0x0C: m_0C_mail_multiplier         = read_uint8(is); break;
        case 0x0D: m_0D_passenger_acceptance    = read_uint8(is); break;
        case 0x0E: m_0E_mail_acceptance         = read_uint8(is); break;
        case 0x0F: m_0F_goods_etc_acceptance    = read_uint8(is);
                   m_0F_accepts_goods           = (m_0F_goods_etc_acceptance & 0x80) == 0x00;
                   m_0F_goods_etc_acceptance    = m_0F_accepts_goods ? m_0F_goods_etc_acceptance : -m_0F_goods_etc_acceptance;
                   break;
        case 0x10: m_10_la_rating_decrease      = read_uint16(is); break;
        case 0x11: m_11_removal_cost_multiplier = read_uint8(is); break;
        case 0x12: m_12_building_name_id        = read_uint16(is); break;
        case 0x13: m_13_availability_mask       = read_uint16(is); break;
        case 0x14: m_14_callback_flags          = read_uint8(is); break;
        case 0x15: m_15_override_byte           = read_uint8(is); break;
        case 0x16: m_16_refresh_multiplier      = read_uint8(is); break;
        case 0x17: m_17_four_random_colours[0]  = read_uint8(is);
                   m_17_four_random_colours[1]  = read_uint8(is);
                   m_17_four_random_colours[2]  = read_uint8(is);
                   m_17_four_random_colours[3]  = read_uint8(is); break;
        case 0x18: m_18_appearance_probability  = read_uint8(is); break;
        case 0x19: m_19_extra_flags             = read_uint8(is); break;
        case 0x1A: m_1A_animation_frames        = read_uint8(is);
                   m_1A_animation_loops         = (m_1A_animation_frames & 0x80) == 0x80;
                   m_1A_animation_frames       &= 0x7F;
                   break;
        case 0x1B: m_1B_animation_speed         = read_uint8(is); break;
        case 0x1C: m_1C_building_class          = read_uint8(is); break;
        case 0x1D: m_1D_callback_flags_2        = read_uint8(is); break;
        case 0x1E: m_1E_accepted_cargo_types[0] = read_uint8(is);
                   m_1E_accepted_cargo_types[1] = read_uint8(is);
                   m_1E_accepted_cargo_types[2] = read_uint8(is);
                   m_1E_accepted_cargo_types[3] = read_uint8(is); break;
        case 0x1F: m_1F_minimum_life_years      = read_uint16(is); break;
        case 0x20: m_20_accepted_cargo_list.read(is); break;
        case 0x21: m_21_long_minimum_year       = read_uint16(is); break;
        case 0x22: m_22_long_maximum_year       = read_uint16(is); break;
        default:   throw RUNTIME_ERROR("Unknown property");
    }

    return true;
}   


bool Action00Houses::write_property(std::ostream& os, uint8_t property) const
{
    switch (property)
    {
        case 0x08: write_uint8(os, m_08_substitute_building_id); break;
        case 0x09: write_uint8(os, m_09_building_flags); break;
        case 0x0A: write_uint8(os, m_0A_first_year_available);
                   write_uint8(os, m_0A_last_year_available); break;
        case 0x0B: write_uint8(os, m_0B_population); break;
        case 0x0C: write_uint8(os, m_0C_mail_multiplier); break;
        case 0x0D: write_uint8(os, m_0D_passenger_acceptance); break;
        case 0x0E: write_uint8(os, m_0E_mail_acceptance); break;
        case 0x0F: write_uint8(os, m_0F_accepts_goods ? m_0F_goods_etc_acceptance : -m_0F_goods_etc_acceptance); break;
        case 0x10: write_uint16(os, m_10_la_rating_decrease); break;
        case 0x11: write_uint8(os, m_11_removal_cost_multiplier); break;
        case 0x12: write_uint16(os, m_12_building_name_id); break;
        case 0x13: write_uint16(os, m_13_availability_mask); break;
        case 0x14: write_uint8(os, m_14_callback_flags); break;
        case 0x15: write_uint8(os, m_15_override_byte); break;
        case 0x16: write_uint8(os, m_16_refresh_multiplier); break;
        case 0x17: write_uint8(os, m_17_four_random_colours[0]);
                   write_uint8(os, m_17_four_random_colours[1]);
                   write_uint8(os, m_17_four_random_colours[2]);
                   write_uint8(os, m_17_four_random_colours[3]); break;
        case 0x18: write_uint8(os, m_18_appearance_probability); break;
        case 0x19: write_uint8(os, m_19_extra_flags); break;
        case 0x1A: write_uint8(os, m_1A_animation_frames | (m_1A_animation_loops ? 0x80 : 0x00)); break;
        case 0x1B: write_uint8(os, m_1B_animation_speed); break;
        case 0x1C: write_uint8(os, m_1C_building_class); break;
        case 0x1D: write_uint8(os, m_1D_callback_flags_2); break;
        case 0x1E: write_uint8(os, m_1E_accepted_cargo_types[0]);
                   write_uint8(os, m_1E_accepted_cargo_types[1]);
                   write_uint8(os, m_1E_accepted_cargo_types[2]);
                   write_uint8(os, m_1E_accepted_cargo_types[3]); break;
        case 0x1F: write_uint16(os, m_1F_minimum_life_years); break;
        case 0x20: m_20_accepted_cargo_list.write(os); break;
        case 0x21: write_uint16(os, m_21_long_minimum_year); break;
        case 0x22: write_uint16(os, m_22_long_maximum_year); break;
        default:   throw RUNTIME_ERROR("Unknown property");
    }

    return true;
}


bool Action00Houses::print_property(std::ostream& os, uint8_t property, uint16_t indent) const 
{
    switch (property)
    {
        case 0x08: desc_08.print(m_08_substitute_building_id, os, indent); break;
        case 0x09: desc_09.print(m_09_building_flags, os, indent); break;
        case 0x0A: desc_0A0.print(m_0A_first_year_available, os, indent); os << "\n";
                   desc_0A1.print(m_0A_last_year_available, os, indent); break;
        case 0x0B: desc_0B.print(m_0B_population, os, indent); break;
        case 0x0C: desc_0C.print(m_0C_mail_multiplier, os, indent); break;
        case 0x0D: desc_0D.print(m_0D_passenger_acceptance, os, indent); break;
        case 0x0E: desc_0E.print(m_0E_mail_acceptance, os, indent); break;
        case 0x0F: desc_0F0.print(m_0F_accepts_goods, os, indent); break; os << "\n";
                   desc_0F1.print(m_0F_goods_etc_acceptance, os, indent); break;
        case 0x10: desc_10.print(m_10_la_rating_decrease, os, indent); break;
        case 0x11: desc_11.print(m_11_removal_cost_multiplier, os, indent); break;
        case 0x12: desc_12.print(m_12_building_name_id, os, indent); break;
        case 0x13: desc_13.print(m_13_availability_mask, os, indent); break;
        case 0x14: desc_14.print(m_14_callback_flags, os, indent); break;
        case 0x15: desc_15.print(m_15_override_byte, os, indent); break;
        case 0x16: desc_16.print(m_16_refresh_multiplier, os, indent); break;
        case 0x17: desc_17.print(m_17_four_random_colours, os, indent); break;
        case 0x18: desc_18.print(m_18_appearance_probability, os, indent); break;
        case 0x19: desc_19.print(m_19_extra_flags, os, indent); break;
        case 0x1A: desc_1A0.print(m_1A_animation_frames, os, indent); os << "\n";
                   desc_1A1.print(m_1A_animation_loops, os, indent); break;
        case 0x1B: desc_1B.print(m_1B_animation_speed, os, indent); break;
        case 0x1C: desc_1C.print(m_1C_building_class, os, indent); break;
        case 0x1D: desc_1D.print(m_1D_callback_flags_2, os, indent); break;
        case 0x1E: desc_1E.print(m_1E_accepted_cargo_types, os, indent); break;
        case 0x1F: desc_1F.print(m_1F_minimum_life_years, os, indent); break;
        case 0x20: desc_20.print(m_20_accepted_cargo_list, os, indent); break;
        case 0x21: desc_21.print(m_21_long_minimum_year, os, indent); break;
        case 0x22: desc_22.print(m_22_long_maximum_year, os, indent); break;
        default:   throw RUNTIME_ERROR("Unknown property");
    }

    os << "\n";
    return true;
}


bool Action00Houses::parse_property(TokenStream& is, const std::string& name, uint8_t& property)
{
    const auto& it = g_indices.find(name);
    if (it != g_indices.end())
    {
        uint16_t index = it->second;
        property = (index >> 8); // The property index is in the high byte.
        switch (index)
        {
            case 0x08'00: desc_08.parse(m_08_substitute_building_id, is); break;
            case 0x09'00: desc_09.parse(m_09_building_flags, is); break;
            case 0x0A'00: desc_0A0.parse(m_0A_first_year_available, is);
            case 0x0A'01: desc_0A1.parse(m_0A_last_year_available, is); break;
            case 0x0B'00: desc_0B.parse(m_0B_population, is); break;
            case 0x0C'00: desc_0C.parse(m_0C_mail_multiplier, is); break;
            case 0x0D'00: desc_0D.parse(m_0D_passenger_acceptance, is); break;
            case 0x0E'00: desc_0E.parse(m_0E_mail_acceptance, is); break;
            case 0x0F'00: desc_0F0.parse(m_0F_accepts_goods, is); break;
            case 0x0F'01: desc_0F1.parse(m_0F_goods_etc_acceptance, is); break;
            case 0x10'00: desc_10.parse(m_10_la_rating_decrease, is); break;
            case 0x11'00: desc_11.parse(m_11_removal_cost_multiplier, is); break;
            case 0x12'00: desc_12.parse(m_12_building_name_id, is); break;
            case 0x13'00: desc_13.parse(m_13_availability_mask, is); break;
            case 0x14'00: desc_14.parse(m_14_callback_flags, is); break;
            case 0x15'00: desc_15.parse(m_15_override_byte, is); break;
            case 0x16'00: desc_16.parse(m_16_refresh_multiplier, is); break;
            case 0x17'00: desc_17.parse(m_17_four_random_colours, is);
            case 0x18'00: desc_18.parse(m_18_appearance_probability, is); break;
            case 0x19'00: desc_19.parse(m_19_extra_flags, is); break;
            case 0x1A'00: desc_1A0.parse(m_1A_animation_frames, is);
            case 0x1A'01: desc_1A1.parse(m_1A_animation_loops, is); break;
            case 0x1B'00: desc_1B.parse(m_1B_animation_speed, is); break;
            case 0x1C'00: desc_1C.parse(m_1C_building_class, is); break;
            case 0x1D'00: desc_1D.parse(m_1D_callback_flags_2, is); break;
            case 0x1E'00: desc_1E.parse(m_1E_accepted_cargo_types, is);
            case 0x1F'00: desc_1F.parse(m_1F_minimum_life_years, is); break;
            case 0x20'00: desc_20.parse(m_20_accepted_cargo_list, is); break;
            case 0x21'00: desc_21.parse(m_21_long_minimum_year, is); break;
            case 0x22'00: desc_22.parse(m_22_long_maximum_year, is); break;
            default:      throw RUNTIME_ERROR("Unknown property");
        }

        return true;
    }

    throw RUNTIME_ERROR("Unknown property");
}
