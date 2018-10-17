#pragma once

#include "hlt/hlt.hpp"
#include "hlt/map.hpp"

//#define BehaviourFuncPrams hlt::Map* a_Map, hlt::Ship* a_Ship, std::vector<hlt::Move>* a_MoveList
//#define BehaviourPrams map, ship, &moves
#define BehaviourFuncPrams hlt::Map* a_Map, hlt::Ship* a_Ship
#define BehaviourPrams map, ship
#define runBehaviour(function) if(function) { continue; }

class ShipBehaviours {
public:
	static ShipBehaviours* m_Singleton;
	static hlt::Map* m_CurrentMap;

	bool dockWithPlanet(hlt::Ship* a_Ship, hlt::Planet* a_Planet);
	/*
	//docking behaviour
	//if the ship is docked/docking/undocking, it will return true
	//and add to that planets 
	bool behaviourDocked(BehaviourFuncPrams);

	bool behaviourGotoPlanet(BehaviourFuncPrams);

	bool behaviourAttackNearestEnemy(BehaviourFuncPrams);

	//finds the closest unwowned planet to a_Ship
	hlt::Planet* getClosestDesiredPlanet(hlt::Ship& a_Ship);
	//docks with planet
	//if not ship not in docking distance then this will navigate the ship to the planet
	
	bool attackShip(BehaviourFuncPrams, const hlt::Ship& a_Target);
	*/
};

