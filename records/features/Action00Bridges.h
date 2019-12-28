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
#pragma once
#include "Action00Feature.h"
#include "StreamHelpers.h"
#include <vector>


class Action00Bridges : public Action00Feature
{
public:
    Action00Bridges() : Action00Feature() {}

    // Binary serialisation
    bool read_property(std::istream& is, uint8_t property) override;
    bool write_property(std::ostream& os, uint8_t property) const override;
    // Text serialisation
    bool print_property(std::ostream& os, uint8_t property, uint16_t indent) const override;
    bool parse_property(TokenStream& is, const std::string& name, uint8_t& index) override;

private:
    class BridgeTable
    {
    public:
        void read(std::istream& is);
        void write(std::ostream& os) const;
        void print(std::ostream& os, uint16_t indent) const;
        void parse(TokenStream& is);

    private:
        std::array<uint32_t, 32> m_sprites;
    };

public:
    class BridgeLayout
    {
    public:    
        void read(std::istream& is);
        void write(std::ostream& os) const;
        void print(std::ostream& os, uint16_t indent) const;
        void parse(TokenStream& is);

    private:
        uint8_t                  m_first_table_id;
        std::vector<BridgeTable> m_tables;
    };

private:
    uint8_t      m_00_fallback_type_id;
    uint8_t      m_08_year_available;
    uint8_t      m_09_minimum_length;
    uint8_t      m_0A_maximum_length;
    uint8_t      m_0B_cost_factor;
    uint16_t     m_0C_maximum_speed;
    BridgeLayout m_0D_bridge_layout;
    uint8_t      m_0E_various_flags;
    uint32_t     m_0F_long_year_available;
    uint16_t     m_10_purchase_text;
    uint16_t     m_11_description_rail;
    uint16_t     m_12_description_road;
    uint16_t     m_13_cost_factor_word;
};

