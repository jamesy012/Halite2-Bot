#include "map.hpp"
#include "log.hpp"
#include "hlt.hpp"

#include <chrono>
#include <algorithm>

namespace hlt {
	bool distEntitySort(Entity::DistEntity& a_Lhs, Entity::DistEntity& a_Rhs) { return (a_Lhs.m_Distance<a_Rhs.m_Distance); }

	Map::Map(const int width, const int height, hlt::PlayerId a_PlayerID) : map_width(width), map_height(height), m_PlayerID(a_PlayerID) {

	}



	void Map::maploaded() {
		//calc amount of ships
		for (unsigned int i = 0; i < ship_map.size(); i++) {
			if (i == m_PlayerID) {
				m_NumberOfFriendlyShips = ship_map.at(i).size();

				for (unsigned int q = 0; q < ship_map.at(i).size(); q++) {
					if (ships.at(i).at(q).docking_status == hlt::ShipDockingStatus::Undocked) {
						m_NumberOfFriendlyUndockedShips++;
					}
				}

			} else {
				m_NumberOfEnemyShips += ship_map.at(i).size();
			}
			m_NumberOfShips += ship_map.at(i).size();
		}

		//calc number of planets
		for (unsigned int i = 0; i < planets.size(); i++) {
			if (planets[i].owner_id == m_PlayerID) {
				m_NumberOfFriendlyPlanets++;
			} else {
				m_NumberOfEnemyPlanets++;
			}
			m_NumberOfPlanets++;
			planets[i].m_RequiredUnits = planets[i].docking_spots;
			//todo:: move elsewhere, since now we cant get ships by distance right now
			/*
			auto enemysNearBy = getEnemyShipsByDistance(planets[i]);
			for (unsigned int q = 0; q < enemysNearBy.size(); q++) {
				if (enemysNearBy[q] == nullptr) {
					continue;
				}
				
				//double dist = planets[i].location.get_distance_to(enemysNearBy[q]->location);
				//if (dist <= planets[i].radius + 10) {
				//	planets[i].m_EnemiesNearBy++;
				//}
				
			}
			planets[i].m_RequiredUnits = planets[i].docking_spots + (int) ceil(planets[i].m_EnemiesNearBy * 1.5);
			//Log::log(std::to_string(i) + " has required of " + std::to_string(planets[i].m_RequiredUnits) + " - " + std::to_string(planets[i].docking_spots) + " - " + std::to_string(planets[i].docked_ships.size()) + " - " + std::to_string(planets[i].m_EnemiesNearBy));
			*/
		}

		auto t1 = std::chrono::high_resolution_clock::now();
		
		//Set up DistEntity data for all planets
		for (unsigned int i = 0; i < planets.size(); i++) {
			hlt::Planet* planet = &planets[i];
		
			//setUpEntityVectorCapacitys(planet);
		
			//no need to do distance between planets, this wont change..
			//addDistEntityPlanets(planet);
		
			addDistEntityShips(planet,true);
		}
		


		//Set up DistEntity data for all ships
		for (unsigned int i = 0; i < ship_map.size(); i++) {
			//do ship DistEntity data
			//go through all ships for this player
			for (unsigned int q = 0; q < ship_map.at(i).size(); q++) {
				hlt::Ship* ship = &ships.at(i).at(q);
				//skip docked
				if (ship->docking_status != ShipDockingStatus::Undocked) {
					continue;
				}

				//setUpEntityVectorCapacitys(ship);

				//planets
				addDistEntityPlanets(ship);

				//ships
				addDistEntityShips(ship);
			}
		}
		auto t2 = std::chrono::high_resolution_clock::now();
		auto diff = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
		hlt::Log::log("Ships Time: " + std::to_string(1000 * diff.count()));

	}

	Ship & Map::get_ship(PlayerId player_id, EntityId ship_id) {
		return ships.at(player_id).at(ship_map.at(player_id).at(ship_id));
	}

	Planet& Map::get_planet(EntityId planet_id) {
		return planets.at(planet_map.at(planet_id));
	}

	void Map::setUpEntityVectorCapacitys(hlt::Entity* a_Entity) const {
		a_Entity->m_PlanetsByDistance.reserve(m_NumberOfPlanets);
		a_Entity->m_FriendlyPlanetsByDistance.reserve(m_NumberOfFriendlyPlanets);
		a_Entity->m_EnemyPlanetsByDistance.reserve(m_NumberOfEnemyPlanets);

		a_Entity->m_ShipsByDistance.reserve(m_NumberOfShips);
		a_Entity->m_FriendlyShipsByDistance.reserve(m_NumberOfFriendlyShips);
		a_Entity->m_EnemyShipsByDistance.reserve(m_NumberOfEnemyShips);
	}


	//TODO: these two functions below can be quicker, add and sort the full ship/planets list, then take from that in order of 0 to number of ships/planets
	//means we only sort 1 list, then populate the other 2 from that

	void Map::addDistEntityShips(hlt::Entity* a_Entity, bool a_SkipDocked) {

		std::vector<Entity::DistEntity> shipDeList = std::vector<Entity::DistEntity>(m_NumberOfShips);
		int shipIndex = 0;
		for (unsigned int w = 0; w < ship_map.size(); w++) {

			std::vector<Entity::DistEntity> thisPlayersShipEnity = std::vector<Entity::DistEntity>(ship_map.at(w).size());
			for (unsigned int j = 0; j < thisPlayersShipEnity.size(); j++) {
				hlt::Ship* otherShip = &ships.at(w).at(j);
				Entity::DistEntity de;

				if (otherShip == a_Entity) {
					de.m_Entity = otherShip;
					de.m_Distance = constants::DISTANCE_FOR_SAME_OBJECT;
				} else if (a_SkipDocked && otherShip->docking_status != ShipDockingStatus::Undocked) {
					de.m_Entity = otherShip;
					de.m_Distance = constants::DISTANCE_FOR_DOCKED_OBJECT;
				} else {
					de = a_Entity->getDistEntity(otherShip);
				}

				shipDeList[shipIndex] = de;
				shipIndex++;
			}
		}

		std::sort(shipDeList.begin(), shipDeList.end(), distEntitySort);
		//a_Entity->m_ShipsByDistance.insert(a_Entity->m_ShipsByDistance.end(), sortedList.begin(), sortedList.end());

		a_Entity->m_FriendlyShipsByDistance.resize(m_NumberOfFriendlyShips);
		a_Entity->m_EnemyShipsByDistance.resize(m_NumberOfEnemyShips);
		a_Entity->m_ShipsByDistance.resize(m_NumberOfShips);
		int enemyIndex = 0, friendlyIndex = 0;
		for (unsigned int i = 0; i < shipDeList.size(); i++) {
			a_Entity->m_ShipsByDistance[i] = shipDeList[i];
			if (shipDeList[i].m_Entity->owner_id == m_PlayerID) {
				a_Entity->m_FriendlyShipsByDistance[friendlyIndex++] = shipDeList[i];
			} else {
				a_Entity->m_EnemyShipsByDistance[enemyIndex++] = shipDeList[i];

			}
		}
	}
	void Map::addDistEntityPlanets(hlt::Entity * a_Entity) {

		std::vector<Entity::DistEntity> planetDeList = std::vector<Entity::DistEntity>(m_NumberOfPlanets);

		for (unsigned int q = 0; q < planets.size(); q++) {
			hlt::Planet* otherPlanet = &planets[q];
			Entity::DistEntity de;
				de.m_Entity = otherPlanet;
			if (otherPlanet == a_Entity) {
				de.m_Distance = constants::DISTANCE_FOR_SAME_OBJECT;
			} else {
				de.m_Distance = a_Entity->location.get_distance_to(otherPlanet->location) / otherPlanet->radius;
			}
			planetDeList[q] = de;
		}

		std::sort(planetDeList.begin(), planetDeList.end(), distEntitySort);

		a_Entity->m_FriendlyPlanetsByDistance.resize(m_NumberOfFriendlyPlanets);
		a_Entity->m_EnemyPlanetsByDistance.resize(m_NumberOfEnemyPlanets);
		a_Entity->m_PlanetsByDistance.resize(m_NumberOfPlanets);
		int enemyIndex = 0, friendlyIndex = 0;
		for (unsigned int i = 0; i < planetDeList.size(); i++) {
			a_Entity->m_PlanetsByDistance[i] = planetDeList[i];
			if (planetDeList[i].m_Entity->owner_id == m_PlayerID) {
				a_Entity->m_FriendlyPlanetsByDistance[friendlyIndex++] = planetDeList[i];
			} else {
				a_Entity->m_EnemyPlanetsByDistance[enemyIndex++] = planetDeList[i];

			}
		}
	}
}
