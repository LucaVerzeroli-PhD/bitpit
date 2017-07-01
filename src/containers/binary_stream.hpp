/*---------------------------------------------------------------------------*\
 *
 *  bitpit
 *
 *  Copyright (C) 2015-2017 OPTIMAD engineering Srl
 *
 *  -------------------------------------------------------------------------
 *  License
 *  This file is part of bitpit.
 *
 *  bitpit is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU Lesser General Public License v3 (LGPL)
 *  as published by the Free Software Foundation.
 *
 *  bitpit is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 *  License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with bitpit. If not, see <http://www.gnu.org/licenses/>.
 *
\*---------------------------------------------------------------------------*/

#ifndef __BITPIT_BINARY_STREAM_HPP__
#define __BITPIT_BINARY_STREAM_HPP__

#include <vector>
#include <string>
#include <fstream>
#include <cstring>
#include <iostream>
#include <stdexcept>

// Forward declarations
namespace bitpit {

class IBinaryStream;
class OBinaryStream;

};

// Stream operators
template<typename T>
bitpit::IBinaryStream & operator>>(bitpit::IBinaryStream &stream, T &value);

template<>
bitpit::IBinaryStream & operator>>(bitpit::IBinaryStream &stream, std::string &value);

template<typename T>
bitpit::OBinaryStream & operator<<(bitpit::OBinaryStream &stream, const T &value);

template<>
bitpit::OBinaryStream & operator<<(bitpit::OBinaryStream &stream, const std::string &value);

// Binary stream
namespace bitpit{

class IBinaryStream {

template<typename T>
friend IBinaryStream & (::operator>>)(IBinaryStream &stream, T &value);

public:
    IBinaryStream(void);
    IBinaryStream(std::size_t capacity);
    IBinaryStream(const char *buffer, std::size_t capacity);
    IBinaryStream(const std::vector<char> &buffer);

    void open(const char *buffer, std::size_t capacity);
    bool eof() const;

    std::ifstream::pos_type tellg() const;
    bool seekg(std::size_t pos);
    bool seekg(std::streamoff offset, std::ios_base::seekdir way);

    char * data();
    const char * data() const;

    void setCapacity(std::size_t capacity);
    std::size_t capacity() const;

private:
    std::vector<char> m_buffer;
    std::size_t m_pos;

    template<typename T>
    void read(T &value);
    void read(char *data, std::size_t size);

};

class OBinaryStream {

template<typename T>
friend OBinaryStream & (::operator<<)(OBinaryStream &stream, const T &value);

public:
    OBinaryStream();
    OBinaryStream(std::size_t capacity);

    void open(std::size_t capacity);
    bool eof() const;

    std::ofstream::pos_type tellg() const;
    bool seekg(std::size_t pos);
    bool seekg(std::streamoff offset, std::ios_base::seekdir way);

    void squeeze();

    char * data();
    const char * data() const;

    void setCapacity(std::size_t capacity);
    std::size_t capacity() const;

private:
    std::vector<char> m_buffer;
    std::size_t m_pos;

    template<typename T>
    void write(const T &value);
    void write(const char *data, std::size_t size);
};

}

// Template implementation
#include "binary_stream.tpp"

#endif
