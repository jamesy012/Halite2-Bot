#include "map.hpp"
#include "log.hpp"
#include "hlt.hpp"

//Macro for sorting objects by a distance
//Note: objectDistance value will be modifyed
//arraySize - amount of object in objectDistancePrefix, objectReferencePrefix and the returnList arrays
//objectDistance - object to get the distance double from
//objectReference - where to get a reference from to put into the return list
//returnList - array that will be sorted based on objectDistance and from objectReference
//Prefix - object to accuss via array index
//Suffix - member from that array (CAN BE EMPTY)
#define SORTDISTANCE(arraySize,												\
objectDistancePrefix, objectDistanceSuxfix,									\
objectReferencePrefix, objectReferenceSuffix,								\
returnList)																	\
for (unsigned int q = 0; q < arraySize; q++) {								\
	/* index of next closest entity */										\
	int index = -1;															\
	/* value of the smallest distance */									\
	double smallest = 9999;													\
	/* go through all objectDistance looking for the smallest */ 			\
	for (unsigned int i = 0; i < arraySize; i++) {							\
		/* check if it's closer */ 											\
		if (objectDistancePrefix[i] objectDistanceSuxfix < smallest) {		\
			/* assign new closest value */ 									\
			smallest = objectDistancePrefix[i] objectDistanceSuxfix;		\
			/* update to new smallest index */								\
			index = i;														\
		}																	\
	}																		\
	/* assign next object in the return list with the next closest object */\
	returnList[q] = objectReferencePrefix[index] objectReferenceSuffix;		\
	/* remove that planets distance so it doesn't show up again */			\
	objectDistancePrefix[index] objectDistanceSuxfix = 9999;				\
}

namespace hlt {


	Map::Map(const int width, const int height, hlt::PlayerId a_PlayerID) : map_width(width), map_height(height),m_PlayerID(a_PlayerID){
		
	}

	void Map::maploaded() {
		//calc amount of ships
		for (unsigned int i = 0; i < ship_map.size(); i++) {
			if (i == m_PlayerID) {
				m_NumberOfMyShips = ship_map.at(i).size();
				for (unsigned int q = 0; q < ship_map.at(i).size(); q++) {
					if (ships.at(i).at(q).docking_status == hlt::ShipDockingStatus::Undocked) {
						m_NumberOfMyUndockedShips++;
					}
				}
			} else {
				m_NumberOfEnemyShips += ship_map.at(i).size();
			}
		}

		for (unsigned int i = 0; i < planets.size(); i++) {
			if (planets[i].owner_id == m_PlayerID) {
				m_NumberOfMyPlanets++;
			}
		}
	}

	Ship & Map::get_ship(PlayerId player_id, EntityId ship_id) {
		return ships.at(player_id).at(ship_map.at(player_id).at(ship_id));
	}

	Planet& Map::get_planet(EntityId planet_id) {
		return planets.at(planet_map.at(planet_id));
	}

	const std::vector<Map::ShipDataStuct> Map::getShipDataStuct(const hlt::Entity& a_Entity) const {

		//create distance vector
		std::vector<ShipDataStuct> shipDistanceInfo = std::vector<ShipDataStuct>(m_NumberOfEnemyShips);

		int index = -1;
		//set data for distance vector
		for (unsigned int i = 0; i < ship_map.size(); i++) {
			if (i == m_PlayerID) {
				continue;
			}
			for (unsigned int q = 0; q < ship_map.at(i).size(); q++) {
				const Ship& ship = ships.at(i).at(q);
				shipDistanceInfo[++index].m_Ship = &ship;

				shipDistanceInfo[index].m_Distance = a_Entity.location.get_distance_to(shipDistanceInfo[index].m_Ship->location);
			}
		}

		return shipDistanceInfo;
	}

	const std::vector<const hlt::Ship*> Map::getEnemyShipsByDistance(const hlt::Entity& a_Entity) const {
		std::vector<Map::ShipDataStuct> shipDistanceInfo = getShipDataStuct(a_Entity);

		//create vector that lists planets in order of distance
		std::vector<const hlt::Ship*> shipsSorted = std::vector<const hlt::Ship*>(m_NumberOfEnemyShips);

		SORTDISTANCE(shipDistanceInfo.size(), shipDistanceInfo, .m_Distance, shipDistanceInfo, .m_Ship,shipsSorted);

		//return planets
		return shipsSorted;
	}
	const std::vector<const hlt::Planet*> Map::getPlanetsByDistance(const hlt::Entity& a_Entity) const {
		//create distance vector
		std::vector<double> distances = std::vector<double>(planets.size());
		//set data for distance vector
		for (unsigned int i = 0; i < planets.size(); i++) {
			distances[i] = a_Entity.location.get_distance_to(planets[i].location);
		}

		//create vector that lists planets in order of distance
		std::vector<const hlt::Planet*> planetsSorted = std::vector<const hlt::Planet*>(planets.size());

		SORTDISTANCE(distances.size(), distances, , &planets, , planetsSorted);

		//return planets
		return planetsSorted;
	}

	const unsigned int Map::getNumOfEnemyShips() const {
		return m_NumberOfEnemyShips;
	}

	const unsigned int Map::getNumOfMyShips() const {
		return m_NumberOfMyShips;
	}
}
