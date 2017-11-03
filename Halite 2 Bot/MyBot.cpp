#include "hlt/hlt.hpp"
#include "hlt/navigation.hpp"

#include <cmath>

std::vector<hlt::Move> moves;

bool dockWithPlanet(const hlt::Map& a_Map, const hlt::Ship& a_Ship, const hlt::Planet& a_Planet) {
	if (a_Ship.can_dock(a_Planet)) {
		moves.push_back(hlt::Move::dock(a_Ship.entity_id, a_Planet.entity_id));
		return true;
	}

	const hlt::possibly<hlt::Move> move =
		hlt::navigation::navigate_ship_to_dock(a_Map, a_Ship, a_Planet, hlt::constants::MAX_SPEED);
	if (move.second) {
		moves.push_back(move.first);
		return true;
	}
	return false;
}

bool attackShip(const hlt::Map& a_Map, const hlt::Ship& a_Ship, const hlt::Ship& a_Target) {
	hlt::Location loc = a_Target.location;
	loc.pos_x += a_Ship.m_VelX / 2;
	loc.pos_y += a_Ship.m_VelY / 2;

	hlt::Location closestPos = a_Ship.location.get_closest_point(loc, a_Ship.radius / 2);

	const hlt::possibly<hlt::Move> move =
		hlt::navigation::navigate_ship_towards_target(a_Map, a_Ship, closestPos, hlt::constants::MAX_SPEED, true, hlt::constants::MAX_NAVIGATION_CORRECTIONS, M_PI / 90.0);
	if (move.second) {
		moves.push_back(move.first);
		return true;
	}
	return false;
}

int main() {
	const hlt::Metadata metadata = hlt::initialize("Birb the killer robot! v4");
	const hlt::PlayerId player_id = metadata.player_id;
	hlt::m_PLAYERID = player_id;

	const hlt::Map& initial_map = metadata.initial_map;

	// We now have 1 full minute to analyse the initial map.
	std::ostringstream initial_map_intelligence;
	initial_map_intelligence
		<< "width: " << initial_map.map_width
		<< "; height: " << initial_map.map_height
		<< "; players: " << initial_map.ship_map.size()
		<< "; my ships: " << initial_map.ship_map.at(player_id).size()
		<< "; planets: " << initial_map.planets.size();
	hlt::Log::log(initial_map_intelligence.str());


	for (;;) {
		moves.clear();
		hlt::Map map = hlt::in::get_map();

		bool isFirstShip = true;
		int index = -1;
		for (const hlt::Ship& ship : map.ships.at(player_id)) {
			std::vector<const hlt::Ship*> shipsByDistance = map.getEnemyShipsByDistance(ship);

			if (ship.docking_status != hlt::ShipDockingStatus::Undocked) {
				continue;
				/*
				//This just seems detrimental to the bot
				//might have to rethink this, because it's helpful
				if (map.get_planet(ship.docked_planet).docked_ships.size() == 1) {
					continue;
				}

				const hlt::Ship* targetShip = shipsByDistance[0];

				//note: we have already done the get distance too
				if (ship.location.get_distance_to(targetShip->location) <= 10) {
					moves.push_back(hlt::Move::undock(ship.entity_id));
					hlt::Log::log(std::to_string(ship.entity_id) + " Undocking - enemy too close");

					map.get_planet(ship.docked_planet).docked_ships.resize(map.get_planet(ship.docked_planet).docked_ships.size() - 1);
				}
				continue;
				*/
			}
			index++;


			bool didAttack = false;
			for (unsigned int i = 0; i < fmin(10u, shipsByDistance.size()); i++) {
				if (shipsByDistance[i] == nullptr) {
					continue;
				}
				const hlt::Ship* targetShip = shipsByDistance[i];

				//note: we have already done the get distance too
				if (ship.location.get_distance_to(targetShip->location) <= hlt::constants::MAX_SPEED*1.5) {
					hlt::Log::log(std::to_string(ship.entity_id) + " Enemy close - Attacking");

					if (attackShip(map, ship, *targetShip)) {
						didAttack = true;
						break;
					} else {
						hlt::Log::log(std::to_string(ship.entity_id) + " Enemy close - Attacking - FAILED??");
					}
				} else {
					break;
				}
			}
			if (didAttack) {
				continue;
			}

			if ((isFirstShip || index / 2 > (int) map.m_NumberOfMyUndockedShips) && (map.m_NumberOfMyUndockedShips != map.m_NumberOfMyShips || map.m_NumberOfMyPlanets == 0)) {
				const hlt::Ship* targetShip = nullptr;
				int tsIndex = 0;
				while (targetShip == nullptr && tsIndex < shipsByDistance.size()) {
					targetShip = shipsByDistance[tsIndex++];
				}
				if (targetShip != nullptr) {
					hlt::Log::log(std::to_string(ship.entity_id) + (isFirstShip ? " First ship - Attacking " : " Index above docked unit count - Attacking"));

					isFirstShip = false;
					if (attackShip(map, ship, *targetShip)) {
						continue;
					} else {
						hlt::Log::log(std::to_string(ship.entity_id) + " __ - Attacking - FAILED??");
					}
				}
			}

			std::vector<const hlt::Planet*> planetsByDistance = map.getPlanetsByDistance(ship);

			for (const hlt::Planet* planet : planetsByDistance) {
				if (planet->owned) {

					if (planet->owner_id == player_id) {
						if (!planet->is_full()) {
							if (dockWithPlanet(map, ship, *planet)) {
								break;
							} else {
								hlt::Log::log(std::to_string(ship.entity_id) + ": Unable to dock will unfull planet");
							}
						}
						continue;
					} else {
						if (planet->docked_ships.size() != 0) {
							if (attackShip(map, ship, map.get_ship(planet->owner_id, planet->docked_ships[0]))) {
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

				dockWithPlanet(map, ship, *planet);

				break;
			}
		}

		if (!hlt::out::send_moves(moves)) {
			hlt::Log::log("send_moves failed; exiting");
			break;
		}
	}
}
