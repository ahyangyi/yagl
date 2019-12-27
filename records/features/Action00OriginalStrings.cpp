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
#include "Action00OriginalStrings.h"
#include "StreamHelpers.h"


bool Action00OriginalStrings::read_property(std::istream& is, uint8_t property)
{
    throw RUNTIME_ERROR("Action00OriginalStrings::read not implemented");
}   


bool Action00OriginalStrings::write_property(std::ostream& os, uint8_t property) const
{
    throw RUNTIME_ERROR("Action00OriginalStrings::write not implemented");
}


bool Action00OriginalStrings::print_property(std::ostream& os, uint8_t property, uint16_t indent) const 
{
    throw RUNTIME_ERROR("Action00OriginalStrings::print not implemented");
}


bool Action00OriginalStrings::parse_property(TokenStream& is, const std::string& name, uint8_t& index)
{
    throw RUNTIME_ERROR("Action00OriginalStrings::parse not implemented");
}
