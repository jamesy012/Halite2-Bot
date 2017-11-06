#include "hlt_in.hpp"
#include "log.hpp"
#include "hlt_out.hpp"

#include "../Defines.h"

namespace hlt {
    namespace in {
        static int g_turn = 0;
        static std::string g_bot_name;
        static int g_map_width;
        static int g_map_height;

        void setup(const std::string& bot_name, int map_width, int map_height) {
            g_bot_name = bot_name;
            g_map_width = map_width;
            g_map_height = map_height;
        }

        Map* get_map(hlt::PlayerId a_PlayerID, Map* a_Map, std::chrono::steady_clock::time_point* a_TimeStart) {
            if (g_turn == 1) {
                out::send_string(g_bot_name);
            }

            const std::string input = get_string();

            if (!std::cin.good()) {
                // This is needed on Windows to detect that game engine is done.
                std::exit(0);
            }

#ifndef READY_FOR_BUILD
			if (a_TimeStart != nullptr) {
				*a_TimeStart = std::chrono::high_resolution_clock::now();
			}
#endif

            if (g_turn == 0) {
                Log::log("--- PRE-GAME ---");
            } else {
                Log::log("--- TURN " + std::to_string(g_turn) + " ---");
            }
            ++g_turn;

            return parse_map(input, g_map_width, g_map_height, a_PlayerID, a_Map);
        }
    }
}
