

#include "hlt/hlt.hpp"
#include "hlt/navigation.hpp"

#include <cmath>
#include <chrono>
#include <time.h>

#include "Vector2.h"
#include "Defines.h"

#include "ShipBehaviours.h"

#ifndef USEASTAR
#include "AStar.h"
#endif



std::vector<hlt::Move> moves;
#ifdef USEASTAR
AStar* m_AStar;
#endif

#ifdef USE_OLD_SYSTEM
//moves to a_Planet and docks with a_Planet when the ship is close enough
bool dockWithPlanet(BehaviourFuncPrams, hlt::Planet& a_Planet) {
	//can we dock with a_Planet?
	if (a_Ship->can_dock(a_Planet)) {
		a_Planet.m_UnitsGettingSentTo++;
		moves.push_back(hlt::Move::dock(a_Ship->entity_id, a_Planet.entity_id));
		return true;
	}

	const hlt::possibly<hlt::Move> move =
#ifdef USEASTAR
		hlt::navigation::aStarDock(m_AStar, &a_Ship, &a_Planet);
#else
		hlt::navigation::navigate_ship_to_dock(*a_Map, *a_Ship, a_Planet, hlt::constants::MAX_SPEED);
#endif
	if (move.second) {
		moves.push_back(move.first);
		a_Planet.m_UnitsGettingSentTo++;
		return true;
	}
	return false;
}

bool attackShip(BehaviourFuncPrams, const hlt::Ship& a_Target) {
	hlt::Location loc = a_Target.location;
	loc.m_Pos.m_X += std::min(hlt::constants::WEAPON_RADIUS, a_Ship->m_VelX) / 2;
	loc.m_Pos.m_Y += std::min(hlt::constants::WEAPON_RADIUS, a_Ship->m_VelY) / 2;

	hlt::Location closestPos = a_Ship->location.get_closest_point_if_in_range(loc, hlt::constants::WEAPON_RADIUS / 2, hlt::constants::SHIP_RADIUS * 2);

	const hlt::possibly<hlt::Move> move =
#ifdef USEASTAR
		hlt::navigation::aStarNavigate(m_AStar, &a_Ship, &closestPos);
#else
		hlt::navigation::navigate_ship_towards_target(*a_Map, *a_Ship, closestPos, hlt::constants::MAX_SPEED, true, hlt::constants::MAX_NAVIGATION_CORRECTIONS, 0.5f * (M_PI / 180.0f));
#endif
	if (move.second) {
		moves.push_back(move.first);
		return true;
	}
	return false;
}

bool attackClosestShip(BehaviourFuncPrams, std::vector<hlt::Entity::DistEntity>* a_EnemyShipsByDistance) {
	const hlt::Ship* targetShip = nullptr;
	unsigned int tsIndex = 0;
	while (targetShip == nullptr && tsIndex < a_EnemyShipsByDistance->size()) {
		targetShip = (hlt::Ship*)a_EnemyShipsByDistance->at(tsIndex++).m_Entity;
	}
	if (targetShip != nullptr) {
		ezLog(a_Ship, " ATTACKING CLOSEST SHIP");

		if (attackShip(a_Map, a_Ship, *targetShip)) {
			return true;
		} else {
			ezLog(a_Ship, " __ - Attacking - FAILED??");
		}
	}
	return false;
}

//docking behaviour
//returns true if ship is docked or doing anything docking related
bool runDocked(BehaviourFuncPrams) {
	//if ship is anything other then undocked. we consider it docked
	if (a_Ship->docking_status != hlt::ShipDockingStatus::Undocked) {
		ezLog(a_Ship, ": IS DOCKED TO " + std::to_string(a_Ship->docked_planet));

		//update the m_UnitsGettingSentTo to include docked units
		//a_Map->get_planet(a_Ship->docked_planet).m_UnitsGettingSentTo++;
		return true;
	}
	return false;
}

bool runAttackNearByEneimes(BehaviourFuncPrams, double a_Distance) {
	std::vector<hlt::Entity::DistEntity>* enemyShipsByDistance = &a_Ship->m_EnemyShipsByDistance;

	bool didMove = false;
	for (unsigned int i = 0; i < fmin(10u, enemyShipsByDistance->size()); i++) {
		if (enemyShipsByDistance->at(i).m_Entity == nullptr) {

			ezLog(a_Ship, " shipsByDistance[i] == nullptr ");
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
		if (target.m_Distance <= a_Distance) {
			ezLog(a_Ship, " Enemy close - Attacking " + std::to_string(target.m_Entity->entity_id));

			if (attackShip(a_Map, a_Ship, (hlt::Ship&)*target.m_Entity)) {
				ezLog(a_Ship, ": CLOSE SHIP ATTACK");

				didMove = true;
				break;
			} else {
				ezLog(a_Ship, " Enemy close - Attacking - FAILED??");
			}
		} else {
			break;
		}
	}
	//true if we did make a move here
	return didMove;
}

bool runIndexAttack(BehaviourFuncPrams, int a_UseableShipIndexCount) {

	if (0 > (int) (a_Map->m_NumberOfFriendlyUndockedShips*0.65 - a_UseableShipIndexCount) && (a_Map->m_NumberOfFriendlyUndockedShips != a_Map->m_NumberOfFriendlyShips || a_Map->m_NumberOfFriendlyPlanets != 0)) {
		if (attackClosestShip(a_Map, a_Ship, &a_Ship->m_EnemyShipsByDistance)) {
			ezLog(a_Ship, ": INDEX ATTACK");

			return true;
		}
	}
	return false;
}

bool runPlanetAttack(BehaviourFuncPrams) {
	std::vector<hlt::Entity::DistEntity>* planetsByDistance = &a_Ship->m_PlanetsByDistance;//map->getPlanetsByDistance(ship);

	for (hlt::Entity::DistEntity distEntity : *planetsByDistance) {
		hlt::Planet* planet = (hlt::Planet*)distEntity.m_Entity;
		if (planet->owned) {

			if (planet->owner_id == a_Map->m_PlayerID) {//owned by us
												/*
												if (planet->m_UnitsGettingSentTo >= planet->m_RequiredUnits) {
												hlt::Log::log(std::to_string(ship.entity_id) + ": MOVE TO PLANET " + std::to_string(planet->entity_id)+" - NEEDS REINFORCEMENTS OR HAS DOCK SPOTS");

												dockWithPlanet(map, ship, *planet);
												break;
												}
												*/
				if (planet->m_EnemyShipsByDistance[0].m_Distance <= planet->radius * 2) {
					ezLog(a_Ship, ": ENEMY SHIP CLOSE TO FRIENDLY PLANET " + std::to_string(planet->entity_id) + " ATTACKING " + std::to_string(planet->m_EnemyShipsByDistance[0].m_Entity->entity_id));
					if (attackShip(a_Map, a_Ship, (hlt::Ship&)*planet->m_EnemyShipsByDistance[0].m_Entity)) {
						return true;
					}
				}

				if (!planet->is_full()) {
					if (planet->m_UnitsGettingSentTo >= planet->m_RequiredUnits) {
						ezLog(a_Ship, ": DOCK GOING FULL MOVING ELSEWHERE (FRIENDLY) " + std::to_string(planet->entity_id));
						continue;
					}

					if (dockWithPlanet(a_Map, a_Ship, *planet)) {
						ezLog(a_Ship, ": DOCK FRIENDLY " + std::to_string(planet->entity_id));
						return true;
						break;
					} else {
						ezLog(a_Ship, ": Unable to dock will unfull planet " + std::to_string(planet->entity_id));
					}
				}
				continue;
			} else {//owned by enemy
					//if (planet->m_EnemyShipsByDistance[0].m_TiledDistance > planet->radius *1.5 && false) {
					//	ezLog(ship, ": WANTS TO DOCK WITH " + std::to_string(planet->entity_id) + " ATTACKING CLOSE BY INSTEAD: " + std::to_string(planet->m_EnemyShipsByDistance[0].m_Entity->entity_id));
					//	attackShip(map, ship, (hlt::Ship&)planet->m_EnemyShipsByDistance[0].m_Entity);
					//	break;
					//} else {
				if (planet->docked_ships.size() != 0) {
					if (attackShip(a_Map, a_Ship, a_Map->get_ship(planet->owner_id, planet->docked_ships[0]))) {
						ezLog(a_Ship, ": Planet Attack Docked, Planet " + std::to_string(planet->entity_id) + ", Ship " + std::to_string(planet->docked_ships[0]));
						return true;
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
				//}
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
		if (planet->m_EnemyShipsByDistance[0].m_Distance <= planet->radius * 2) {
			ezLog(a_Ship, ": ENEMY SHIP CLOSE TO FRIENDLY PLANET " + std::to_string(planet->entity_id) + " ATTACKING " + std::to_string(planet->m_EnemyShipsByDistance[0].m_Entity->entity_id));
			if (attackShip(a_Map, a_Ship, (hlt::Ship&)*planet->m_EnemyShipsByDistance[0].m_Entity)) {
				return true;
			}
		}

		if (planet->m_UnitsGettingSentTo >= planet->m_RequiredUnits) {
			ezLog(a_Ship, ": DOCK GOING FULL MOVING ELSEWHERE (UNOWNED) " + std::to_string(planet->entity_id));
			continue;
		}

		ezLog(a_Ship, ": DOCK EMPTY " + std::to_string(planet->entity_id));
		dockWithPlanet(a_Map, a_Ship, *planet);
		return true;
		break;
	}
	return false;
}

bool runAttackIfNoMoves(BehaviourFuncPrams) {
	std::vector<hlt::Entity::DistEntity>* enemyShipsByDistance = &a_Ship->m_EnemyShipsByDistance;
	//if all else fails, attack the closest enemy ship
	if (moves.size() != 0 && moves[moves.size() - 1].ship_id != a_Ship->entity_id) {
		ezLog(a_Ship, ": DID NOT MAKE A MOVE, ATTACKING CLOSEST ENEMY");
		if (attackClosestShip(a_Map, a_Ship, enemyShipsByDistance)) {
			return true;
		}
	}
	return false;
}
#endif
void debugLogMoves(hlt::Map* a_Map) {
	hlt::Log::log("moves: " + std::to_string(moves.size()) + " ships " + std::to_string(a_Map->m_NumberOfFriendlyUndockedShips));
	for (unsigned int i = 0; i < moves.size(); i++) {
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
}

int main() {


	hlt::Metadata metadata = hlt::initialize("V10 Birb the killer robot!");
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
	hlt::Map* oldMap = nullptr;

	//turn counter
	int turnCount = 0;
	//time at the start of the turn, set in get_map after we have received input
	std::chrono::steady_clock::time_point t1;

	srand(time(NULL) + player_id);
	hlt::Location pos[3];
	for (int i = 0; i < map->ships.at(player_id).size(); i++) {
		pos[i] = map->ships.at(player_id).at(i).location;
	}
	//hlt::Location pos = hlt::Location{ Vector2{46.0,147.0} };

	ShipBehaviours sb;
	sb.m_Singleton = &sb;

	for (;;) {
		//update turn
		turnCount++;

		//clear moves and get new map
		if (oldMap != nullptr) {
			delete oldMap;
		}
		oldMap = map;
		moves.clear();
		map = hlt::in::get_map(player_id, oldMap, &t1);
		map->m_TurnNum = turnCount;

		sb.m_CurrentMap = map;

#ifdef USEASTAR
		AStar aStar;
		aStar.genMap(map, &map->m_ListOfShips, &map->planets);
		m_AStar = &aStar;
		//_ASSERT(false);
		//std::vector<AStar::Node*> path = aStar.pathToNode(aStar.getNodeFromPosition(map->map_width / 2, map->map_height / 2), aStar.getNodeFromPosition(map->ships.at(player_id).at(0).location.m_Pos));
		//for (int i = 0; i < path.size(); i++) {
		//	path[i]->m_CanTraverse = false;
		//}
		//aStar.logToLogs();
#endif

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
		//for (int i = 0; i < map->ships.at(player_id).size(); i++) {
		//	if (map->ships.at(player_id).at(i).location.get_distance_to(pos[i]) < 1 || !aStar.getNodeFromPosition(pos[i].m_Pos)->m_CanTraverse) {
		//		pos[i].m_Pos.m_X = rand() % map->map_width;
		//		pos[i].m_Pos.m_Y = rand() % map->map_height;
		//	}
		//	const hlt::possibly<hlt::Move> move =
		//		hlt::navigation::aStarNavigate(&aStar, &map->ships.at(player_id).at(i), &pos[i]);
		//	if (move.second) {
		//		moves.push_back(move.first);
		//	}
		//}

#ifdef USE_OLD_SYSTEM


		//counter that counts up for every ship thats not docked/docking
		int useableShipCountindex = -1;

		//if docked, stay docked. end turn
		//if there is an enemy near by, attack it. end turn
		//if we have enough ships then this is an attack ship, attack the closest ship. end turn
		//finally look for closest planet, (found one? dock with it. end turn). if all planets taken, attack closest enemy planet. end turn
		//if we didn't get a move, then attack the closest enemy

		//go through all my ships
		for (hlt::Ship& shipRef : map->ships.at(player_id)) {
			hlt::Ship* ship = &shipRef;
			
			runBehaviour(runDocked(BehaviourPrams));
			
			useableShipCountindex++;
			
			runBehaviour(runAttackNearByEneimes(BehaviourPrams, hlt::constants::WEAPON_RADIUS * 3.0));
			
			runBehaviour(runIndexAttack(BehaviourPrams, useableShipCountindex));
			
			runBehaviour(runPlanetAttack(BehaviourPrams));
			
			runBehaviour(runAttackIfNoMoves(BehaviourPrams));

			ezLog(ship, ": HAD NO MOVES!");
		}
#else
		//ship attack loop
		for (hlt::Ship* ship : map->m_ListOfMyUndockedShips) {

			//look for near by enemy ships

		}

		//planet docking/attacking loop
		for (hlt::Ship* ship : map->m_ListOfMyUndockedShips) {
			if (ship->m_MoveTarget != nullptr) {
				continue;
			}

			//go through all planets
			for (hlt::Entity::DistEntity& dePlanets : ship->m_PlanetsByDistance) {
				hlt::Planet* planet = (hlt::Planet*)dePlanets.m_Entity;

				//if we own this planet
				if (planet->owner_id == map->m_PlayerID) {

					if (planet->m_UnitsGettingSentTo < planet->m_RequiredUnits) {
						if (sb.dockWithPlanet(ship, planet)) {
							ship->m_MoveTarget = planet;
							ship->m_IsTargetAShip = false;
							break;//docking successful 
						}
					}

					continue;
				} 
				//if this is an unowned planet
				if (!planet->owned) {

					if (sb.dockWithPlanet(ship, planet)) {
						ship->m_MoveTarget = planet;
						ship->m_IsTargetAShip = false;
						break;//docking successful 
					}

					continue;
				}
				//else this planet is owned by the enemy

			}



		}

		//move ships that havent been moved
		for (hlt::Ship* ship : map->m_ListOfMyUndockedShips) {
			//attack nearest enemy

		}

		for (hlt::Ship* ship : map->m_ListOfMyUndockedShips) {
			if (ship->m_MoveTarget != nullptr) {
				moves.push_back(ship->m_Move);
			}
		}
#endif // USE_OLD_SYSTEM

		//debugLogMoves(map);

		if (!hlt::out::send_moves(moves)) {
			hlt::Log::log("send_moves failed; exiting");
			break;
		}

		//timer info
#ifndef READY_FOR_BUILD
		auto t2 = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> diff = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
		hlt::Log::log("Turn Time: " + std::to_string(1000 * diff.count()));
#endif
	}
}
