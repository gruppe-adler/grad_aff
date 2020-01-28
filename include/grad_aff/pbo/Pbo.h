#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>
#include <sstream>
#include <map>

#include "../grad_aff.h"
#include "../StreamUtil.h"
#include "Entry.h"

#include <tsl/ordered_map.h>

namespace fs = std::filesystem;

namespace grad_aff {
    class GRAD_AFF_API Pbo {
        std::shared_ptr<std::istream> is;  
        std::streampos dataPos = 0;
        std::streampos preHashPos = 0;
    public:
        Pbo(std::string filename);
        Pbo(std::vector<uint8_t> data, std::string pboName = "");
        void readPbo(bool withData = true);
        bool checkHash();
        void extractPbo(fs::path outPath);
        void extractSingleFile(fs::path entryName, fs::path outPath, bool fullPath = true);

        void writePbo(fs::path outPath);
    
        void readSingleData(fs::path entryPath);

        void addFile(fs::path file);
        void addDir(fs::path dir);
        void removeFile(fs::path file);

        std::vector<uint8_t> getEntryData(fs::path entryPath);

        std::string pboName = "";
        //std::vector<Entry> entries = {};
        tsl::ordered_map<std::string, std::shared_ptr<Entry>> entries = {};
        std::map<std::string, std::string> productEntries = {};
        std::vector<uint8_t> hash;
    };
}