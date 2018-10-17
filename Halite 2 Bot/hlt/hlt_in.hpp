#pragma once

#include <sstream>
#include <iostream>
#include "log.hpp"
#include "map.hpp"

#include <chrono>

namespace hlt {
	namespace in {
		static std::string get_string() {
			std::string result;
			std::getline(std::cin, result);
			return result;
		}

		static std::pair<EntityId, Ship> parse_ship(std::stringstream& iss, const PlayerId owner_id, unsigned int a_ShipIndex, Map* a_Map) {
			Ship ship;

			iss >> ship.entity_id;
			iss >> ship.location.m_Pos.m_X;
			iss >> ship.location.m_Pos.m_Y;
			iss >> ship.health;

			// No longer in the game, but still part of protocol.
			double vel_x_deprecated, vel_y_deprecated;
			iss >> vel_x_deprecated;
			iss >> vel_y_deprecated;

			int docking_status;
			iss >> docking_status;
			ship.docking_status = static_cast<ShipDockingStatus>(docking_status);

			iss >> ship.docked_planet;
			iss >> ship.docking_progress;
			iss >> ship.weapon_cooldown;

			ship.owner_id = owner_id;
			ship.radius = constants::SHIP_RADIUS;

			//calc shipVelocity
			if (a_Map != nullptr && ship.docking_status == ShipDockingStatus::Undocked) {

				auto shipLastTurn = a_Map->ship_map.at(owner_id).find(ship.entity_id);

				if (shipLastTurn == a_Map->ship_map.at(owner_id).end()) {
					ship.m_VelX = ship.m_VelY = 0;
				} else {
					Ship* thisShipLastTurn = &a_Map->ships.at(owner_id).at(shipLastTurn->second);
					ship.m_VelX = ship.location.m_Pos.m_X - thisShipLastTurn->location.m_Pos.m_X;
					ship.m_VelY = ship.location.m_Pos.m_Y - thisShipLastTurn->location.m_Pos.m_Y;
				}
			}


			return std::make_pair(ship.entity_id, ship);
		}

		static std::pair<EntityId, Planet> parse_planet(std::istream& iss) {
			Planet planet;

			iss >> planet.entity_id;
			iss >> planet.location.m_Pos.m_X;
			iss >> planet.location.m_Pos.m_Y;
			iss >> planet.health;
			iss >> planet.radius;
			iss >> planet.docking_spots;
			iss >> planet.current_production;
			iss >> planet.remaining_production;

			int owned;
			iss >> owned;
			if (owned == 1) {
				planet.owned = true;
				int owner;
				iss >> owner;
				planet.owner_id = static_cast<PlayerId>(owner);
			} else {
				planet.owned = false;
				int false_owner;
				iss >> false_owner;
				planet.owner_id = -1;
			}

			unsigned int num_docked_ships;
			iss >> num_docked_ships;

			planet.docked_ships.reserve(num_docked_ships);
			for (unsigned int i = 0; i < num_docked_ships; ++i) {
				EntityId ship_id;
				iss >> ship_id;
				planet.docked_ships.push_back(ship_id);
			}

			return std::make_pair(planet.entity_id, planet);
		}

		static Map* parse_map(const std::string& input, const int map_width, const int map_height, hlt::PlayerId a_PlayerID, Map* a_Map) {
			std::stringstream iss(input);

			int num_players;
			iss >> num_players;

			Map* map = new Map(map_width, map_height, a_PlayerID);

			for (int i = 0; i < num_players; ++i) {
				PlayerId player_id;
				int player_id_int;
				iss >> player_id_int;

				player_id = static_cast<PlayerId>(player_id_int);

				unsigned int num_ships;
				iss >> num_ships;

				std::vector<Ship>& ship_vec = map->ships[player_id];
				entity_map<unsigned int>& ship_map = map->ship_map[player_id];

				ship_vec.reserve(num_ships);
				for (unsigned int j = 0; j < num_ships; ++j) {
					const auto& ship_pair = parse_ship(iss, player_id, j, a_Map);
					ship_vec.push_back(ship_pair.second);
					ship_map[ship_pair.first] = j;
				}
			}

			unsigned int num_planets;
			iss >> num_planets;

			map->planets.reserve(num_planets);
			for (unsigned int i = 0; i < num_planets; ++i) {
				const auto& planet_pair = parse_planet(iss);
				map->planets.push_back(planet_pair.second);
				map->planet_map[planet_pair.first] = i;
			}

			map->maploaded();
			return map;
		}

		void setup(const std::string& bot_name, int map_width, int map_height);
		Map* get_map(hlt::PlayerId a_PlayerID, Map* a_Map, std::chrono::steady_clock::time_point* a_TimeStart);
	}
}
