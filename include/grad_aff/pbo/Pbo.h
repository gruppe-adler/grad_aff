#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>
#include <sstream>
#include <map>

#include "../pbo/Entry.h"

#include "../StreamUtil.h"

namespace fs = std::filesystem;

namespace grad_aff {
    class Pbo {

        std::shared_ptr<std::istream> is;

        
        std::streampos dataPos = 0;
    public:
        Pbo(std::string filename);
        Pbo(std::vector<uint8_t> data, std::string pboName = "");
        bool readPbo(bool withData = true, bool checkHash = false);
        void extractPbo(fs::path outPath);
        void Pbo::extractSingleFile(fs::path entryName, fs::path outPath, bool fullPath = true);

        void writePbo(fs::path outPath);

        void setPboName(std::string pboName);

        void readSingleData(fs::path entryPath);

        void addFile(fs::path file);
        void addDir(fs::path dir);
        void removeFile(fs::path file);

        std::vector<uint8_t> getEntryData(fs::path entryPath);

        std::string pboName = "";
        std::vector<Entry> entries = {};
        std::map<std::string, std::string> productEntries = {};
    };
}