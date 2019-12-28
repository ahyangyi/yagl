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
#include "Record.h"


class Action02SpriteLayoutRecord : public ActionRecord
{
public:
    Action02SpriteLayoutRecord()
    : ActionRecord{RecordType::ACTION_02_SPRITE_LAYOUT}
    {
    }

    // Binary serialisation
    void read(std::istream& is, const GRFInfo& info) override;
    void write(std::ostream& os, const GRFInfo& info) const override;    
    // Text serialisation
    void print(std::ostream& os, const SpriteZoomMap& sprites, uint16_t indent) const override;
    void parse(TokenStream& is) override;

private:
    struct SpriteRegisters
    {
        static constexpr uint8_t HIDE_SPRITE    = 0x01;   // Only draw sprite if value of register TileLayoutRegisters::dodraw is non-zero.
        static constexpr uint8_t SPRITE_OFFSET  = 0x02;   // Add signed offset to sprite from register TileLayoutRegisters::sprite.
        static constexpr uint8_t PALETTE_OFFSET = 0x04;   // Add signed offset to palette from register TileLayoutRegisters::palette.
        static constexpr uint8_t PALETTE_ACT01  = 0x08;   // Palette is from Action 1 (moved to SPRITE_MODIFIER_CUSTOM_SPRITE in palette during loading).
        static constexpr uint8_t BB_XY_OFFSET   = 0x10;   // Add signed offset to bounding box X and Y positions. 
        static constexpr uint8_t BB_Z_OFFSET    = 0x20;   // Add signed offset to bounding box Z positions.
        static constexpr uint8_t CHILD_X_OFFSET = 0x10;   // Add signed offset to child sprite X positions.
        static constexpr uint8_t CHILD_Y_OFFSET = 0x20;   // Add signed offset to child sprite Y positions.
        static constexpr uint8_t SPRITE_VAR10   = 0x40;   // Resolve sprite with a specific value in variable 10.
        static constexpr uint8_t PALETTE_VAR10  = 0x80;   // Resolve palette with a specific value in variable 10.

        uint16_t flags;
        uint8_t  hide_sprite;       // bit 0
        uint8_t  sprite_offset;     // bit 1
        uint8_t  palette_offset;    // bit 2
        uint8_t  palette_act01;     // bit 3
        uint8_t  offset_x;          // bit 4
        uint8_t  offset_y;          // bit 4 (parent), bit 5 (child/ground)
        uint8_t  offset_z;          // bit 5 (parent)
        uint8_t  sprite_var10;      // bit 6 
        uint8_t  palette_var10;     // bit 7

        void read(std::istream& is, bool is_parent);
        void write(std::ostream& os, bool is_parent) const;
        void print(std::ostream& os, bool is_parent, uint16_t indent) const;
    };

private:
    struct BuildingSprite
    {
        uint32_t        sprite; /* May be zero only in the simple case */
        SpriteRegisters regs;
        int8_t          xofs;
        int8_t          yofs;
        uint8_t         zofs; /* Absent for simple case. 0x80 when re-using previous bounding box */
        uint8_t         xext; /* Absent when re-using previous bounding box */
        uint8_t         yext; /* Absent when re-using previous bounding box */
        uint8_t         zext; /* Absent when re-using previous bounding box */
        bool            new_bb;
    };

private:
    FeatureType     m_feature;
    uint8_t         m_set_id;
    uint32_t        m_ground_sprite;
    SpriteRegisters m_ground_regs;
    bool            m_advanced;
    std::vector<BuildingSprite> m_building_sprites;
};
