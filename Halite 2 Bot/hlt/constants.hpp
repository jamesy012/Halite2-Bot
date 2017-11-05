#pragma once

namespace hlt {
	namespace constants {
		////////////////////////////////////////////////////////////////////////
		// Implementation-independent language-agnostic constants

		/** Games will not have more players than this */
		constexpr int MAX_PLAYERS = 4;

		/** Max number of units of distance a ship can travel in a turn */
		constexpr int MAX_SPEED = 7;

		/** Radius of a ship */
		constexpr double SHIP_RADIUS = 0.5;

		/** Starting health of ship, also its max */
		constexpr int MAX_SHIP_HEALTH = 255;

		/** Starting health of ship, also its max */
		constexpr int BASE_SHIP_HEALTH = 255;

		/** Weapon cooldown period */
		constexpr int WEAPON_COOLDOWN = 1;

		/** Weapon damage radius */
		constexpr double WEAPON_RADIUS = 5.0;

		/** Weapon damage */
		constexpr int WEAPON_DAMAGE = 64;

		/** Radius in which explosions affect other entities */
		constexpr double EXPLOSION_RADIUS = 10.0;

		/** Distance from the edge of the planet at which ships can try to dock */
		constexpr double DOCK_RADIUS = 4.0;

		/** Number of turns it takes to dock a ship */
		constexpr unsigned int DOCK_TURNS = 5;

		/** Number of turns it takes to create a ship per docked ship */
		constexpr int BASE_PRODUCTIVITY = 6;

		/** Distance from the planets edge at which new ships are created */
		constexpr int SPAWN_RADIUS = 2;

		////////////////////////////////////////////////////////////////////////
		// Implementation-specific constants

		constexpr double FORECAST_FUDGE_FACTOR = SHIP_RADIUS + 0.2;
		constexpr int MAX_NAVIGATION_CORRECTIONS = 40;

		/**
		 * Used in Location::get_closest_point()
		 * Minimum distance specified from the object's outer radius.
		 */
		constexpr double MIN_DISTANCE_FOR_CLOSEST_POINT = 3;


		constexpr double SHIP_PREDICTED_MOVE_AVOIDANCE_RADIUS = 2;

		constexpr double DISTANCE_FOR_USED_OBJECT = 9999;
		constexpr double DISTANCE_FOR_SAME_OBJECT = DISTANCE_FOR_USED_OBJECT - 1;
		constexpr double DISTANCE_FOR_DOCKED_OBJECT = DISTANCE_FOR_USED_OBJECT - 2;


		constexpr double NUM_OF_UNDOCKED_SHIPS_FOR_SHIP_MOVE_PREDICTION = 20;
	}
}
