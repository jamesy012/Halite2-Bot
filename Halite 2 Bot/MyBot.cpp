#include "hlt/hlt.hpp"
#include "hlt/navigation.hpp"

#include <cmath>
#include <chrono>

#include "Vector2.h"

//#define LOGGING

#ifdef LOGGING
#define ezLog(ship,x) hlt::Log::log(std::to_string(ship.entity_id) + x);
#else
#define ezLog(ship,x) 
#endif // LOGGING


std::vector<hlt::Move> moves;

bool dockWithPlanet(hlt::Map* a_Map, hlt::Ship& a_Ship, hlt::Planet& a_Planet) {
	if (a_Ship.can_dock(a_Planet)) {
		a_Planet.m_UnitsGettingSentTo++;
		moves.push_back(hlt::Move::dock(a_Ship.entity_id, a_Planet.entity_id));
		return true;
	}

	const hlt::possibly<hlt::Move> move =
		hlt::navigation::navigate_ship_to_dock(*a_Map, a_Ship, a_Planet, hlt::constants::MAX_SPEED);
	if (move.second) {
		moves.push_back(move.first);
		a_Planet.m_UnitsGettingSentTo++;
		return true;
	}
	return false;
}

bool attackShip(hlt::Map* a_Map, hlt::Ship& a_Ship, const hlt::Ship& a_Target) {
	hlt::Location loc = a_Target.location;
	loc.m_Pos.m_X += a_Ship.m_VelX / 2;
	loc.m_Pos.m_Y += a_Ship.m_VelY / 2;

	hlt::Location closestPos = a_Ship.location.get_closest_point(loc, hlt::constants::SHIP_RADIUS);

	const hlt::possibly<hlt::Move> move =
		hlt::navigation::navigate_ship_towards_target(*a_Map, a_Ship, closestPos, hlt::constants::MAX_SPEED, true, hlt::constants::MAX_NAVIGATION_CORRECTIONS, M_PI / 125.0);
	if (move.second) {
		moves.push_back(move.first);
		return true;
	}
	return false;
}

bool attackClosestShip(hlt::Map* a_Map, hlt::Ship& a_Ship, std::vector<hlt::Entity::DistEntity>* a_EnemyShipsByDistance) {
	const hlt::Ship* targetShip = nullptr;
	unsigned int tsIndex = 0;
	while (targetShip == nullptr && tsIndex < a_EnemyShipsByDistance->size()) {
		targetShip = (hlt::Ship*)a_EnemyShipsByDistance->at(tsIndex++).m_Entity;
	}
	if (targetShip != nullptr) {
		ezLog(a_Ship," ATTACKING CLOSEST SHIP");

		if (attackShip(a_Map, a_Ship, *targetShip)) {
			return true;
		} else {
			ezLog(a_Ship, " __ - Attacking - FAILED??");
		}
	}
	return false;
}

int main() {
	hlt::Metadata metadata = hlt::initialize("V7 Birb the killer robot!");
	const hlt::PlayerId player_id = metadata.player_id;

	hlt::Map& initial_map = *metadata.initial_map;

	// We now have 1 full minute to analyse the initial map->
	std::ostringstream initial_map_intelligence;
	initial_map_intelligence
		<< "width: " << initial_map.map_width
		<< "; height: " << initial_map.map_height
		<< "; players: " << initial_map.ship_map.size()
		<< "; my ships: " << initial_map.ship_map.at(player_id).size()
		<< "; planets: " << initial_map.planets.size();
	hlt::Log::log(initial_map_intelligence.str());

	hlt::Map* map = &initial_map;

	//turn counter
	int turnCount = 0;
	//time at the start of the turn, set in get_map after we have received input
	std::chrono::steady_clock::time_point t1;

	for (;;) {
		//update turn
		turnCount++;

		//clear moves and get new map
		moves.clear();
		map = hlt::in::get_map(player_id, map, &t1);
		map->m_TurnNum = turnCount;

		//turn info
		{
			std::ostringstream turnStats;
			turnStats << "Turn Stats:" <<
				"\nMy Ships: " << map->m_NumberOfFriendlyShips <<
				"\n\tUndocked: " << map->m_NumberOfFriendlyUndockedShips <<
				"\nEnemy Ships: " << map->m_NumberOfEnemyShips
				;

			hlt::Log::log(turnStats.str());
		}

		//counter that counts up for every ship thats not docked/docking
		int useableShipCountindex = -1;
		for (hlt::Ship& ship : map->ships.at(player_id)) {
			//hlt::Log::log(std::to_string(ship.entity_id) + " vel X " + std::to_string(ship.m_VelX) + " vel Y " + std::to_string(ship.m_VelY));




			if (ship.docking_status != hlt::ShipDockingStatus::Undocked) {
				ezLog(ship, ": IS DOCKED TO " + std::to_string(ship.docked_planet));

				//update the m_UnitsGettingSentTo to include docked units
				map->get_planet(ship.docked_planet).m_UnitsGettingSentTo++;
				continue;
				/*
				//This just seems detrimental to the bot
				//might have to rethink this, because it's helpful
				if (map->get_planet(ship.docked_planet).docked_ships.size() == 1) {
					continue;
				}

				const hlt::Ship* targetShip = enemyShipsByDistance[0];

				//note: we have already done the get distance too
				if (ship.location.get_distance_to(targetShip->location) <= 10) {
					moves.push_back(hlt::Move::undock(ship.entity_id));
					hlt::Log::log(std::to_string(ship.entity_id) + " Undocking - enemy too close");

					map->get_planet(ship.docked_planet).docked_ships.resize(map->get_planet(ship.docked_planet).docked_ships.size() - 1);
				}
				continue;
				*/
			}

			std::vector<hlt::Entity::DistEntity>* enemyShipsByDistance = &ship.m_EnemyShipsByDistance;//map->getEnemyShipsByDistance(ship);
			//for (unsigned int i = 0; i < enemyShipsByDistance->size(); i++) {
			//	hlt::Log::log(std::to_string(ship.entity_id) + " dist with " + std::to_string(enemyShipsByDistance->at(i).m_Entity->entity_id) + ": " + std::to_string(enemyShipsByDistance->at(i).m_Distance));
			//}
			useableShipCountindex++;

			bool didMove = false;
			for (unsigned int i = 0; i < fmin(10u, enemyShipsByDistance->size()); i++) {
				if (enemyShipsByDistance->at(i).m_Entity == nullptr) {
					
					ezLog(ship, " shipsByDistance[i] == nullptr ");
					continue;
				}
				const hlt::Entity::DistEntity target = enemyShipsByDistance->at(i);

				//move to firendly units if they are close, for assistance. currently broken
				/*
				std::vector<const hlt::Ship*> friendlyShipsByDistance = map->getFriendlyShipsByDistance(ship);
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
				if (target.m_Distance <= hlt::constants::MAX_SPEED*3) {
					ezLog(ship, " Enemy close - Attacking " + std::to_string(target.m_Entity->entity_id));

					if (attackShip(map, ship, (hlt::Ship&)*target.m_Entity)) {
						ezLog(ship, ": CLOSE SHIP ATTACK");

						didMove = true;
						break;
					} else {
						ezLog(ship, " Enemy close - Attacking - FAILED??");
					}
				} else {
					break;
				}
			}
			if (didMove) {
				continue;
			}
			//hlt::Log::log(std::to_string(ship.entity_id) + ": IA Value: " + std::to_string((int) (map->m_NumberOfFriendlyUndockedShips*0.25 - useableShipCountindex)) + " With wsci at " + std::to_string(useableShipCountindex));

			if (0 > (int) (map->m_NumberOfFriendlyUndockedShips*0.65 -useableShipCountindex) && (map->m_NumberOfFriendlyUndockedShips != map->m_NumberOfFriendlyShips || map->m_NumberOfFriendlyPlanets != 0)) {
				if (attackClosestShip(map, ship, enemyShipsByDistance)) {
					ezLog(ship, ": INDEX ATTACK");

					continue;
				}
			}

			std::vector<hlt::Entity::DistEntity>* planetsByDistance = &ship.m_PlanetsByDistance;//map->getPlanetsByDistance(ship);

			for (hlt::Entity::DistEntity distEntity : *planetsByDistance) {
				hlt::Planet* planet = (hlt::Planet*)distEntity.m_Entity;
				if (planet->owned) {

					if (planet->owner_id == player_id) {//owned by us
						/*
						if (planet->m_UnitsGettingSentTo >= planet->m_RequiredUnits) {
							hlt::Log::log(std::to_string(ship.entity_id) + ": MOVE TO PLANET " + std::to_string(planet->entity_id)+" - NEEDS REINFORCEMENTS OR HAS DOCK SPOTS");

							dockWithPlanet(map, ship, *planet);
							break;
						}
						*/

						if (!planet->is_full()) {
							if (planet->m_UnitsGettingSentTo >= planet->m_RequiredUnits) {
								ezLog(ship, ": DOCK GOING FULL MOVING ELSEWHERE (FRIENDLY) " + std::to_string(planet->entity_id));
								continue;
							}

							if (dockWithPlanet(map, ship, *planet)) {
								ezLog(ship, ": DOCK FRIENDLY " + std::to_string(planet->entity_id));

								break;
							} else {
								ezLog(ship, ": Unable to dock will unfull planet " + std::to_string(planet->entity_id));
							}
						}
						continue;
					} else {//owned by enemy
						if (planet->m_EnemyShipsByDistance[0].m_Distance > planet->radius *1.5 && false) {
							ezLog(ship, ": WANTS TO DOCK WITH " + std::to_string(planet->entity_id) + " ATTACKING CLOSE BY INSTEAD: " + std::to_string(planet->m_EnemyShipsByDistance[0].m_Entity->entity_id));
							attackShip(map, ship, (hlt::Ship&)planet->m_EnemyShipsByDistance[0].m_Entity);
							break;
						} else {
							if (planet->docked_ships.size() != 0) {
								if (attackShip(map, ship, map->get_ship(planet->owner_id, planet->docked_ships[0]))) {
									ezLog(ship, ": Planet Attack Docked");
									ezLog(ship, ": - Planet " + std::to_string(planet->entity_id));
									ezLog(ship, ": - Ship " + std::to_string(planet->docked_ships[0]));

								} else {
									continue;
								}
								//const hlt::possibly<hlt::Move> move =
								//hlt::navigation::navigate_ship_to_dock(map, ship, map->get_ship(planet->owner_id, planet->docked_ships[0]), hlt::constants::MAX_SPEED);
								//if (move.second) {
								//	moves.push_back(move.first);
								//} else {
								//	continue;
								//}
								break;
							}
						}
					}
				}//not owned by anyone

				//todo: this needs a list of enemy ships by distance relative to the planet, not this ship
				//if there is a ship close to this planet?
				/*
				if (ship.location.get_distance_to(enemyShipsByDistance[0]->location) <= planet->radius * 2.5) {
					hlt::Log::log(std::to_string(ship.entity_id) + ": WANTS TO DOCK WITH " + std::to_string(planet->entity_id) +
								  " BUT " + std::to_string(enemyShipsByDistance[0]->entity_id) + " IS NEAR BY");

					attackShip(map, ship, *enemyShipsByDistance[0]);
					break;

				}//else dock normally
				*/

				if (planet->m_UnitsGettingSentTo >= planet->m_RequiredUnits) {
					ezLog(ship, ": DOCK GOING FULL MOVING ELSEWHERE (UNOWNED) " + std::to_string(planet->entity_id));
					continue;
				}

				ezLog(ship, ": DOCK EMPTY " + std::to_string(planet->entity_id));
				dockWithPlanet(map, ship, *planet);

				break;
			}

			//if all else fails, attack the closest enemy ship
			if (moves.size() != 0 && moves[moves.size() - 1].ship_id != ship.entity_id) {
				if (attackClosestShip(map, ship, enemyShipsByDistance)) {

				} else {
					ezLog(ship, ": HAD NO MOVES!");

				}
			}

		}
		//hlt::Log::log("math check: " + std::to_string(useableShipCountindex) + " - " + std::to_string(((int) map->m_NumberOfFriendlyUndockedShips - 1)*0.8));
		/*
		hlt::Log::log("moves: " + std::to_string(moves.size()) + " ships " + std::to_string(map->m_NumberOfFriendlyUndockedShips));
		for (int i = 0; i < moves.size(); i++) {
			//hlt::Ship thisShip= map->get_ship(player_id,moves[i].ship_id);
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
