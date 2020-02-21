#pragma once

#include "../grad_aff.h"
#include "../StreamUtil.h"

#include <tao/pegtl.hpp>

#include "RapParser.h"
#include "ClassEntry.h"

#include <variant>
#include <vector>
#include <istream>
#include <filesystem>
#include <string>
#include <sstream>

namespace fs = std::filesystem;

using SubType = std::variant<RapClass, RapValue, RapArray, RapExtern, RapDelete, RapArrayFlag>;

namespace grad_aff {
    class GRAD_AFF_API Rap {
    private:
        std::shared_ptr<std::istream> is;

        std::shared_ptr<RapArray> readArray(std::istream& is);
        std::string readClassBody(std::istream& is, std::vector<std::shared_ptr<ClassEntry>>& classes);

        std::shared_ptr<ClassEntry> readClassEntry(std::istream& is);

    public:
        Rap();
        Rap(std::string filename);
        Rap(std::vector<uint8_t> data, std::string rapName = "");
        std::string rapName = "";

        std::vector<std::shared_ptr<ClassEntry>> classEntries = {};

        void readRap();
        void parseConfig(fs::path path);

        void readConfig(fs::path path);

        void preprocess(std::string& input);

        void convertClass(std::vector<std::shared_ptr<ClassEntry>>& entries, std::shared_ptr<ClassEntry>& rootPtr, std::vector<std::shared_ptr<ClassEntry>>::iterator& it);
    };
}