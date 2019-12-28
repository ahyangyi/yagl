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
#include "NewGRFData.h"
#include "Action00Record.h"
#include "Action01Record.h"
#include "Action02BasicRecord.h"
#include "Action02VariableRecord.h"
#include "Action02RandomRecord.h"
#include "Action02IndustryRecord.h"
#include "Action02SpriteLayoutRecord.h"
#include "Action03Record.h"
#include "Action04Record.h"
#include "Action05Record.h"
#include "Action06Record.h"
#include "Action07Record.h"
#include "Action08Record.h"
#include "Action0ARecord.h"
#include "Action0BRecord.h"
#include "Action0CRecord.h"
#include "Action0DRecord.h"
#include "Action0ERecord.h"
#include "Action0FRecord.h"
#include "Action10Record.h"
#include "Action11Record.h"
#include "Action12Record.h"
#include "Action13Record.h"
#include "Action14Record.h"
#include "ActionFERecord.h"
#include "ActionFFRecord.h"
#include "RecolourRecord.h"
#include "RealSpriteRecord.h"
#include "SpriteIndexRecord.h"
#include "StreamHelpers.h"
#include "SpriteSheetGenerator.h"
#include "CommandLineOptions.h"
#include <sstream>


// Expected value for the first bytes in the GRF format 2 container. 
static constexpr std::array<uint8_t, 8> CONTAINER2_IDENTIFIER 
    = { 0x47, 0x52, 0x46, 0x82, 0x0D, 0x0A, 0x1A, 0x0A };


int writeImage(char* filename, int width, int height, const char* title);


void NewGRFData::read(std::istream& is)
{
    //writeImage("test.png", 512, 256, "Test image");

    // The structure of a GRF file is pretty simple. It is just a list of 
    // variable length records in up to three sections: 
    // Header:  Format2 only        - exactly one record.
    // Data:    Format1 and Format2 - zero or more records.
    // Sprites: Format2 only        - zero of more records.

    // We need to test whether this is format 1 or a format 2 file.
    // Attempt to read the header record.
    m_info.format = read_format(is);

    // This is just the index of the current record. 
    uint32_t record_index = 0;
    // This is the number of real sprite records we need to append to the 
    // most recent top level container record (Actions 01, 05, 0A, 11 and 12).
    // The sprites always immediately follow the container definition.
    uint16_t num_sprites = 0;

    // Read data records from the data section.
    while (true)
    {
        // This section is terminated with a zero length record. The size of the length of the record
        // depends on the file format version.
        uint32_t size = (m_info.format == GRFFormat::Container1) ? read_uint16(is) : read_uint32(is);
        if (size == 0)
            break;

        // The info byte determines what type of record we are dealing with. 
        uint8_t info = read_uint8(is);

        std::shared_ptr<Record> record = nullptr;
        switch (info)
        {
            // This record is a pseudo-sprite. It contains data about properties and other things, 
            // rather than an image.
            case 0xFF:
                // The special case of having 4 bytes as the first record indicates the record counter.
                // This may or may not be present.
                if ((size == 4) && (record_index == 0))
                {
                    // Number of records in the file. We could use this to spot errors like reading past
                    // the end of the file, but there is no need to store.
                    uint32_t number_of_records = read_uint32(is);
                    std::cout << "Number of records: " << number_of_records << '\n';
                    // We didnt create a record, so skip the rest of the loop.
                    continue;
                }
                else
                {
                    // This is an actual pseudo-sprite. The condition (num_sprites == 0) tells read_record
                    // whether we are read a top-level record (a pseudo-sprite) or a real-sprite (or a 
                    // recolour-sprite). This is needed to disambiguate the start of an Action00 from 
                    // recolour-sprite. 

                    // This has fallen over while reading from zbase. The reason seems to be that the sprites
                    // appear as top level items, so a recolour sprite is treated as Action00. This does not 
                    // go well.
                    record = read_record(is, size, num_sprites == 0, m_info);
                    //record = read_record(is, size, size != 257, m_info);
                }
                break;

            // This is a sprite reference (Format2 only). It's just the index of a sprite in the 
            // later sprite section of the file. It seems that these occur in the places where real-sprites
            // would occur in Format1 files (following container records such as Action01).
            case 0xFD:
                record = std::make_shared<SpriteIndexRecord>();
                record->read(is, m_info);
                break;

            // TODO recolour sprites.

            // This is a real-sprite (Format1 only), or a recolour-sprite. The size might be misleading so we 
            // have to decompress the image to find out. Pass record index as the (fake) sprite ID. The 
            // info byte we read is in this context interpreted as the sprite's compression byte. Create an
            // index record to mimic the behaviour of Format2 files. Things are simpler this way.      
            default:
                // In this case info = compression for a real sprite, and we use the record index
                // as the sprite id.
                read_sprite(is, record_index, size, info, m_info);
                record = std::make_shared<SpriteIndexRecord>(record_index);
                break;
        } 

        // This is our slightly too trusting method for grouping sprites into containers. 
        // The format for the file is simply a long list of records. Some of the records are effectively
        // containers, such as Action01, and indicate how many sprites they contain, call it NUM. 
        // The next NUM records in the file are the sprites which the container "holds". If the wrong
        // number of sprites are in the file, it'll all go a bit wrong. 
        if (num_sprites > 0)
        {
            // We need to append the record to the previous container.
            m_records.back()->append_sprite(record);
            --num_sprites; 
        }   
        else
        {
            // This is a top level record, maybe a container. If it's a container,
            // num_sprites will be set to a non-zero value for the number of contained
            // sprites.
            m_records.push_back(record);
            num_sprites = record->num_sprites_to_read();
        }  

        if (CommandLineOptions::options().debug())
        {
            std::cout << record_index << '\n'; 
            record->print(std::cout, m_sprites, 0); // Indent = 0     
        }

        // Important: this is used to give indices to real sprites for Container1 files. 
        ++record_index;
    }

    // Read sprite records from the sprite section. Only applies to Format2 files.
    if (m_info.format == GRFFormat::Container2)
    {
        //while (true)
        while (is.peek() != EOF)
        {               
            // This section is terminated with a zero length record. 
            uint32_t sprite_id = read_uint32(is);
            if (sprite_id == 0)
                break;

            // Read the size and compression to match what we did above. 
            uint32_t size        = read_uint32(is);
            uint8_t  compression = read_uint8(is);
            
            read_sprite(is, sprite_id, size, compression, m_info);
        }
    }
}


GRFFormat NewGRFData::read_format(std::istream& is)
{
    // If this is not a format 2 file, we will read past the end of file
    // and maybe get an exception. This is not an error: it just means we have 
    // an empty GRF file.
    try
    {
        uint16_t leader = read_uint16(is);
        if (leader == 0)
        {
            // We don't need to store this value as the string is constant.
            std::array<uint8_t, 8> identifier  = {};
            for (uint8_t i = 0; i < identifier.size(); ++i)
            {
                identifier[i] = read_uint8(is);
            }

            // We don't need to store these values as they are calculated or constant.
            uint32_t sprite_offs = read_uint32(is);
            uint8_t  compression = read_uint8(is);

            if (identifier == CONTAINER2_IDENTIFIER) 
            {
                return GRFFormat::Container2;
            }
            else
            {
                return GRFFormat::Container1;
            }           
        }
    }
    catch(...)
    {
    }

    // Restore the stream for reading Format1.
    is.seekg(0, std::istream::beg);
    return GRFFormat::Container1;
}


void NewGRFData::read_sprite(std::istream& is, uint32_t sprite_id, uint32_t size, uint8_t compression, const GRFInfo& info)
{
    std::shared_ptr<RealSpriteRecord> sprite = std::make_shared<RealSpriteRecord>(sprite_id, size, compression);
    sprite->read(is, m_info);

    // Sprites with the same ID are stored in map indexed by zoom level. 
    // These maps are stored in a map index by the sprite ID.
    if (m_sprites.find(sprite_id) == m_sprites.end())
    {
        m_sprites[sprite_id] = SpriteZoomVector{};
    }
    m_sprites[sprite_id].push_back(sprite);

    if (CommandLineOptions::options().debug())
    {
        sprite->print(std::cout, m_sprites, 0);
    }
}


std::shared_ptr<Record> NewGRFData::read_record(std::istream& is, uint32_t size, bool top_level, const GRFInfo& info)
{   
    // Extract the type and data of this record. A little bit of interpretation is 
    // required to work out how to parse the data. Whether we parse the data or not,
    // it has now been taken out of the file stream.
    uint8_t action = read_uint8(is);
    std::string data;
    data.resize(size - 1);
    is.read(&data[0], size - 1);

    // Work out exactly what we are dealing with here before calling a factory method to
    // create the appropriate type of object.
    RecordType record_type;
    switch (action)
    {
        case 0x00:
            if (top_level)
            {
                // Defines new properties for anything added or changed by the NewGRF
                record_type = RecordType::ACTION_00; 
            }
            else
            {
                if (size == 0x01)
                {
                    // Special case for a null sprite.
                    record_type = RecordType::FAKE_SPRITE;
                }
                else if (size == 0x101)
                {
                    // Palettes are usually children of another action, but can also be identified by their size.
                    // The top_level test avoids potential confusion with Action00 records.
                    record_type = RecordType::RECOLOUR;
                }
                else
                {
                    throw RUNTIME_ERROR("Unknown action record type");
                }                
            }
            break;

        case 0x01: 
            // Defines one or more graphics sets
            record_type = RecordType::ACTION_01; 
            break;

        case 0x02: 
            // Action02 (variants): Defines graphics set IDs
            // This byte is in the basic case a number of graphics sets, presumably always less than 0x80.
            switch (static_cast<uint8_t>(data[2]))
            {
                case 0x80: // Use 80 to randomize the object (vehicle, station, building, industry, object) based on its own triggers and bits.
                case 0x83: // Use 83 to randomize the object based on its "related" object (s.b.).
                case 0x84: // Use 84 to randomize the vehicle based on any vehicle in the consist.
                    record_type = RecordType::ACTION_02_RANDOM;
                    break;

                case 0x81: // Lowest byte / Primary object
                case 0x82: // Lowest byte / "Related" object
                case 0x85: // Lowest word / Primary object
                case 0x86: // Lowest word / "Related" object
                case 0x89: // Lowest dword / Primary object
                case 0x8A: // Lowest dword / "Related" object
                    record_type = RecordType::ACTION_02_VARIABLE;
                    break;

                default:
                    // Switch on the feature
                    switch (data[0])
                    {
                        case 0x07: // Feature 0x07 = town buildings
                        case 0x09: // Feature 0x09 = industry tiles
                        case 0x0F: // Feature 0x0F = objects
                        case 0x11: // Feature 0x11 = airport tiles
                            record_type = RecordType::ACTION_02_SPRITE_LAYOUT;
                            break;

                        case 0x0A: // Feature 0x0A = industry production
                            record_type = RecordType::ACTION_02_INDUSTRY;
                            break;

                        default: // Standard Action 02
                            record_type = RecordType::ACTION_02_BASIC;
                    }
            }
            break;

        case 0x03: record_type = RecordType::ACTION_03; break; // Associates Action 2s with an Action 0
        case 0x04: record_type = RecordType::ACTION_04; break; // Defines text strings
        case 0x05: record_type = RecordType::ACTION_05; break; // Replaces base set sprites not present in the original TTD
        case 0x06: record_type = RecordType::ACTION_06; break; // Modifies the contents of the following sprite
        case 0x07: record_type = RecordType::ACTION_07; break; // Conditionally skips sprites or jumps to a label
        case 0x08: record_type = RecordType::ACTION_08; break; // Defines GRFID, Name and Description
        case 0x09: record_type = RecordType::ACTION_09; break; // Conditionally skips sprites or jumps to a label
        case 0x0A: record_type = RecordType::ACTION_0A; break; // Replaces TTD base set sprites
        case 0x0B: record_type = RecordType::ACTION_0B; break; // Generates an error message
        case 0x0C: record_type = RecordType::ACTION_0C; break; // Does nothing (can be used to embed comments)
        case 0x0D: record_type = RecordType::ACTION_0D; break; // Assigns parameters and calculates results
        case 0x0E: record_type = RecordType::ACTION_0E; break; // Deactivates a NewGRF file
        case 0x0F: record_type = RecordType::ACTION_0F; break; // Defines new town name styles
        case 0x10: record_type = RecordType::ACTION_10; break; // Defines a label for action 7/9
        case 0x11: record_type = RecordType::ACTION_11; break; // Defines new sounds
        case 0x12: record_type = RecordType::ACTION_12; break; // Adds Unicode font glyphs
        case 0x13: record_type = RecordType::ACTION_13; break; // Translates GRF-specific strings
        case 0x14: record_type = RecordType::ACTION_14; break; // Static NewGRF information (OpenTTD only)
     
        // These are not documented as actions per se, but have the same format.
        case 0xFE: record_type = RecordType::ACTION_FE; break; // Imported sound effects
        case 0xFF: record_type = RecordType::ACTION_FF; break; // Binary sound effects
        
        default:   
            throw RUNTIME_ERROR("Unknown action record type");
    }

    // Use a factory to create the appropriate object and then parse the data 
    // previously read from the file.
    std::shared_ptr<Record> record = make_record(record_type);
    std::istringstream iss(data);
    record->m_data = data;
    record->read(iss, m_info); 

    // We need to know the GRF version so we can pass it to the other records, some 
    // of which are interpreted differently depending on the version. 
    // TODO move this inside Action08 - would need a reinterpret cast.
    if (record_type == RecordType::ACTION_08)
    {
        auto action08  = std::dynamic_pointer_cast<Action08Record>(record);
        m_info.version = action08->grf_version();
    }

    return record;
}


std::shared_ptr<Record> NewGRFData::make_record(RecordType record_type)
{
    switch (record_type)
    {
        case RecordType::ACTION_00:               return std::make_shared<Action00Record>(); 
        case RecordType::ACTION_01:               return std::make_shared<Action01Record>(); 
        case RecordType::ACTION_02_BASIC:         return std::make_shared<Action02BasicRecord>();
        case RecordType::ACTION_02_RANDOM:        return std::make_shared<Action02RandomRecord>(); 
        case RecordType::ACTION_02_VARIABLE:      return std::make_shared<Action02VariableRecord>(); 
        case RecordType::ACTION_02_INDUSTRY:      return std::make_shared<Action02IndustryRecord>(); 
        case RecordType::ACTION_02_SPRITE_LAYOUT: return std::make_shared<Action02SpriteLayoutRecord>(); 
        case RecordType::ACTION_03:               return std::make_shared<Action03Record>(); 
        case RecordType::ACTION_04:               return std::make_shared<Action04Record>(); 
        case RecordType::ACTION_05:               return std::make_shared<Action05Record>(); 
        case RecordType::ACTION_06:               return std::make_shared<Action06Record>(); 
        case RecordType::ACTION_07:               return std::make_shared<Action07Record>(RecordType::ACTION_07); 
        case RecordType::ACTION_08:               return std::make_shared<Action08Record>(); 
        // Action09 is identical to Action 07 except in how it is used.
        case RecordType::ACTION_09:               return std::make_shared<Action07Record>(RecordType::ACTION_09); 
        case RecordType::ACTION_0A:               return std::make_shared<Action0ARecord>(); 
        case RecordType::ACTION_0B:               return std::make_shared<Action0BRecord>(); 
        case RecordType::ACTION_0C:               return std::make_shared<Action0CRecord>(); 
        case RecordType::ACTION_0D:               return std::make_shared<Action0DRecord>(); 
        case RecordType::ACTION_0E:               return std::make_shared<Action0ERecord>(); 
        case RecordType::ACTION_0F:               return std::make_shared<Action0FRecord>(); 
        case RecordType::ACTION_10:               return std::make_shared<Action10Record>(); 
        case RecordType::ACTION_11:               return std::make_shared<Action11Record>(); 
        case RecordType::ACTION_12:               return std::make_shared<Action12Record>(); 
        case RecordType::ACTION_13:               return std::make_shared<Action13Record>(); 
        case RecordType::ACTION_14:               return std::make_shared<Action14Record>(); 
        
        // These are sound effects.
        case RecordType::ACTION_FE:               return std::make_shared<ActionFERecord>();
        case RecordType::ACTION_FF:               return std::make_shared<ActionFFRecord>();

        // These are graphics.
        case RecordType::RECOLOUR:                return std::make_shared<RecolourRecord>(); 
        case RecordType::SPRITE_INDEX:            return std::make_shared<SpriteIndexRecord>();

        // Special case of an empty sprite used for absent image.
        //case RecordType::FAKE_SPRITE:             return std::make_shared<FakeSpriteRecord>();
        default:                                  throw RUNTIME_ERROR("NewGRFData::create_action");
    }

    return std::make_shared<Record>();
}


// Total number of records in the GRF file. Sprites and sounds are nested.
uint32_t NewGRFData::total_records() const
{
    uint32_t result = m_records.size();
 
    for (std::shared_ptr<Record> record: m_records)
    {
        result += record->num_sprites_to_write();
    }

    return result;
}


void NewGRFData::write_format(std::ostream& os) const
{
    if (m_info.format == GRFFormat::Container2)
    {
        // Write a leader to indicate the container version.
        write_uint16(os, 0);

        for (const auto& item: CONTAINER2_IDENTIFIER)
        {
            write_uint8(os, item);
        }

        // We need to calculate some values now, or come back later to overwrite.
        write_uint32(os, 0x12345678); // Sprite offset in the file.
        write_uint8(os, 0xAB); // Sprite compression
    }
}


void NewGRFData::write_counter(std::ostream& os) const
{
    // Create the counter record at the start of the file.
    switch (m_info.format)
    {
        case GRFFormat::Container1: write_uint16(os, 0x0004); break;
        case GRFFormat::Container2: write_uint32(os, 0x0004); break;
    }
    write_uint8(os, 0xFF);
    // It appears we should not count the counter itself.
    write_uint32(os, total_records());
}


void NewGRFData::write_record(std::ostream& os, std::shared_ptr<Record> record) const
{
    if (record->record_type() == RecordType::REAL_SPRITE)
    {
        // Real sprites calculate their own length, which has a non-obvious
        // relationship to the data length.
        record->write(os, m_info);
    }
    else if ( (record->record_type() == RecordType::SPRITE_INDEX) && 
              (m_info.format == GRFFormat::Container1)                 ) 
    {
        // For Container1 we faked up the sprite index records for convenience. Now 
        // we need to retrieve the real sprite and write that out instead.
        auto reference = std::static_pointer_cast<SpriteIndexRecord>(record);
        const SpriteZoomVector& sprites = m_sprites.at(reference->sprite_id());
        if (sprites.size() != 1)
        {
            throw RUNTIME_ERROR("Expected single real sprite");
        }
        write_record(os, sprites[0]);
    }       
    else 
    {
        // All the others are handled in same way.

        // Write the record into a buffer in order to obtain its length.
        // The buffer is copied to the output after writing a record header.
        std::ostringstream ss;
        record->write(ss, m_info);
        std::string data = ss.str();
        uint16_t length = data.length();

        // Record header
        if (m_info.format == GRFFormat::Container1)
        {
            write_uint16(os, length);
        }
        else
        {
            write_uint32(os, length);
        }

        // All pseudo-sprites are prefixed with 0xFF, except index records for real sprites.        
        uint8_t prefix = (record->record_type() == RecordType::SPRITE_INDEX) ? 0xFD : 0xFF;
        write_uint8(os, prefix);

        // Copy out the record data
        for (const auto byte: data)
        {
            write_uint8(os, byte);
        }
    }
}


void NewGRFData::write(std::ostream& os) const
{
    // Header section indicates that this a Container2 format, or not.
    // The counter is an optional record containing the number of records in the GRF.
    write_format(os);
    write_counter(os);

    for (std::shared_ptr<Record> record: m_records)
    {
        write_record(os, record);

        // Containers are used to hold records in a logical tree which is
        // not really present in the GRF. This is mostly used for the collection
        // of sprites which comes after Actions 01, 05, 0A, and so on. And Action 11.
        for (uint32_t j = 0; j < record->num_sprites_to_write(); ++j)
        {
            write_record(os, record->get_sprite(j));
        }
    }

    // Data section terminator - zero-length record
    if (m_info.format == GRFFormat::Container1)
    {
        write_uint16(os, 0x0000);
    }
    else
    {
        write_uint32(os, 0x0000000);
    } 

    // For the Container version 2, all the actual image data goes at the end.
    // This section does not exist for Container version 1.
    if (m_info.format == GRFFormat::Container2)
    {
        for (const auto& it: m_sprites)
        {
            for (const auto sprite: it.second)
            {
                sprite->write(os, m_info);
            }
        }

        write_uint32(os, 0x0000000);
    }
}


void NewGRFData::print(std::ostream& os, const std::string& output_dir, const std::string& image_file_base) const
{
    // Create sprite sheets first in order to have the filenames and locations in place
    // for when we write out the YAGL.
    SpriteSheetGenerator generator(m_sprites, image_file_base, m_info.format);
    generator.generate();

    // Write out any binary sound effects. These are all held in ActionFF records, which
    // are the children of any Action11 records we have (there should be only one, I think).
    for (auto record: m_records)
    {
        if (record->record_type() == RecordType::ACTION_11)
        {
            const auto action11 = std::dynamic_pointer_cast<Action11Record>(record);
            action11->write_binary_files(output_dir);
        }
    }

    // TODO We need to be able to cope with changes in the text format.
    // The simplest approach is to reject text files with different 
    // versions... 
    // NOTE using colons to avoid parsing the version as a number.
    os << "yagl_version: 0:0:1\n\n";

    // Finally write out the YAGL script.
    for (auto record: m_records)
    {
        record->print(os, m_sprites, 0);
    }
}


void NewGRFData::parse(TokenStream& is)
{
    const TokenValue& token = is.peek();
    if (is.match(TokenType::Ident) != "yagl_version")
    {
        throw ParserError("Expected YAGL version number", token);
    }
    is.match(TokenType::Colon);

    // Having a floating point format would be a problem here, so just 
    // have integers separated by colons.
    uint16_t major = is.match_integer();
    is.match(TokenType::Colon);
    uint16_t minor = is.match_integer();
    is.match(TokenType::Colon);
    uint16_t build = is.match_integer();

    // Top level parser. Every record has the format 'keyword [<...>] { ... }'. 
    // We create an object corresponding to the keyword, and then have that object
    // parse its own internals. The GRF file is nothing more than a long list of 
    // such records. Reading the text should result in the same data structure as 
    // reading the equivalent binary file. 
    while (is.peek().type != TokenType::Terminator)
    {
        TokenValue token = is.peek();
        RecordType type  = RecordFromName(token.value);
        std::shared_ptr<Record> record = make_record(type);
        m_records.push_back(record);
        record->parse(is);
    }    
}