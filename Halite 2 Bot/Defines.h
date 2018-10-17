#pragma once

//#define READY_FOR_BUILD
#define LOGGING
//#define USEASTAR

#ifdef LOGGING
#define ezLog(ship,x) hlt::Log::log(std::to_string(ship->entity_id) + ": " + x);
#else
#define ezLog(ship,x) 
#endif // LOGGING
