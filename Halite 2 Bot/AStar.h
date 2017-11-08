#pragma once

#include <vector>
#include "Vector2.h"

namespace hlt {
	struct Ship;
	struct Planet;
	class Map;
};

//todo: store the last frames map when it comes to planets, so we dont have to compute those again
class AStar {
public:
	struct Node {
		int m_X = 0, m_Y = 0;
		//cost to travel over this tile (always 1, could be used to say dont go near a area??)
		double m_GCost = 0;
		double m_BaseGCost = 0;
		//distance to ending tile
		double m_HCost = 0;
		//calulation of h+g cost
		double m_FCost = 0;
		Node* m_Previous = nullptr;
		bool m_InStack = false;
		bool m_IsTravered = false;
		int m_TiledDistance = 0;

		bool m_CanTraverse = true;

		//what was the index of the last search, used to know if we should reset the nodes or not
		int m_LastSearchIndex = -1;

	};

	AStar();
	~AStar();

	void genMap(hlt::Map* a_Map, std::vector<hlt::Ship*>* a_Ships, std::vector<hlt::Planet>* a_Planets);

	std::vector<AStar::Node*> pathToNode(Node* a_Start, Node* a_End);
	AStar::Node* getNodeFromPosition(Vector2 a_Position);
	void addUsedPath(Vector2 a_Start, Vector2 a_End);
	int getClosestUnobsturctedNode(Vector2 a_Position, std::vector<AStar::Node*>* a_Path);

	void logToLogs();

private:


	enum Direction {
		kUp,
		kUpRight,
		kRight,
		kDownRight,
		kDown,
		kDownLeft,
		kLeft,
		kUpLeft,
		NUM_OF_DIRECTIONS
	};

	const double m_DirectionDist[2] = { 1, 1.41f };

	std::vector<std::vector<AStar::Node>> m_Map;
	int m_MapWidth, m_MapHeight;

	int m_CurrentSearchIndex = 0;

	std::vector<AStar::Node*> getNodesInArea(AStar::Node* a_StartingPoint, double a_Distance);
	AStar::Node* getNeighborFromNode(AStar::Node* a_Node, Direction a_Direction);
	AStar::Node* getNodeFromPosition(int a_X, int a_Y);
	void updateSearchIndex();
	bool checkSearchIndex(AStar::Node* a_NodeToCheck);
	double getDistanceBetweenNodes(AStar::Node* a_From, AStar::Node* a_To);

	static bool fCostSort(AStar::Node* a_Lhs, AStar::Node* a_Rhs) { return (a_Lhs->m_FCost < a_Rhs->m_FCost); }

};

