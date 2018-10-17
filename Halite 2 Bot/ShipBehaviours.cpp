#include "ShipBehaviours.h"

#include "hlt/navigation.hpp"
#include "Defines.h"

#define BehaviourInternalPrams a_Map, a_Ship, a_MoveList

ShipBehaviours* ShipBehaviours::m_Singleton = nullptr;
hlt::Map* ShipBehaviours::m_CurrentMap = nullptr;

bool ShipBehaviours::dockWithPlanet(hlt::Ship* a_Ship, hlt::Planet* a_Planet) {
	//can we dock with a_Planet?
	if (a_Ship->can_dock(*a_Planet)) {
		ezLog(a_Ship, "DockwithPlanet - move - dock " + std::to_string(a_Planet->entity_id));
		//if so, then send a dock command
		a_Ship->m_Move = hlt::Move::dock(a_Ship->entity_id, a_Planet->entity_id);
		a_Planet->m_UnitsGettingSentTo++;
		return true;
	}

	const hlt::possibly<hlt::Move> move =
#ifdef USEASTAR
		hlt::navigation::aStarDock(m_AStar, &a_Ship, &a_Planet);
#else
		hlt::navigation::navigate_ship_to_dock(*m_CurrentMap, *a_Ship, *a_Planet, hlt::constants::MAX_SPEED);
#endif
	if (move.second) {
		ezLog(a_Ship, "DockwithPlanet - move - navigation " + std::to_string(a_Planet->entity_id));
		a_Ship->m_Move = move.first;
		a_Planet->m_UnitsGettingSentTo++;
		return true;
	}
	return false;
}

/*
bool ShipBehaviours::behaviourDocked(BehaviourFuncPrams) {
	//check if ship has anything to do with docking
	if (a_Ship->docking_status != hlt::ShipDockingStatus::Undocked) {
		//log this
		ezLog(a_Ship, "Is docked to: " + std::to_string(a_Ship->docked_planet));
		return true;
	}
	//nothing done here, return false
	return false;
}

bool ShipBehaviours::behaviourGotoPlanet(BehaviourFuncPrams) {
	//get planet list by distance
	auto planetList = &a_Ship->m_PlanetsByDistance;
	//go through each planet
	for (hlt::Entity::DistEntity& dePlanet : *planetList) {
		//get planet
		hlt::Planet* planet = (hlt::Planet*)dePlanet.m_Entity;
		//check if it's unowned
		if (!planet->owned || planet->owner_id == a_Map->m_PlayerID) {
			//cool, this planet doesnt have a owner.
			//lets check to see if we have already sent enough ships here
			if (planet->m_UnitsGettingSentTo != planet->docking_spots) {
				if (planet != nullptr) {
					ezLog(a_Ship, "Is docking to: " + std::to_string(planet->entity_id));
					//if we found a planet, then dock with it
					bool result = dockWithPlanet(BehaviourInternalPrams, planet);
					if (result) {
						return true;
					} else {
						ezLog(a_Ship, "Failed to dock to: " + std::to_string(planet->entity_id));
					}
				}
			}
		}
	}

	return false;
}

bool ShipBehaviours::behaviourAttackNearestEnemy(BehaviourFuncPrams) {
	return false;
}

bool ShipBehaviours::dockWithPlanet(BehaviourFuncPrams, hlt::Planet* a_Planet) {
	//can we dock with a_Planet?
	if (a_Ship->can_dock(*a_Planet)) {
		ezLog(a_Ship, "DockwithPlanet - move - dock " + std::to_string(a_Planet->entity_id));
		//if so, then send a dock command
		a_MoveList->push_back(hlt::Move::dock(a_Ship->entity_id, a_Planet->entity_id));
		a_Planet->m_UnitsGettingSentTo++;
		return true;
	}

	const hlt::possibly<hlt::Move> move =
#ifdef USEASTAR
		hlt::navigation::aStarDock(m_AStar, &a_Ship, &a_Planet);
#else
		hlt::navigation::navigate_ship_to_dock(*a_Map, *a_Ship, *a_Planet, hlt::constants::MAX_SPEED);
#endif
	if (move.second) {
		ezLog(a_Ship, "DockwithPlanet - move - navigation " + std::to_string(a_Planet->entity_id));
		a_MoveList->push_back(move.first);
		a_Planet->m_UnitsGettingSentTo++;
		return true;
	}
	return false;
}

bool ShipBehaviours::attackShip(BehaviourFuncPrams, const hlt::Ship & a_Target) {
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
		a_MoveList->push_back(move.first);
		return true;
	}
	return false;
}
*/
