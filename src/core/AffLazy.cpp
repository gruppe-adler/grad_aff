#include "core/AffLazy.h"

#include <boost/interprocess/streams/bufferstream.hpp>

#ifdef _WIN32
#include "core/devices/IStreamDevice.hpp"
#endif

void grad::aff::core::AffLazy::readLazy(std::filesystem::path path) {
    readLazy(path.string());
}

void grad::aff::core::AffLazy::readLazy(std::string path) {
    if (!std::filesystem::exists(path)) {
        throw std::runtime_error("file not found");
    }
    stream = std::dynamic_pointer_cast<std::basic_iostream<char>>(std::make_shared<std::fstream>(path, std::ios::binary | std::ios::in | std::ios::out));
    readLazyFromStream(true);
}

void grad::aff::core::AffLazy::readLazy(std::wstring path) {
    if (!std::filesystem::exists(path)) {
        throw std::runtime_error("file not found");
    }
    stream = std::dynamic_pointer_cast<std::basic_iostream<char>>(std::make_shared<std::fstream>(path, std::ios::binary | std::ios::in | std::ios::out));
    readLazyFromStream(true);
}

void grad::aff::core::AffLazy::readLazy(const char* path) {
    readLazy(std::string(path));
}

void grad::aff::core::AffLazy::readLazy(std::vector<char> data) {
    streamData = data;
    stream = std::dynamic_pointer_cast<std::basic_iostream<char>>(std::make_shared<boost::interprocess::bufferstream>(streamData.data(), streamData.size()));
    readLazyFromStream(true);
}

void grad::aff::core::AffLazy::readLazy(std::vector<uint8_t> data) {
    std::copy(data.begin(), data.end(), std::back_inserter(streamData));
    stream = std::dynamic_pointer_cast<std::basic_iostream<char>>(std::make_shared<boost::interprocess::bufferstream>(streamData.data(), streamData.size()));
    readLazyFromStream(true);
}

#ifdef _WIN32
void grad::aff::core::AffLazy::readLazy(IStream* istream) {
    stream = std::dynamic_pointer_cast<std::basic_iostream<char>>(std::make_shared<boost::iostreams::stream<grad::aff::core::devices::IStreamDevice>>(istream));
    readLazyFromStream(true);
}
#endif

void grad::aff::core::AffLazy::readFromStream() {
    readLazyFromStream(false);
}
