#pragma once

#include "collision.hpp"
#include "map.hpp"
#include "move.hpp"
#include "util.hpp"

#include "../Vector2.h"

namespace hlt {
	namespace navigation {
		static bool check_and_add_entity_between(
			std::vector<const Entity *>& entities_found,
			const Location& start,
			const Location& target,
			const Entity& entity_to_check) {
			const Location &location = entity_to_check.location;
			if (location == start || location == target) {
				return false;
			}
			if (collision::segment_circle_intersect(start, target, entity_to_check, constants::FORECAST_FUDGE_FACTOR)) {
				entities_found.push_back(&entity_to_check);
				return true;
			}
			return false;
		}

		static std::vector<const Entity *> objects_between(const Map& map, const Location& start, const Location& target) {
			std::vector<const Entity *> entities_found;

			for (const Planet& planet : map.planets) {
				check_and_add_entity_between(entities_found, start, target, planet);
			}

			for (const auto& player_ship : map.ships) {
				for (const Ship& ship : player_ship.second) {
					check_and_add_entity_between(entities_found, start, target, ship);
				}
			}

			for (const auto& movePos : map.m_MovePositions) {
				//it only uses location and radius for it's check
				Entity entity = Entity();
				entity.location.m_Pos = movePos.m_Position;
				entity.radius = hlt::constants::SHIP_PREDICTED_MOVE_AVOIDANCE_RADIUS;
				bool didAdd = check_and_add_entity_between(entities_found, start, target, entity);
				//if (didAdd) {
				//	Log::log(std::string(start.m_Pos) + " collision with " + std::string(target.m_Pos));
				//}
			}


			return entities_found;
		}

		static possibly<Move> navigate_ship_towards_target(
			Map& map,
			Ship& ship,
			const Location& target,
			const int max_thrust,
			const bool avoid_obstacles,
			const int max_corrections,
			const double angular_step_rad) {
			if (max_corrections <= 0) {
				return { Move::noop(), false };
			}

			double distance = ship.location.get_distance_to(target);
			double angle_rad = ship.location.orient_towards_in_rad(target);
			/*
			if (avoid_obstacles && !objects_between(map, ship.location, target).empty()) {
				const double new_target_dx = cos(angle_rad + angular_step_rad) * distance;
				const double new_target_dy = sin(angle_rad + angular_step_rad) * distance;
				const Location new_target = { Vector2(ship.location.m_Pos.m_X + new_target_dx, ship.location.m_Pos.m_Y + new_target_dy) };
			
				return navigate_ship_towards_target(
					map, ship, new_target, max_thrust, true, (max_corrections - 1), angular_step_rad);
			}
			*/
			if (avoid_obstacles) {
				if (!objects_between(map, ship.location, target).empty()) {
					bool pathFailed = true;
					for (int i = 0; i < max_corrections; i++) {
						int scale = i % 2 == 0 ? 1 : -1;

						int value = (i + 2) / 2 * scale;

						const double new_target_dx = cos(angle_rad + angular_step_rad*value) * distance;
						const double new_target_dy = sin(angle_rad + angular_step_rad*value) * distance;
						const Location new_target = { Vector2(ship.location.m_Pos.m_X + new_target_dx, ship.location.m_Pos.m_Y + new_target_dy) };

						if (objects_between(map, ship.location, new_target).empty()) {
							distance = ship.location.get_distance_to(new_target);
							angle_rad = ship.location.orient_towards_in_rad(new_target);
							pathFailed = false;
							break;
						}
					}
					if (pathFailed) {
						return { Move::noop(), false };
					}
				}
			}
			

			int thrust;
			if (distance < max_thrust) {
				// Do not round up, since overshooting might cause collision.
				thrust = (int) distance;
			} else {
				thrust = max_thrust;
			}

			const int angle_deg = util::angle_rad_to_deg_clipped(angle_rad);


			//calculating the postion this object will be after this 
			//ONLY DO THIS IF WE HAVE LESS THEN NUM_OF_UNDOCKED_SHIPS_FOR_SHIP_MOVE_PREDICTION SHIPS
			if (map.m_NumberOfFriendlyUndockedShips <= constants::NUM_OF_UNDOCKED_SHIPS_FOR_SHIP_MOVE_PREDICTION) {
				Vector2 direction = (target.m_Pos - ship.location.m_Pos).normalized() * thrust;
				Vector2 posNextTurn = ship.location.m_Pos;

				Map::MovePositions mp;
				for (int i = 1; i <= thrust; i++) {
					mp = { posNextTurn * thrust/i, &ship };
					map.m_MovePositions.push_back(mp);
				}

				//Log::log("Move prediction: " + std::to_string(ship.entity_id) + " " + std::string(ship.location.m_Pos) + " " + std::string(direction) + " " + std::string(posNextTurn));
			}

			return { Move::thrust(ship.entity_id, thrust, angle_deg), true };
		}

		static possibly<Move> navigate_ship_to_dock(
			Map& map,
			Ship& ship,
			Entity& dock_target,
			int max_thrust) {
			int max_corrections = constants::MAX_NAVIGATION_CORRECTIONS;
			bool avoid_obstacles = true;
			double angular_step_rad = M_PI / 180.0;
			Location target = ship.location.get_closest_point(dock_target.location, dock_target.radius);

			return navigate_ship_towards_target(
				map, ship, target, max_thrust, avoid_obstacles, max_corrections, angular_step_rad);
		}

	}
}
