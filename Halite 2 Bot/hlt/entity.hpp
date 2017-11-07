#pragma once

#include "location.hpp"
#include "types.hpp"

namespace hlt {
	struct Entity {
		EntityId entity_id;
		PlayerId owner_id;
		Location location;
		int health;
		double radius;

		bool is_alive() const {
			return health > 0;
		}

		struct DistEntity {
			Entity* m_Entity;
			double m_Distance;
		};

		Entity::DistEntity getDistEntity(Entity* a_Entity) const {
			Entity::DistEntity et;
			et.m_Entity = a_Entity;
			et.m_Distance = location.get_distance_to(a_Entity->location);
			return et;
		}

		static bool distEntitySort(Entity::DistEntity& a_Lhs, Entity::DistEntity& a_Rhs) { return (a_Lhs.m_Distance<a_Rhs.m_Distance); }

		std::vector<DistEntity> m_EnemyShipsByDistance;
		std::vector<DistEntity> m_FriendlyShipsByDistance;
		std::vector<DistEntity> m_ShipsByDistance;

		std::vector<DistEntity> m_EnemyPlanetsByDistance;
		std::vector<DistEntity> m_FriendlyPlanetsByDistance;
		std::vector<DistEntity> m_PlanetsByDistance;


	};
}
