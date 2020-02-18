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

    this->numberOfLods = readBytes<uint32_t>(is);


}

void grad_aff::Odol::readModelInfo() {
    std::vector<float_t> lodTypes(this->numberOfLods);

    for (size_t i = 0; i < this->numberOfLods; i++) {
        lodTypes.push_back(readBytes<float_t>(is));
    }

    auto index = readBytes<uint32_t>(is);

    auto memLodSpehre = readBytes<float_t>(is);
    auto geoLodSpehre = readBytes<float_t>(is);
    auto geoLodSpehre = readBytes<float_t>(is);

    std::vector<uint32_t> pointFlags(3);
    for (size_t i = 0; i < pointFlags.size(); i++) {
        pointFlags.push_back(readBytes<uint32_t>(is));
    }

    auto offset1 = readXYZTriplet(is);
    auto mapIconColor = readBytes<uint32_t>(is);
    auto mapselectedColor = readBytes<uint32_t>(is);

    auto viewDensity = readBytes<float_t>(is);

    auto bboxMinPosition = readXYZTriplet(is);
    auto bboxMaxPosition = readXYZTriplet(is);
    auto centreOfGravity = readXYZTriplet(is);
    auto offset2 = readXYZTriplet(is);
    auto cogOffset = readXYZTriplet(is);

    std::vector<XYZTriplet> modelMassVectors(3);
    for (size_t i = 0; i < modelMassVectors.size(); i++) {
        modelMassVectors.push_back(readXYZTriplet(is));
    }

}