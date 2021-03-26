#pragma once

namespace grad::aff::core {

    template<typename T>
    T readBytes(std::istream& is) {
        T t = 0;
        is.read(reinterpret_cast<char*>(&t), sizeof(T));
        return t;
    }

}
