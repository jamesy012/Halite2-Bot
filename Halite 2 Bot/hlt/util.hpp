#pragma once

#define M_PI 3.14159265358979323846
#include <cmath>

namespace hlt {
    namespace util {
        static int angle_rad_to_deg_clipped(const double angleRad) {
            const long degUnclipped = lround(angleRad * 180.0 / M_PI);
            // Make sure return value is in [0, 360) as required by game engine.
            return static_cast<int>(((degUnclipped % 360L) + 360L) % 360L);
        }
    }
}
