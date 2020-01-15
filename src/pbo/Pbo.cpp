#include "grad_aff/pbo/Pbo.h"

#include <openssl/sha.h>

#include <boost/algorithm/string.hpp>
namespace ba = boost::algorithm;

grad_aff::Pbo::Pbo(std::string pboFilename) {
    this->is = std::make_shared<std::ifstream>(pboFilename, std::ios::binary);
    this->pboName = ((fs::path)pboFilename).replace_extension("").string();
};

grad_aff::Pbo::Pbo(std::vector<uint8_t> data, std::string pboName) {
    this->is = std::make_shared<std::stringstream>(std::string(data.begin(), data.end()));
    this->pboName = pboName;
}

bool grad_aff::Pbo::readPbo(bool withData, bool checkData) {
    //std::ifstream ifs(filename, std::ios::binary);
    auto initalZero = readBytes(*is, 1);
    if (initalZero[0] != 0) {
        throw std::runtime_error("Invalid file/no inital zero");
    }
    auto magicNumber = readBytes<uint32_t>(*is);
    if (magicNumber != 0x56657273) {
        throw std::runtime_error("Invalid file/magic number");
    }

    //pboName = filename;

    auto sixteenZeros = readBytes(*is, 16);
    while (peekBytes<uint8_t>(*is) != 0)
    {
        productEntries.insert({ readZeroTerminatedString(*is), readZeroTerminatedString(*is) });
    }

    readBytes<uint8_t>(*is);

    // Entry
    while (peekBytes<uint16_t>(*is) != 0) {
        Entry entry;
        entry.filename = ba::to_lower_copy(readZeroTerminatedString(*is));
        entry.packingMethod = readBytes<uint32_t>(*is);
        entry.orginalSize = readBytes<uint32_t>(*is);
        entry.reserved = readBytes<uint32_t>(*is);
        entry.timestamp = readBytes<uint32_t>(*is);
        entry.dataSize = readBytes<uint32_t>(*is);
        entries.push_back(entry);
    }

    auto nullBytes = readBytes(*is, 21);
    dataPos = is->tellg();
    
    if (!withData)
        return false;

    for (auto& entry : entries) {
        entry.data = readBytes(*is, entry.dataSize);
    }

    if (!checkData)
        return false;

    auto preHashPos = is->tellg();
    is->seekg(0);

    auto rawPboData = readBytes(*is, preHashPos);

    auto nullByte = readBytes(*is, 1);
    std::vector<uint8_t> hash = readBytes(*is, 20);

    SHA_CTX context;
    if (!SHA1_Init(&context)) {
        throw std::runtime_error("SHA1 Init failed!");
    }

    std::vector<uint8_t> calculatedHash(20);
    
    if (!SHA1_Update(&context, reinterpret_cast<const uint8_t*>(rawPboData.data()), rawPboData.size())) {
        throw std::runtime_error("SHA1 Update failed!");
    }

    if (!SHA1_Final(calculatedHash.data(), &context)) {
        throw std::runtime_error("SHA1 Final failed!");
    }

    return (calculatedHash == hash);
}

void grad_aff::Pbo::extractPbo(fs::path outPath)
{
    outPath = outPath / pboName;
    for (auto& entry : entries) {

        auto pathWithoutFilename = (outPath / entry.filename).remove_filename();

        if (!fs::exists(pathWithoutFilename)) {
            fs::create_directories(pathWithoutFilename);
        }

        std::ofstream ofs(outPath / entry.filename, std::ios::binary);
        writeBytes(ofs, entry.data);
        ofs.close();
    }
}


void grad_aff::Pbo::extractSingleFile(fs::path entryName, fs::path outPath, bool fullPath) 
{
    if (entries.size() == 0) {
        this->readPbo(false);
    }

    for (auto& entry : entries) {
        if (entry.filename == entryName) {
            auto writePath = outPath;
            if (fullPath) {
                writePath = outPath / pboName/ entry.filename;
            }
            else {
                writePath = outPath / fs::path(entry.filename).filename();
            }

            if (entry.data.size() == 0) {
                this->readSingleData(entry.filename);
            }

            auto pathWithoutFilename = writePath;
            pathWithoutFilename.remove_filename();
            if (!fs::exists(pathWithoutFilename)) {
                fs::create_directories(pathWithoutFilename);
            }

            std::ofstream ofs(writePath, std::ios::binary);
            writeBytes(ofs, entry.data);
            ofs.close();
            return;
        }
    }
}


void grad_aff::Pbo::readSingleData(fs::path searchEntry) {
    if (entries.size() == 0) {
        this->readPbo(false);
    }
    
    is->seekg(this->dataPos);

    std::streamoff targetDataOffset = 0;

    for (auto &entry : entries) {
        if (entry.filename == searchEntry) {
            is->seekg(targetDataOffset, std::ios::cur);
            entry.data = readBytes(*is, entry.dataSize);
        }
        else {
            targetDataOffset += entry.dataSize;
        }
    }
}

void grad_aff::Pbo::writePbo(fs::path outPath) {

    if (outPath != "" && fs::exists(outPath)) {
        fs::create_directories(outPath);
    }
    std::ofstream ofs(outPath / pboName, std::ios::binary);
    //ofs.rdbuf();
    // write magic
    writeBytes(ofs, { 0x00 });
    writeBytes<uint32_t>(ofs, 0x56657273);

    // write zero
    for (int i = 0; i < 16; i++) {
        writeBytes(ofs, { 0 });
    }

    // write header entries
    for (auto& headEntry : productEntries) {
        writeZeroTerminatedString(ofs, headEntry.first);
        writeZeroTerminatedString(ofs, headEntry.second);
    }
    writeBytes<uint8_t>(ofs, 0);

    // Write Header
    for (auto& entry : entries) {
        writeZeroTerminatedString(ofs, entry.filename.string());
        writeBytes<uint32_t>(ofs, entry.packingMethod);
        writeBytes<uint32_t>(ofs, entry.orginalSize);
        writeBytes<uint32_t>(ofs, entry.reserved);
        writeBytes<uint32_t>(ofs, entry.timestamp);
        writeBytes<uint32_t>(ofs, entry.dataSize);
    }

    for (int i = 0; i < 21; i++) {
        writeBytes(ofs, { 0x00 });
    }

    for (auto& entry : entries) {
        writeBytes(ofs, entry.data);
    }
    writeBytes(ofs, { 0x00 });
    ofs.close();

    //std::*istream *is(path / pboName, std::ios::binary | std::ios::ate);
    auto size = is->tellg();
    is->seekg(0);
    auto rawPboData = readBytes(*is, (std::streamsize)size - 1);


    SHA_CTX context;
    if (!SHA1_Init(&context)) {
        throw std::runtime_error("SHA1 Init failed");
    }

    std::vector<uint8_t> calculatedHash(20);

    if (!SHA1_Update(&context, reinterpret_cast<const unsigned char*>(rawPboData.data()), rawPboData.size())) {
        throw std::runtime_error("SHA1 Update failed");
    }

    if (!SHA1_Final(reinterpret_cast<unsigned char*>(calculatedHash.data()), &context)) {
        throw std::runtime_error("SHA1 Final failed");
    }

    std::ofstream ofsHash(outPath / pboName, std::ios::binary | std::ios::app);
    writeBytes(ofsHash, calculatedHash);
    ofsHash.close();

}

std::vector<uint8_t> grad_aff::Pbo::getEntryData(fs::path entryPath) {
    if (entries.size() == 0)
        readPbo(false);

    if (ba::istarts_with(entryPath.string(), productEntries["prefix"])) {
        entryPath = (fs::path)ba::to_lower_copy(entryPath.string().substr(productEntries["prefix"].size() + 1));
    }

    for (auto& entry : entries) {
        if (entry.filename == entryPath) { // || ba::iequals((productEntries["prefix"] / entry.filename).string(), searchEntry)) {
            if (entry.data.size() == 0) {
                readSingleData(entry.filename);
            }
            return entry.data;
        }
    }
    return {};
}

void grad_aff::Pbo::setPboName(std::string pboName)
{
    this->pboName = pboName;
}
