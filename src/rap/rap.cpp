#include "grad_aff/rap/rap.h"

grad_aff::Rap::Rap() {

}

grad_aff::Rap::Rap(std::string pboFilename) {
    this->is = std::make_shared<std::ifstream>(pboFilename, std::ios::binary);
    this->rapName = ((fs::path)pboFilename).replace_extension("").string();
};

grad_aff::Rap::Rap(std::vector<uint8_t> data, std::string rapName) {
    this->is = std::make_shared<std::stringstream>(std::string(data.begin(), data.end()));
    this->rapName = rapName;
}

std::shared_ptr<RapArray> grad_aff::Rap::readArray(std::istream& is) {
    auto ret = std::make_shared<RapArray>();
    ret->type = readBytes<uint8_t>(is);
    ret->name = readZeroTerminatedString(is);
    ret->nElements = readCompressedInteger(is);
    for (uint32_t i = 0; i < ret->nElements; i++) {
        auto type = readBytes<uint8_t>(is);
        switch (type)
        {
        case 0:
        case 4:
        {
            ret->arrayElements.push_back({ readZeroTerminatedString(is) });
            break;
        }
        case 1:
        {
            ret->arrayElements.push_back({ readBytes<float_t>(is) });
            break;
        }
        case 2:
        {
            ret->arrayElements.push_back({ readBytes<int32_t>(is) });
            break;
        }
        case 3:
        {
            ret->arrayElements.push_back({ *readArray(is) });
            break;
        }
        default:
            break;
        }
    }
    return ret;
}

std::shared_ptr<ClassEntry> grad_aff::Rap::readClassEntry(std::istream& is) {
    std::shared_ptr<ClassEntry> classEntry;

    auto type = readBytes<uint8_t>(is);
    //auto name = readZeroTerminatedString(is);

    switch (type)
    {
    case 0:
    {
        auto rapClass = std::make_shared<RapClass>();
        rapClass->type = type;
        rapClass->name = readZeroTerminatedString(is);
        rapClass->offsetToClassBody = readBytes<uint32_t>(is);
        classEntry = rapClass;
        break;
    }
    case 1:
    {
        auto rapValue = std::make_shared<RapValue>();
        rapValue->type = type;
        rapValue->subType = readBytes<uint8_t>(is);
        rapValue->name = readZeroTerminatedString(is);
        switch (rapValue->subType)
        {
        case 0:
        case 4:
            rapValue->value = readZeroTerminatedString(is);
            break;
        case 1:
            rapValue->value = readBytes<float_t>(is);
            break;
        case 2:
            rapValue->value = readBytes<int32_t>(is);
            break;
        case 3:
            // array, not used
            break;
        default:
            break;
        }
        classEntry = rapValue;
        break;
    }
    case 2:
    {
        //auto rapArray = std::make_shared<RapArray>();
        //is.seekg((name.size() + 1) * -1, std::ios::cur);
        //rapArray = readArray(is);
        is.seekg(-1, std::ios::cur);
        classEntry = readArray(is);
        //rapArray.type = type;
        //rapArray.name = name;
        //rapArray.
        break;
    }
    case 3:
    {
        auto rapExtern = std::make_shared<RapExtern>();
        rapExtern->name = readZeroTerminatedString(is);
        rapExtern->type = type;
        classEntry = rapExtern;
        break;
    }
    case 4:
    {
        auto rapDelete = std::make_shared<RapDelete>();
        rapDelete->name = readZeroTerminatedString(is);
        rapDelete->type = type;
        classEntry = rapDelete;
        break;
    }
    case 5:
        break;
    default:
        break;
    }
    return classEntry;
}

std::string grad_aff::Rap::readClassBody(std::istream& is, std::vector<std::shared_ptr<ClassEntry>>& classes) {
    /*
    for (auto& class_ : classes) {
        if (class_->type == 0) {
            auto start = is.tellg();
            //is.seekg(class_->)
            auto cl = std::static_pointer_cast<RapClass>(class_);
            is.seekg(cl->offsetToClassBody, std::ios::beg);
        }
    }*/

    auto classBodyInheritedClassname = readZeroTerminatedString(is);
    auto nEntries = readCompressedInteger(is);

    //std::vector<std::shared_ptr<ClassEntry>> classEntries;

    for (uint32_t i = 0; i < nEntries; i++) {        
        classes.push_back(readClassEntry(is));
    }

    for (auto& entry : classes) {
        if (entry->type == 0) {
            auto rapClassPtr = std::static_pointer_cast<RapClass>(entry);
            auto classEntries = std::vector<std::shared_ptr<ClassEntry>>();
            
            is.seekg(rapClassPtr->offsetToClassBody, std::ios::beg);
            rapClassPtr->inheritedClassname = readClassBody(is, classEntries);
            rapClassPtr->classEntries = classEntries;
        }
    }

    return classBodyInheritedClassname;
}

//void grad_aff::Rap::re

void grad_aff::Rap::readRap() {
    this->classEntries.clear();
    //auto signature = readBytes(*is, 4);
    // TODO assert;

    auto initalZero = readBytes<uint8_t>(*is);
    assert(initalZero == 0);
    auto rap = readString(*is, 3);
    assert(rap == "raP");

    auto always0 = readBytes<uint32_t>(*is);
    assert(always0 == 0);
    auto always8 = readBytes<uint32_t>(*is);
    assert(always8 == 8);

    auto offsetToEnums = readBytes<uint32_t>(*is);

    auto cfgPatchInherited = readClassBody(*is, classEntries);

    assert(cfgPatchInherited == "");

    for (auto& entry : classEntries) {
        if (entry->name == "Stage0") {
            auto rapClassPtr = std::static_pointer_cast<RapClass>(entry);

            for (auto& subEntry : rapClassPtr->classEntries) {
                if (subEntry->name == "texture") {
                    auto texturePtr = std::static_pointer_cast<RapValue>(subEntry);
                    auto str = std::get<std::string>(texturePtr->value);
                }
            }
        }
    }

    //auto rapClassPtr = std::static_pointer_cast<RapClass>(classEntries[0]);
    //auto rapClassPtr2 = std::static_pointer_cast<RapClass>(rapClassPtr->classEntries[0]);
    //auto rapClassPtr3 = std::static_pointer_cast<RapClass>(rapClassPtr->classEntries[1]);

    // TODO enums

    //readClassBody(*is, classEntries);
}

// lol
void grad_aff::Rap::preprocess(std::string& input) {
    std::vector< boost::iterator_range<std::string::iterator> > findVec;
    boost::find_all(findVec, input, "//");

    auto com = input.find("//");

    while (com != input.npos) {
        auto nL = input.find('\n', com);

        input.erase(com, nL - com);

        com = input.find("//");
    }

    // Do this properly one day
    boost::replace_all(input, " true", "1");
    boost::replace_all(input, "=true", "=1");
    boost::replace_all(input, " false", "0");
    boost::replace_all(input, "=false", "=0");

}

void grad_aff::Rap::convertClass(std::vector<std::shared_ptr<ClassEntry>>& entries, std::shared_ptr<ClassEntry>& rootPtr, std::vector<std::shared_ptr<ClassEntry>>::iterator& it) {

    auto rc = std::make_shared<RapClass>();
    rc->name = (*it)->name;
    rc->type = (*it)->type;
    rootPtr = rc;
   
    it++;

    //auto rc = std::static_pointer_cast<RapClass>(*it);

    for (it; it != entries.end(); it++) {

        if (!(*it)) {
            return;
        }

        rc->classEntries.push_back(*it);

        if ((*it)->type == 0) {
            convertClass(entries, rc->classEntries.back(), it);
        }
    }
}

void grad_aff::Rap::parseConfig(fs::path path) {

    this->classEntries.clear();

    std::ifstream file(path);
    std::string stringInput((std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>());

    preprocess(stringInput);

    //std::cout << stringInput << std::endl;

    std::vector<std::shared_ptr<ClassEntry>> entries;

    pegtl::memory_input in(stringInput, std::string(""));
    pegtl::parse< grad_aff::RapParser2::topLevel, grad_aff::RapParser2::action >(in, entries);

    

    for (auto it = entries.begin(); it != entries.end(); it++) {
        this->classEntries.push_back(*it);

        if ((*it)->type == 0) {
            convertClass(entries, this->classEntries.back(), it);
        }
    }

}