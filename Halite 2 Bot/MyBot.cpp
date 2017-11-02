#include "hlt/hlt.hpp"
#include "hlt/navigation.hpp"

std::vector<hlt::Move> moves;

void dockWithPlanet(const hlt::Map& a_Map,const hlt::Ship& a_Ship,const hlt::Planet& a_Planet) {
	if (a_Ship.can_dock(a_Planet)) {
		moves.push_back(hlt::Move::dock(a_Ship.entity_id, a_Planet.entity_id));
		return;
	}

	const hlt::possibly<hlt::Move> move =
		hlt::navigation::navigate_ship_to_dock(a_Map, a_Ship, a_Planet, hlt::constants::MAX_SPEED);
	if (move.second) {
		moves.push_back(move.first);
	}
}

int main() {
    const hlt::Metadata metadata = hlt::initialize("Birb the killer robot!");
    const hlt::PlayerId player_id = metadata.player_id;

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
		const hlt::Map map = hlt::in::get_map();

        for (const hlt::Ship& ship : map.ships.at(player_id)) {
			if (ship.docking_status != hlt::ShipDockingStatus::Undocked) {
				continue;
			}

			std::vector<const hlt::Planet*> planetsByDistance = map.getPlanetsByDistance(ship.location);

            for (const hlt::Planet* planet : planetsByDistance) {
				if (planet->owned) {

					if (planet->owner_id == player_id) {
						if (!planet->is_full()) {
							dockWithPlanet(map, ship, *planet);
							break;
						}
						continue;
					} else {
						if (planet->docked_ships.size() != 0) {
							const hlt::possibly<hlt::Move> move =
							hlt::navigation::navigate_ship_to_dock(map, ship, map.get_ship(planet->owner_id, planet->docked_ships[0]), hlt::constants::MAX_SPEED);
							if (move.second) {
								moves.push_back(move.first);
							} else {
								continue;
							}
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
