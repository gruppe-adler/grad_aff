#include "paa/Tagg.h"

#include "core/AffCompat.h"

grad::aff::Paa::Tagg* grad::aff::Paa::TaggCreate() {
    return new grad::aff::Paa::Tagg();
}

grad::aff::Paa::Tagg* grad::aff::Paa::TaggClone(Tagg* rhsTagg) {
    auto newTagg = TaggCreate();
    *newTagg = *rhsTagg;
    return newTagg;
}

void grad::aff::Paa::TaggDestroy(Tagg* tagg) {
    delete tagg;
}

void grad::aff::Paa::TaggSetSignature(Tagg* tagg, const char* signature, size_t size) {
    std::string sig(signature, size);
    tagg->signature = sig;
}

size_t grad::aff::Paa::TaggGetSignatureSize(Tagg* tagg) {
    return tagg->signature.size();
}

void grad::aff::Paa::TaggGetSignature(Tagg* tagg, char** signature, size_t size) {
    if (size < tagg->signature.size()) {
        *signature = '\0';
    }
    else {
        *signature = aff_strdup(tagg->signature.c_str());
    }
}

size_t grad::aff::Paa::TaggGetDataSize(Tagg* tagg) {
    return tagg->data.size();
}

void grad::aff::Paa::TaggSetData(Tagg* tagg, uint8_t* data, size_t size) {
    std::vector<uint8_t> dataVec(data, data + size);
    tagg->data = dataVec;
}

void grad::aff::Paa::TaggGetData(Tagg* tagg, uint8_t** data, size_t size) {
    if (size < tagg->data.size()) {
        *data = nullptr;
    }
    else {
        std::memcpy(*data, tagg->data.data(), tagg->data.size());
    }
}

