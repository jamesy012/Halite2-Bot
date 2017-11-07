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

		struct MovePositions {
			Vector2 m_Position;
			Ship* m_Ship;
		};

		//list of positions that ships will be moving to, this turn
		//the positions are slightly off where it will land next turn
		std::vector<MovePositions> m_MovePositions;

		unsigned int m_NumberOfShips = 0;
		unsigned int m_NumberOfEnemyShips = 0;
		unsigned int m_NumberOfFriendlyShips = 0;
		unsigned int m_NumberOfFriendlyUndockedShips = 0;

		unsigned int m_NumberOfPlanets = 0;
		unsigned int m_NumberOfEnemyPlanets = 0;
		unsigned int m_NumberOfFriendlyPlanets = 0;

		unsigned int m_TurnNum = 0;

		hlt::PlayerId m_PlayerID;

		Map(int width, int height, hlt::PlayerId a_PlayerID);
		void maploaded();

		//gets ship object from player ID and ship ID
		Ship& get_ship(PlayerId player_id, EntityId ship_id);

		//gets planet object from planet ID
		Planet& get_planet(EntityId planet_id);

	protected:
		
	private:
	
		void setUpEntityVectorCapacitys(hlt::Entity* a_Entity) const;
		void addDistEntityShips(hlt::Entity* a_Entity,bool a_SkipDocked = false);
		void addDistEntityPlanets(hlt::Entity* a_Entity);
	};
}
