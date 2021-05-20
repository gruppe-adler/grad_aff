#include "core/AffBase.h"

#include <boost/interprocess/streams/bufferstream.hpp>
#include <filesystem>
#include <sstream>

namespace bip = boost::interprocess;
namespace fs = std::filesystem;

#ifdef _WIN32
#include "core/devices/IStreamDevice.hpp"
#endif

void grad::aff::core::AffBase::read(std::filesystem::path path) {
    read(path.string());
}

void grad::aff::core::AffBase::read(std::string path) {
    auto fsPath = fs::path(path.c_str()); // gcc needs this
    if (!fs::exists(fsPath)) {
        throw std::runtime_error("file not found");
    }
    stream = std::dynamic_pointer_cast<std::basic_iostream<char>>(std::make_shared<std::fstream>(fsPath, std::ios::binary | std::ios::in | std::ios::out));
    readFromStream();
}

void grad::aff::core::AffBase::read(std::wstring path) {
    auto fsPath = fs::path(path.c_str()); // gcc needs this
    if (!fs::exists(fsPath)) {
        throw std::runtime_error("file not found");
    }
    stream = std::dynamic_pointer_cast<std::basic_iostream<char>>(std::make_shared<std::fstream>(fs::path(fsPath), std::ios::binary | std::ios::in | std::ios::out));
    readFromStream();
}

void grad::aff::core::AffBase::read(const char* path) {
    read(std::string(path));
}

void grad::aff::core::AffBase::read(std::vector<char> data) {
    streamData = data;
    stream = std::dynamic_pointer_cast<std::basic_iostream<char>>(std::make_shared<bip::bufferstream>(streamData.data(), streamData.size()));
    readFromStream();
}

void grad::aff::core::AffBase::read(std::vector<uint8_t> data) {
    std::copy(data.begin(), data.end(), std::back_inserter(streamData));
    stream = std::dynamic_pointer_cast<std::basic_iostream<char>>(std::make_shared<bip::bufferstream>(streamData.data(), streamData.size()));
    readFromStream();
}

#ifdef _WIN32
void grad::aff::core::AffBase::read(IStream* istream) {
    stream = std::dynamic_pointer_cast<std::basic_iostream<char>>(std::make_shared<boost::iostreams::stream<devices::IStreamDevice>>(istream));
    readFromStream();
}

void grad::aff::core::AffBase::write(IStream* istream) {
    writeToStream(std::dynamic_pointer_cast<std::basic_iostream<char>>(std::make_shared<boost::iostreams::stream<devices::IStreamDevice>>(istream)));
}
#endif

void grad::aff::core::AffBase::write() {
    writeToStream(stream);
}

void grad::aff::core::AffBase::write(std::filesystem::path path) {
    write(path.string());
}

void grad::aff::core::AffBase::write(const char* path) {
    write(std::string(path));
}

void grad::aff::core::AffBase::write(std::string path) {
    writeToStream(std::dynamic_pointer_cast<std::basic_iostream<char>>(std::make_shared<std::fstream>(path, std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc)));
}

void grad::aff::core::AffBase::write(std::vector<uint8_t>& data) {
    auto os = std::make_shared<std::stringstream>();
    writeToStream(std::dynamic_pointer_cast<std::basic_iostream<char>>(os));

    auto startPos = os->tellg();
    os->seekg(0, std::ios::end);
    auto size = std::size_t(os->tellg() - startPos);
    os->seekg(0, std::ios::beg);

    data.resize(size);
    os->read(reinterpret_cast<char*>(data.data()), std::streamsize(data.size()));
}
