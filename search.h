#ifndef DD2380_SEARCH_H
#define DD2380_SEARCH_H

#include "node.h"

#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <queue>


class Comp_Source_Displacement {
public:
    bool operator ()(Node const& n1, Node const& n2) const {
        return n2.source_displacement() < n1.source_displacement();
    }
};

class Comp_Target_Displacement {
public:
    bool operator ()(Node const& n1, Node const& n2) const {
        return n2.target_displacement() < n1.target_displacement();
    }
};

typedef std::priority_queue<Node, std::vector<Node>, Comp_Target_Displacement> CTDPriorityQueue;
typedef std::priority_queue<Node, std::vector<Node>, Comp_Source_Displacement> CSDPriorityQueue;

void bidirectional_find_forward(CTDPriorityQueue &, 
						std::unordered_set<Node> &, 
						std::unordered_map<Node, Node> &,
						std::vector<Node> &);
						
void bidirectional_find_reverse(CSDPriorityQueue &, 
						std::unordered_set<Node> &, 
						std::unordered_map<Node, Node> &,
						std::vector<Node> &);
												
void find_overlapping_neighbors(std::vector<Node> &, 
							std::unordered_set<Node> &,
							bool &, bool &,
							Node &, Node &);
							
													
#endif
