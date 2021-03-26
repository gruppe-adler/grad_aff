#pragma once

#include <cstdint>
#include <streambuf>

#include <boost/iostreams/stream.hpp>

namespace bi = boost::iostreams;

class BaseStream {
public:
    typedef typename char char_type;
    struct category :
        bi::seekable_device_tag,
        bi::closable_tag
    {};

    virtual ~BaseStream() {}

    virtual std::streamsize read(char_type* s, std::streamsize n);
    virtual std::streamsize write(const char_type* s, std::streamsize n);

    virtual bi::stream_offset seek(bi::stream_offset off, std::ios_base::seekdir way);
    virtual void close();

protected:
    typedef typename int64_t size_type;
    size_type pos_;
};

