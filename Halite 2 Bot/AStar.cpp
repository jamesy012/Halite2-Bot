#include "AStar.h"

#include "hlt/map.hpp"
#include "hlt/entity.hpp"
#include "hlt/log.hpp"

#include "hlt/entity.hpp"
#include "hlt/planet.hpp"

#include <queue>
#include <sstream>
#include <algorithm>



AStar::AStar() {
}


AStar::~AStar() {
}

void AStar::genMap(hlt::Map * a_Map, std::vector<hlt::Ship*>* a_Ships, std::vector<hlt::Planet>* a_Planets) {
	m_MapWidth = a_Map->map_width;
	m_MapHeight = a_Map->map_height;

	//gen map info
	m_Map = std::vector<std::vector<Node>>(m_MapWidth);
	for (int x = 0; x < m_MapWidth; x++) {
		m_Map[x] = std::vector<Node>(m_MapHeight);
		for (int y = 0; y < m_MapHeight; y++) {
			Node* node = &m_Map[x][y];
			node->m_X = x;
			node->m_Y = y;
		}
	}

	//add planets
	for (int i = 0; i < a_Planets->size(); i++) {
		std::vector<AStar::Node*> nodes = getNodesInArea(getNodeFromPosition(a_Planets->at(i).location.m_Pos), a_Planets->at(i).radius-1);
		for (int q = 0; q < nodes.size(); q++) {
			nodes[q]->m_CanTraverse = false;
		}
		for (int q = 0; q < a_Planets->at(i).docked_ships.size();q++) {
			hlt::Ship* ship = &a_Map->get_ship(a_Planets->at(i).owner_id, a_Planets->at(i).docked_ships[q]);
			nodes = getNodesInArea(getNodeFromPosition(ship->location.m_Pos), 2);
			for (int q = 0; q < nodes.size(); q++) {
				nodes[q]->m_CanTraverse = false;
			}
		}
	}

	for (int i = 0; i < a_Ships->size(); i++) {
		AStar::Node* node = getNodeFromPosition(a_Ships->at(i)->location.m_Pos);
		node->m_CanTraverse = false;	
		//if (a_Ships->at(i)->owner_id == a_Map->m_PlayerID) {
		//	for (int q = 0; q <= AStar::Direction::NUM_OF_DIRECTIONS; q++) {
		//		Node* neigbourNode = getNeighborFromNode(node, (AStar::Direction)q);
		//		if (neigbourNode == nullptr) {
		//			continue;
		//		}
		//		//if (a_Ships->at(i)->docking_status != hlt::ShipDockingStatus::Undocked) {
		//		neigbourNode->m_CanTraverse = false;
		//		//}
		//		//neigbourNode->m_BaseGCost += 3.0f;
		//	}
		//}
	}

}

std::vector<AStar::Node*> AStar::pathToNode(Node * a_Start, Node * a_End) {
	std::deque<AStar::Node*> queue;
	std::vector<AStar::Node*> tilesInPath;

	if (!a_End->m_CanTraverse) {
		hlt::Log::log("Cant traverse");
		return std::vector<AStar::Node*>();
	}

	updateSearchIndex();
	a_Start->m_LastSearchIndex = m_CurrentSearchIndex;

	queue.push_back(a_Start);

	a_Start->m_GCost = 0;
	a_Start->m_Previous = a_Start;
	a_Start->m_TiledDistance = 0;

	a_End->m_IsTravered = false;

	Node* ending = a_End;

	bool foundEnd = false;
	bool isFirstRun = true;

	while (!queue.empty()) {
		std::sort(queue.begin(), queue.end(), AStar::fCostSort);
		Node* node = queue.front();
		queue.pop_front();

		node->m_IsTravered = true;
		node->m_InStack = false;

		if (node == ending) {
			break;
		}

		if (foundEnd) {
			continue;
		}
		for (int i = 0; i <= AStar::Direction::NUM_OF_DIRECTIONS; i++) {
			Node* neigbourNode = getNeighborFromNode(node, (AStar::Direction)i);
			if (neigbourNode == nullptr) {
				continue;
			}

			if (neigbourNode == ending) {
				foundEnd = true;
			} else {
				if (!neigbourNode->m_CanTraverse && !isFirstRun) {
					continue;
				}

				if (neigbourNode->m_IsTravered || neigbourNode->m_InStack) {
					continue;
				}
			}
			if (checkSearchIndex(neigbourNode)) {
				neigbourNode->m_IsTravered = false;
				neigbourNode->m_InStack = false;
				neigbourNode->m_Previous = nullptr;
				neigbourNode->m_TiledDistance = 0;
				neigbourNode->m_FCost = neigbourNode->m_GCost = 999999;
				neigbourNode->m_HCost = 0;
			}


			double calcGCost = node->m_GCost + m_DirectionDist[i%2] + neigbourNode->m_BaseGCost;

			if (neigbourNode->m_HCost == 0) {
				neigbourNode->m_HCost = getDistanceBetweenNodes(a_End, neigbourNode);
			}
			double calcFCost = calcGCost + neigbourNode->m_HCost;

			if (calcFCost < neigbourNode->m_FCost) {
				neigbourNode->m_Previous = node;
				neigbourNode->m_FCost = calcFCost;
				neigbourNode->m_GCost = calcGCost;

				neigbourNode->m_InStack = true;
				neigbourNode->m_TiledDistance = node->m_TiledDistance + 1;

				queue.push_back(neigbourNode);

				//optimization??
				if (neigbourNode->m_TiledDistance >= 10) {
					ending = neigbourNode;
					foundEnd = true;
				}
			}

		}
		isFirstRun = false;
	}

	if (ending->m_IsTravered) {
		Node* current = ending;
		while (current != a_Start) {
			tilesInPath.push_back(current);
			current = current->m_Previous;
		}
		//add starting node
		tilesInPath.push_back(current);
	}

	std::reverse(tilesInPath.begin(), tilesInPath.end());

	return tilesInPath;
}

void AStar::logToLogs() {
	std::ostringstream mapString;

	for (int y = 0; y < m_MapHeight; y++) {
		for (int x = 0; x < m_MapWidth; x++) {
			mapString << (getNodeFromPosition(x, y)->m_CanTraverse ? getNodeFromPosition(x, y)->m_BaseGCost : 0);
		}
		mapString << "\n";
	}

	hlt::Log::log(mapString.str());
}

std::vector<AStar::Node*> AStar::getNodesInArea(AStar::Node * a_StartingPoint, double a_Distance) {
	std::queue<AStar::Node*> queue;
	std::vector<AStar::Node*> tilesInRange;

	updateSearchIndex();
	a_StartingPoint->m_LastSearchIndex = m_CurrentSearchIndex;

	queue.push(a_StartingPoint);

	while (!queue.empty()) {

		Node* node = queue.front();
		queue.pop();

		tilesInRange.push_back(node);

		node->m_IsTravered = true;
		node->m_InStack = false;

		for (int i = 0; i <= AStar::Direction::NUM_OF_DIRECTIONS; i++) {
			Node* neigbourNode = getNeighborFromNode(node, (AStar::Direction)i);
			if (neigbourNode == nullptr) {
				continue;
			}

			if (checkSearchIndex(neigbourNode)) {
				neigbourNode->m_IsTravered = false;
				neigbourNode->m_InStack = false;
			}

			if (neigbourNode->m_IsTravered || neigbourNode->m_InStack) {
				continue;
			}

			float distance = getDistanceBetweenNodes(a_StartingPoint, neigbourNode);

			if (distance <= a_Distance) {
				neigbourNode->m_InStack = true;
				queue.push(neigbourNode);
			}
		}

	}

	return tilesInRange;
}

AStar::Node * AStar::getNodeFromPosition(Vector2 a_Position) {
	return getNodeFromPosition(a_Position.m_X,a_Position.m_Y);
}

AStar::Node * AStar::getNodeFromPosition(int a_X, int a_Y) {
	return &m_Map[a_X][a_Y];

}

void AStar::addUsedPath(Vector2 a_Start, Vector2 a_End) {
	Vector2 diff = a_End - a_Start;
	double length = diff.length()+1;
	for (int i = 1; i <= length; i++) {
		Vector2 pos = a_Start + diff*(i / length);
		Node* node = getNodeFromPosition(pos);
		node->m_CanTraverse = false;
	}
}

int AStar::getClosestUnobsturctedNode(Vector2 a_Position, std::vector<AStar::Node*>* a_Path) {
	for (int i = a_Path->size()-1; i >= 1; i--) {
		Node* node = a_Path->at(i);
	
		Vector2 dir = a_Position - Vector2{ (double)node->m_X,(double)node->m_Y };
		double distance = dir.length();
		dir.normalize();

		bool isPathFree = true;
		for (int q = 1; q <= distance; q++) {
			Node* dirNode = getNodeFromPosition(a_Position + (dir * q / distance));
			if (!dirNode->m_CanTraverse) {
				isPathFree = false;
				break;
			}
		}

		if (isPathFree) {
			return i;
		}
	}

	return 0;
}

AStar::Node * AStar::getNeighborFromNode(AStar::Node * a_Node, Direction a_Direction) {
	switch (a_Direction) {
		case AStar::Direction::kUp:
			if (a_Node->m_Y != 0) {
				return &m_Map[(int) a_Node->m_X][(int) a_Node->m_Y - 1];
			}
			return nullptr;
		case AStar::Direction::kUpRight:
			if (a_Node->m_Y != 0 && a_Node->m_X != m_MapWidth - 1) {
				return &m_Map[(int) a_Node->m_X + 1][(int) a_Node->m_Y - 1];
			}
			return nullptr;
		case AStar::Direction::kRight:
			if (a_Node->m_X != m_MapWidth - 1) {
				return &m_Map[(int) a_Node->m_X + 1][(int) a_Node->m_Y];
			}
			return nullptr;
		case AStar::Direction::kDownRight:
			if (a_Node->m_Y != m_MapHeight - 1 && a_Node->m_X != m_MapWidth - 1) {
				return &m_Map[(int) a_Node->m_X + 1][(int) a_Node->m_Y + 1];
			}
			return nullptr;
		case AStar::Direction::kDown:
			if (a_Node->m_Y != m_MapHeight - 1) {
				return &m_Map[(int) a_Node->m_X][(int) a_Node->m_Y + 1];
			}
			return nullptr;
		case AStar::Direction::kDownLeft:
			if (a_Node->m_Y != m_MapHeight - 1 && a_Node->m_X != 0) {
				return &m_Map[(int) a_Node->m_X - 1][(int) a_Node->m_Y + 1];
			}
			return nullptr;
		case AStar::Direction::kLeft:
			if (a_Node->m_X != 0) {
				return &m_Map[(int) a_Node->m_X - 1][(int) a_Node->m_Y];
			}
			return nullptr;
		case AStar::Direction::kUpLeft:
			if (a_Node->m_Y != 0 && a_Node->m_X != 0) {
				return &m_Map[(int) a_Node->m_X - 1][(int) a_Node->m_Y - 1];
			}
			return nullptr;
	}
	return nullptr;
}

void AStar::updateSearchIndex() {
	m_CurrentSearchIndex++;
}

bool AStar::checkSearchIndex(AStar::Node * a_NodeToCheck) {
	if (a_NodeToCheck->m_LastSearchIndex != m_CurrentSearchIndex) {
		a_NodeToCheck->m_LastSearchIndex = m_CurrentSearchIndex;
		return true;
	}
	return false;
}

double AStar::getDistanceBetweenNodes(AStar::Node * a_From, AStar::Node * a_To) {
	const double dx = a_From->m_X - a_To->m_X;
	const double dy = a_From->m_Y - a_To->m_Y;
	return std::sqrt(dx*dx + dy*dy);
}
