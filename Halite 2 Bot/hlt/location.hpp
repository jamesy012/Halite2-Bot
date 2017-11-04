#pragma once

#include <ostream>

#include "constants.hpp"
#include "util.hpp"

#include "../Vector2.h"

namespace hlt {
    struct Location {
		Vector2 m_Pos;

        double get_distance_to(const Location& target) const {
            const double dx = m_Pos.m_X - target.m_Pos.m_X;
            const double dy = m_Pos.m_Y - target.m_Pos.m_Y;
            return std::sqrt(dx*dx + dy*dy);
        }

        int orient_towards_in_deg(const Location& target) const {
            return util::angle_rad_to_deg_clipped(orient_towards_in_rad(target));
        }

        double orient_towards_in_rad(const Location& target) const {
            const double dx = target.m_Pos.m_X - m_Pos.m_X;
            const double dy = target.m_Pos.m_Y - m_Pos.m_Y;

            return std::atan2(dy, dx) + 2 * M_PI;
        }

        Location get_closest_point(const Location& target, const double target_radius) const {
            const double radius = target_radius + constants::MIN_DISTANCE_FOR_CLOSEST_POINT;
            const double angle_rad = target.orient_towards_in_rad(*this);

            const double x = target.m_Pos.m_X + radius * std::cos(angle_rad);
            const double y = target.m_Pos.m_Y + radius * std::sin(angle_rad);

            return { Vector2(x, y) };
        }

        friend std::ostream& operator<<(std::ostream& out, const Location& location);
    };

    static bool operator==(const Location& l1, const Location& l2) {
        return l1.m_Pos.m_X == l2.m_Pos.m_X && l1.m_Pos.m_Y == l2.m_Pos.m_Y;
    }
}
