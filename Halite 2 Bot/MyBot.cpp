#include "hlt/hlt.hpp"
#include "hlt/navigation.hpp"

#include <cmath>
#include <chrono>

#include "Vector2.h"

std::vector<hlt::Move> moves;

bool dockWithPlanet(const hlt::Map& a_Map, const hlt::Ship& a_Ship, hlt::Planet& a_Planet) {
	if (a_Ship.can_dock(a_Planet)) {
		a_Planet.m_UnitsGettingSentTo++;
		moves.push_back(hlt::Move::dock(a_Ship.entity_id, a_Planet.entity_id));
		return true;
	}

	const hlt::possibly<hlt::Move> move =
		hlt::navigation::navigate_ship_to_dock(a_Map, a_Ship, a_Planet, hlt::constants::MAX_SPEED);
	if (move.second) {
		moves.push_back(move.first);
		a_Planet.m_UnitsGettingSentTo++;
		return true;
	}
	return false;
}

bool attackShip(const hlt::Map& a_Map, const hlt::Ship& a_Ship, const hlt::Ship& a_Target) {
	hlt::Location loc = a_Target.location;
	loc.m_Pos.m_X += a_Ship.m_VelX / 2;
	loc.m_Pos.m_Y += a_Ship.m_VelY / 2;

	hlt::Location closestPos = a_Ship.location.get_closest_point(loc, hlt::constants::SHIP_RADIUS);

	const hlt::possibly<hlt::Move> move =
		hlt::navigation::navigate_ship_towards_target(a_Map, a_Ship, closestPos, hlt::constants::MAX_SPEED, true, hlt::constants::MAX_NAVIGATION_CORRECTIONS, M_PI / 125.0);
	if (move.second) {
		moves.push_back(move.first);
		return true;
	}
	return false;
}

bool attackClosestShip(const hlt::Map& a_Map, const hlt::Ship& a_Ship, std::vector<const hlt::Ship*>& a_EnemyShipsByDistance) {
	const hlt::Ship* targetShip = nullptr;
	unsigned int tsIndex = 0;
	while (targetShip == nullptr && tsIndex < a_EnemyShipsByDistance.size()) {
		targetShip = a_EnemyShipsByDistance[tsIndex++];
	}
	if (targetShip != nullptr) {
		hlt::Log::log(std::to_string(a_Ship.entity_id) + " ATTACKING CLOSEST SHIP");

		if (attackShip(a_Map, a_Ship, *targetShip)) {
			return true;
		} else {
			hlt::Log::log(std::to_string(a_Ship.entity_id) + " __ - Attacking - FAILED??");
		}
	}
	return false;
}

int main() {
	hlt::Metadata metadata = hlt::initialize("V7 Birb the killer robot!");
	const hlt::PlayerId player_id = metadata.player_id;

	hlt::Map& initial_map = metadata.initial_map;

	// We now have 1 full minute to analyse the initial map.
	std::ostringstream initial_map_intelligence;
	initial_map_intelligence
		<< "width: " << initial_map.map_width
		<< "; height: " << initial_map.map_height
		<< "; players: " << initial_map.ship_map.size()
		<< "; my ships: " << initial_map.ship_map.at(player_id).size()
		<< "; planets: " << initial_map.planets.size();
	hlt::Log::log(initial_map_intelligence.str());

	hlt::Ship ship = initial_map.ships.at(0).at(0);

	std::ostringstream planetsString;
	planetsString
		<< "Planets:\n";
	for (unsigned int i = 0; i < initial_map.planets.size(); i++) {
		hlt::Planet planet = initial_map.planets[i];
		planetsString << "ID: " << planet.entity_id <<
			"; C-production: " << planet.current_production <<
			"; R-production: " << planet.remaining_production <<
			"; radius: " << planet.radius <<
			"; distance: " << ship.location.get_distance_to(planet.location) <<
			"; calc: " << ship.location.get_distance_to(planet.location) / planet.radius <<
			"\n";
	}
	hlt::Log::log(planetsString.str());

	hlt::Map& map = initial_map;

	int turnCount = 0;
	for (;;) {
		std::chrono::steady_clock::time_point t1;
		turnCount++;

		moves.clear();
		map = hlt::in::get_map(player_id, &map, &t1);

		int useableShipCountindex = -1;
		for (const hlt::Ship& ship : map.ships.at(player_id)) {
			//hlt::Log::log(std::to_string(ship.entity_id) + " vel X " + std::to_string(ship.m_VelX) + " vel Y " + std::to_string(ship.m_VelY));

			std::vector<const hlt::Ship*> enemyShipsByDistance = map.getEnemyShipsByDistance(ship);

			if (ship.docking_status != hlt::ShipDockingStatus::Undocked) {
				hlt::Log::log(std::to_string(ship.entity_id) + ": IS DOCKED");

				//update the m_UnitsGettingSentTo to include docked units
				map.get_planet(ship.docked_planet).m_UnitsGettingSentTo++;
				continue;
				/*
				//This just seems detrimental to the bot
				//might have to rethink this, because it's helpful
				if (map.get_planet(ship.docked_planet).docked_ships.size() == 1) {
					continue;
				}

				const hlt::Ship* targetShip = enemyShipsByDistance[0];

				//note: we have already done the get distance too
				if (ship.location.get_distance_to(targetShip->location) <= 10) {
					moves.push_back(hlt::Move::undock(ship.entity_id));
					hlt::Log::log(std::to_string(ship.entity_id) + " Undocking - enemy too close");

					map.get_planet(ship.docked_planet).docked_ships.resize(map.get_planet(ship.docked_planet).docked_ships.size() - 1);
				}
				continue;
				*/
			}
			useableShipCountindex++;



			bool didMove = false;
			for (unsigned int i = 0; i < fmin(10u, enemyShipsByDistance.size()); i++) {
				if (enemyShipsByDistance[i] == nullptr) {
					hlt::Log::log(std::to_string(ship.entity_id) + " shipsByDistance[i] == nullptr ");
					continue;
				}
				const hlt::Ship* targetShip = enemyShipsByDistance[i];

				//move to firendly units if they are close, for assistance. currently broken
				/*
				std::vector<const hlt::Ship*> friendlyShipsByDistance = map.getFriendlyShipsByDistance(ship);
				if (friendlyShipsByDistance[0] != nullptr) {
					double dist = ship.location.get_distance_to(friendlyShipsByDistance[0]->location);
					if (dist >= hlt::constants::MAX_SPEED / 2 && dist <= hlt::constants::MAX_SPEED*3) {
						attackShip(map, ship, *friendlyShipsByDistance[0]);
						didMove = true;
						break;
					}
				}
				*/

				//note: we have already done the get distance to object
				if (ship.location.get_distance_to(targetShip->location) <= hlt::constants::MAX_SPEED*2.6) {
					hlt::Log::log(std::to_string(ship.entity_id) + " Enemy close - Attacking " + std::to_string(targetShip->entity_id));

					if (attackShip(map, ship, *targetShip)) {
						hlt::Log::log(std::to_string(ship.entity_id) + ": CLOSE SHIP ATTACK");

						didMove = true;
						break;
					} else {
						hlt::Log::log(std::to_string(ship.entity_id) + " Enemy close - Attacking - FAILED??");
					}
				} else {
					break;
				}
			}
			if (didMove) {
				continue;
			}

			if ((useableShipCountindex > ((int) map.m_NumberOfMyUndockedShips)*0.65) && (map.m_NumberOfMyUndockedShips != map.m_NumberOfMyShips || map.m_NumberOfMyPlanets == 0)) {
				if (attackClosestShip(map, ship, enemyShipsByDistance)) {
					hlt::Log::log(std::to_string(ship.entity_id) + ": INDEX ATTACK");

					continue;
				}
			}

			std::vector<hlt::Planet*> planetsByDistance = map.getPlanetsByDistance(ship);

			for (hlt::Planet* planet : planetsByDistance) {
				if (planet->owned) {

					if (planet->owner_id == player_id) {
						/*
						if (planet->m_UnitsGettingSentTo >= planet->m_RequiredUnits) {
							hlt::Log::log(std::to_string(ship.entity_id) + ": MOVE TO PLANET " + std::to_string(planet->entity_id)+" - NEEDS REINFORCEMENTS OR HAS DOCK SPOTS");

							dockWithPlanet(map, ship, *planet);
							break;
						}
						*/

						if (!planet->is_full()) {
							if (planet->m_UnitsGettingSentTo >= planet->m_RequiredUnits) {
								hlt::Log::log(std::to_string(ship.entity_id) + ": DOCK FULL MOVING ELSEWHERE " + std::to_string(planet->entity_id));
								continue;
							}

							if (dockWithPlanet(map, ship, *planet)) {
								hlt::Log::log(std::to_string(ship.entity_id) + ": DOCK FRIENDLY " + std::to_string(planet->entity_id));

								break;
							} else {
								hlt::Log::log(std::to_string(ship.entity_id) + ": Unable to dock will unfull planet " + std::to_string(planet->entity_id));
							}
						}
						continue;
					} else {
						if (planet->docked_ships.size() != 0) {
							if (attackShip(map, ship, map.get_ship(planet->owner_id, planet->docked_ships[0]))) {
								hlt::Log::log(std::to_string(ship.entity_id) + ": Planet Attack Docked");
								hlt::Log::log(std::to_string(ship.entity_id) + ": - Planet " + std::to_string(planet->entity_id));
								hlt::Log::log(std::to_string(ship.entity_id) + ": - Ship " + std::to_string(planet->docked_ships[0]));

							} else {
								continue;
							}
							//const hlt::possibly<hlt::Move> move =
							//hlt::navigation::navigate_ship_to_dock(map, ship, map.get_ship(planet->owner_id, planet->docked_ships[0]), hlt::constants::MAX_SPEED);
							//if (move.second) {
							//	moves.push_back(move.first);
							//} else {
							//	continue;
							//}
							break;
						}
					}
				}
				hlt::Log::log(std::to_string(ship.entity_id) + ": DOCK EMPTY " + std::to_string(planet->entity_id));
				dockWithPlanet(map, ship, *planet);
				break;
			}

			//if all else fails, attack the closest enemy ship
			if (moves.size() != 0 && moves[moves.size() - 1].ship_id != ship.entity_id) {
				if (attackClosestShip(map, ship, enemyShipsByDistance)) {

				} else {
					hlt::Log::log(std::to_string(ship.entity_id) + ": HAD NO MOVES!");

				}
			}

		}
		//hlt::Log::log("math check: " + std::to_string(useableShipCountindex) + " - " + std::to_string(((int) map.m_NumberOfMyUndockedShips - 1)*0.8));
		/*
		hlt::Log::log("moves: " + std::to_string(moves.size()) + " ships " + std::to_string(map.m_NumberOfMyUndockedShips));
		for (int i = 0; i < moves.size(); i++) {
			//hlt::Ship thisShip= map.get_ship(player_id,moves[i].ship_id);
			hlt::Move& move = moves[i];
			std::ostringstream string;
			string
				<< "MOVES- Ship:\n";
			string << "ID: " << move.ship_id;
				switch (move.type) {
					case hlt::MoveType::Dock:
						string << "Move Type: Dock ";
						break;
					case hlt::MoveType::Noop:
						string << "Move Type: Noop ";
						break;
					case hlt::MoveType::Undock:
						string << "Move Type: Undock ";
						break;
					case hlt::MoveType::Thrust:
						string << "Move Type: Thrust ";
						break;
				}

			hlt::Log::log(string.str());
		}
		*/


		if (!hlt::out::send_moves(moves)) {
			hlt::Log::log("send_moves failed; exiting");
			break;
		}

		//timer info
		auto t2 = std::chrono::high_resolution_clock::now();
		auto diff = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
		hlt::Log::log("Turn Time: " + std::to_string(1000 * diff.count()));
	}
}
