#include "grad_aff/p3d/odol.h"


grad_aff::Odol::Odol(std::string filename) {
    this->is = std::make_shared<std::ifstream>(filename, std::ios::binary);
};

grad_aff::Odol::Odol(std::vector<uint8_t> data) {
    this->is = std::make_shared<std::stringstream>(std::string(data.begin(), data.end()));
}

void grad_aff::Odol::readOdol() {
    is->seekg(0);
    filename = readString(is, 4);
    version = readBytes<uint32_t>(is);

    if (version >= 58) {
        auto p3dPrefix = readZeroTerminatedString(is);
    }

    auto numberOfLods = readBytes<uint32_t>(is);


}