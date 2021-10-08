#pragma once

#include <variant>
#include <string>
#include <vector>
#include <ostream>
#include <memory>
#include <cmath>

struct ClassEntry {
    uint8_t type;
    std::string name;
    virtual void write(std::ostream& os, std::string spacing) = 0;
};

struct RapClass : ClassEntry {
    uint32_t offsetToClassBody;
    std::string inheritedClassname = "";
    std::vector<std::shared_ptr<ClassEntry>> classEntries = {};

    void write(std::ostream& os, std::string spacing) override {
        os << name;
        if (!inheritedClassname.empty()) {
            os << ":" << inheritedClassname;
        }
        os << "{";

        for (auto var : classEntries)
        {
            var->write(os, spacing);
        }

        os << "};";
    }
};

//struct RapArray : ClassEntry {};

struct ValueVisitor {
    std::ostream& os;
    ValueVisitor(std::ostream& os) : os(os) {};
    void operator()(int32_t i) const {
        os << i;
    }
    void operator()(float_t f) const {
        os << f;
    }
    void operator()(const std::string& s) const {
        os << "\"" << s << " \"\n";
    }
    //void operator()(const RapArray& ra) const {
    //    //  ra.write(os, "    ");
    //}
};

struct RapArray : ClassEntry {
    uint32_t nElements;
    std::vector<std::variant<std::string, float_t, int32_t, RapArray>> arrayElements;
    void write(std::ostream& os, std::string spacing) override {
        os << spacing << name << "[] = {";
        for (size_t i = 0; i < arrayElements.size(); i++) {
            //std::visit(ValueVisitor(os), arrayElements[i]);
            if (i <= arrayElements.size()) {
                os << ", ";
            }
        }
        os << "}";
    }
};



struct RapValue : ClassEntry {
    uint8_t subType;
    std::variant<std::string, float_t, int32_t> value;

    void write(std::ostream& os, std::string spacing) {
        os << spacing << this->name << " = ";
        std::visit(ValueVisitor(os), value);
        os << ";\n";
    }
};

struct RapArrayFlag : RapArray {
    uint32_t flag;
};

struct RapExtern : ClassEntry {
    void write(std::ostream& os, std::string spacing) override {
        os << spacing << " class " << this->name << ";\n";
    }
};
struct RapDelete : ClassEntry {
    void write(std::ostream& os, std::string spacing) override {

    }
};

//
//inline void RapArray::write(std::ostream& os, std::string spacing) {
//
//}


/*
struct ArrayStruct {
    uint8_t type;
};

struct StringElement : ArrayElement {
    std::string stringConstant;
};

struct FloatElement : ArrayElement {
    float_t float
};
*/