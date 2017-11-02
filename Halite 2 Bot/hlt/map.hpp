#pragma once

#include "map.hpp"
#include "types.hpp"
#include "ship.hpp"
#include "planet.hpp"

namespace hlt {
	class Map {
	public:
		int map_width, map_height;

		std::unordered_map<PlayerId, std::vector<Ship>> ships;
		std::unordered_map<PlayerId, entity_map<unsigned int>> ship_map;

		std::vector<Planet> planets;
		entity_map<unsigned int> planet_map;

		Map(int width, int height);

		const Ship& get_ship(const PlayerId player_id, const EntityId ship_id) const {
			return ships.at(player_id).at(ship_map.at(player_id).at(ship_id));
		}

		const Planet& get_planet(const EntityId planet_id) const {
			return planets.at(planet_map.at(planet_id));
		}


		const std::vector<const hlt::Planet*> getPlanetsByDistance(const hlt::Location a_Position) const {
			std::vector<double> distances = std::vector<double>(planets.size());
			for (unsigned int i = 0; i < planets.size(); i++) {
				distances[i] = a_Position.get_distance_to(planets[i].location);
			}
			std::vector<const hlt::Planet*> planetsSorted = std::vector<const hlt::Planet*>(planets.size());
			for (unsigned int q = 0; q < planets.size(); q++) {
				int index = -1;
				double smallest = 9999;
				for (unsigned int i = 0; i < distances.size(); i++) {
					if (distances[i] < smallest) {
						smallest = distances[i];
						index = i;
					}
				}
				planetsSorted[q] = &planets[index];
				distances[index] = 9999;
			}
			return planetsSorted;
		}
	};
}
