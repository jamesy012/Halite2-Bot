#include <cmath>

#include "location.hpp"

namespace hlt {
    std::ostream& operator<<(std::ostream& out, const Location& location) {
        out << '(' << location.m_Pos.m_X << ", " << location.m_Pos.m_Y << ')';
        return out;
    }
}
