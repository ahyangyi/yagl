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
#include <string>
#include "cxxopts.hpp"
#include "Palettes.h"
#include "Record.h"


// A simple singleton so that we can more easily access the command line options 
// from everywhere in the software, if we need to.
class CommandLineOptions
{
    public: 
        enum class Operation { Decode, Encode };   

    public: 
        void parse(int argc, char* argv[]);
        static CommandLineOptions& options();

        Operation          operation() const { return m_operation; }
        const std::string& grf_file()  const { return m_grf_file; }
        const std::string& yagl_dir()  const { return m_yagl_dir; }

        uint32_t           width()     const { return m_width; }
        uint32_t           height()    const { return m_height; }
        PaletteType        palette()   const { return m_palette; }
        GRFFormat          format()    const { return m_format; }

        bool               debug()     const { return m_debug; }
        
    private:
        CommandLineOptions() {}

    private:
        // Required arguments.
        Operation   m_operation;
        std::string m_grf_file;

        // Optional arguments.
        std::string m_yagl_dir = "sprites";
        uint16_t    m_width    = 800;
        uint16_t    m_height   = 16'000;
        PaletteType m_palette  = PaletteType::Default; 
        GRFFormat   m_format   = GRFFormat::Container2;

        // Used for debugging
        bool        m_debug    = false;
};

  