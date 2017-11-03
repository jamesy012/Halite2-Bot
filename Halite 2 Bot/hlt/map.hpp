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


		unsigned int m_NumberOfEnemyShips = 0;
		unsigned int m_NumberOfMyShips = 0;
		unsigned int m_NumberOfMyUndockedShips = 0;

		unsigned int m_NumberOfMyPlanets = 0;

		Map(int width, int height, hlt::PlayerId a_PlayerID);
		void maploaded();

		//gets ship object from player ID and ship ID
		Ship& get_ship(PlayerId player_id, EntityId ship_id);

		//gets planet object from planet ID
		Planet& get_planet(EntityId planet_id);

		//returns 
		const std::vector<const hlt::Ship*> getEnemyShipsByDistance(const hlt::Entity& a_Entity) const;
		const std::vector<const hlt::Ship*> getFriendlyShipsByDistance(const hlt::Entity& a_Entity) const;
		const std::vector<hlt::Planet*> getPlanetsByDistance(const hlt::Entity& a_Entity);


		const unsigned int getNumOfEnemyShips() const;
		const unsigned int getNumOfMyShips() const;
	protected:
		
	private:
		struct ShipDataStuct {
			double m_Distance;
			const hlt::Ship* m_Ship;
		};

		enum OwnerSelectionType {
			All,
			Friendly,
			Enemy,
		};

		hlt::PlayerId m_PlayerID;
	
		const std::vector<ShipDataStuct> getShipDataStuct(const hlt::Entity& a_Entity,const OwnerSelectionType a_Type,const bool a_IncludeDocked) const;
	};
}
