#include "search.h"

#include <unordered_set>
#include <unordered_map>
#include <thread>
#include <chrono>

std::string path_to_string(std::vector<Maze::position> const& path) {
    std::string solution_string = "";
    if (!path.empty()) {
        for (size_t i = 1; i < path.size(); i++) {
            if (       path[i] - path[i-1] == Maze::position( 0, -1)) {
                solution_string += "U";
            } else if (path[i] - path[i-1] == Maze::position( 1,  0)) {
                solution_string += "R";
            } else if (path[i] - path[i-1] == Maze::position( 0,  1)) {
                solution_string += "D";
            } else if (path[i] - path[i-1] == Maze::position(-1,  0)) {
                solution_string += "L";
            }
        }
    }
    return solution_string;
}



void reverse_best_first_search(Maze const& maze,
                               std::vector<Node> const& root_nodes,
                               std::vector<Node>& steps) {
    
    std::priority_queue<Node,
                std::vector<Node>,
                Comp_Source_Displacement>   frontier;
    std::unordered_set<Node>                          interior;
    std::unordered_map<Node, Node>                    previous;
    for (Node const& node : root_nodes) {
        frontier.push(node);
    }
    
    bool found = false;
    Node target;
    while (!found && !frontier.empty()) {
        if (frontier.empty()) return;
        Node current = frontier.top();
        frontier.pop();
        interior.insert(current);
        std::queue<Node> neighbors;
        current.get_predecessors(neighbors);
        while (!neighbors.empty()) {
            Node neighbor = neighbors.front();
            neighbors.pop();
            if (interior.find(neighbor) == interior.end()
//                && neighbor.source_displacement() == std::numeric_limits<size_t>::max()
                ) {
                previous[neighbor] = current;
                if (neighbor.is_source()) {
                    found = true;
                    target = neighbor;
                    break;
                }
                interior.insert(neighbor);
                frontier.push(neighbor);
            }
        }
    }
    
    if (!found) return;
    
    // Output solution
    Node current = target;
    while (current) {
        steps.push_back(current);
        current = previous[current];
    }
}

void forward_best_first_search(Maze const& maze,
                               std::vector<Node> const& root_nodes,
                               std::vector<Node>& steps) {
    
    std::priority_queue<Node,
                std::vector<Node>,
                Comp_Target_Displacement>   frontier;
    std::unordered_set<Node>          interior;
    std::unordered_map<Node, Node>    previous;
    for (Node const& root : root_nodes) {
        frontier.push(root);
    }
    
    bool found = false;
    Node target;
    while (!found && !frontier.empty()) {
        if (frontier.empty()) return;
        Node current = frontier.top();
        frontier.pop();
        interior.insert(current);
        std::queue<Node> neighbors;
        current.get_successors(neighbors);
        while (!neighbors.empty()) {
            Node neighbor = neighbors.front();
            neighbors.pop();
            if (interior.find(neighbor) == interior.end()
                && neighbor.source_displacement() != std::numeric_limits<size_t>::max()) {
                previous[neighbor] = current;
//                std::cout << neighbor << std::endl;
                if (neighbor.is_target()) {
                    found = true;
                    target = neighbor;
                    break;
                }
                interior.insert(neighbor);
                frontier.push(neighbor);
            }
        }
    }
    
    if (!found) return;
    
    // Output solution
    std::vector<Node> temp;
    Node current = target;
    while (current) {
        temp.push_back(current);
        current = previous[current];
    }
    for (int i = temp.size()-1; i > 0; i--) {
        steps.push_back(temp[i]);
    }
}

void threaded_bidirectional_search(Maze const& maze,
                          std::vector<Node> const& initial_nodes,
                          std::vector<Node> const& terminal_nodes,
                          std::vector<Node>& steps) {
    // Forward setup
    CTDPriorityQueue frontier_fw;
    std::unordered_set<Node> interior_fw;
    std::unordered_map<Node, Node> previous_fw;
    for (Node const& node : initial_nodes) {
        frontier_fw.push(node);
    }
    
    bool found_fw = false;
    Node target_fw;  
    std::vector<Node> neighbors_fw;
    
    // Reverse setup    
	CSDPriorityQueue frontier_rv;
    std::unordered_set<Node> interior_rv;
    std::unordered_map<Node, Node> previous_rv;
    
    for (Node const& node : terminal_nodes) {
        frontier_rv.push(node);
    }
    
    bool found_rv = false;
    Node target_rv;
    std::vector<Node> neighbors_rv;
    
    //Threaded variables
    volatile bool kill = false;
    volatile bool work = false;
    
    std::chrono::microseconds dur(100);
    
	std::thread th([&](){
		while(true)
		{
			if(work)
			{	
				if(frontier_rv.empty())
				{
					std::cout << "Empty reverse frontier" << std::endl;
					work = false;
					kill = true;
					return;
				}
				//std::cout << "slave working" << std::endl;
				bidirectional_find_reverse(frontier_rv, interior_rv, previous_rv, neighbors_rv);
				work = false;
			}
			
			if(kill)
			{
				work = false;
				std::cout << "Killed!" << std::endl;
				return;
			}
		}
	});

    while ((!found_fw) && (!found_rv)) {
		if (frontier_fw.empty()) {
				std::cout << "Empty forward frontier" << std::endl;
				kill = true;
				th.join();
				return;	
		}
		
        work = true;
        //std::cout << "main working" << std::endl;
    	bidirectional_find_forward(frontier_fw, interior_fw, previous_fw, neighbors_fw);
		
		int i = 0;
		while(true){
			if(!work) break;
			std::cout << "waiting " << i++ << std::endl;
		}
		
		find_overlapping_neighbors(neighbors_fw, interior_rv, found_rv, found_fw,
							target_rv, target_fw);
		find_overlapping_neighbors(neighbors_rv, interior_fw, found_rv, found_fw,
							target_rv, target_fw);
    }
    std::cout << "Finished" << std::endl;
    kill = true;
    th.join();
    std::cout << "Joined" << std::endl;
    
    if (!found_fw && !found_rv) return;
    
    if (found_fw) {   // Output forward solution
        std::vector<Node> temp;
        Node current = target_fw;
        while (current) {
            temp.push_back(current);
            current = previous_fw[current];
        }
        for (int i = temp.size()-1; i > 0; i--) {
            steps.push_back(temp[i]);
        }
    }
    if (found_rv) {   // Output solution
        Node current = target_rv;
        while (current) {
            steps.push_back(current);
            current = previous_rv[current];
        }
    }
}



void bidirectional_search(Maze const& maze,
                          std::vector<Node> const& initial_nodes,
                          std::vector<Node> const& terminal_nodes,
                          std::vector<Node>& steps) {
    // Forward setup
    CTDPriorityQueue frontier_fw;
    std::unordered_set<Node> interior_fw;
    std::unordered_map<Node, Node> previous_fw;
    for (Node const& node : initial_nodes) {
        frontier_fw.push(node);
    }
    
    bool found_fw = false;
    Node target_fw;  
    std::vector<Node> neighbors_fw;
    
    // Reverse setup    
	CSDPriorityQueue frontier_rv;
    std::unordered_set<Node> interior_rv;
    std::unordered_map<Node, Node> previous_rv;
    
    for (Node const& node : terminal_nodes) {
        frontier_rv.push(node);
    }
    
    bool found_rv = false;
    Node target_rv;
    std::vector<Node> neighbors_rv;

    while ((!found_fw && !frontier_fw.empty()) && (!found_rv && !frontier_rv.empty())) {
		if (frontier_fw.empty() || frontier_rv.empty()) return;
        
    	bidirectional_find_forward(frontier_fw, interior_fw, previous_fw, neighbors_fw);
    
		bidirectional_find_reverse(frontier_rv, interior_rv, previous_rv, neighbors_rv);
		
		find_overlapping_neighbors(neighbors_fw, interior_rv, found_rv, found_fw,
							target_rv, target_fw);
		find_overlapping_neighbors(neighbors_rv, interior_fw, found_rv, found_fw,
							target_rv, target_fw);
    }
    
    if (!found_fw && !found_rv) return;
    
    if (found_fw) {   // Output forward solution
        std::vector<Node> temp;
        Node current = target_fw;
        while (current) {
            temp.push_back(current);
            current = previous_fw[current];
        }
        for (int i = temp.size()-1; i > 0; i--) {
            steps.push_back(temp[i]);
        }
    }
    if (found_rv) {   // Output solution
        Node current = target_rv;
        while (current) {
            steps.push_back(current);
            current = previous_rv[current];
        }
    }
}


void bidirectional_find_forward(CTDPriorityQueue & frontier_fw, 
						std::unordered_set<Node> & interior_fw, 
						std::unordered_map<Node, Node> & previous_fw,
						std::vector<Node> & neighbors_fw) {
	Node current = frontier_fw.top();
	frontier_fw.pop();
	interior_fw.insert(current);

	// Get all successor nodes for this node
	neighbors_fw.clear();
	current.get_successors(neighbors_fw);

	// Insert new non-visted nodes to forward priority queue
	std::for_each(neighbors_fw.begin(), neighbors_fw.end(),[&](Node & neighbor) {
		if (interior_fw.find(neighbor) == interior_fw.end()
		 && neighbor.source_displacement() != std::numeric_limits<size_t>::max()) {
		    previous_fw[neighbor] = current;
		    interior_fw.insert(neighbor);
		    frontier_fw.push(neighbor);
		}
	});
}   
void bidirectional_find_reverse(CSDPriorityQueue & frontier_rv, 
						std::unordered_set<Node> & interior_rv, 
						std::unordered_map<Node, Node> & previous_rv,
						std::vector<Node> & neighbors_rv) {
	Node current = frontier_rv.top();
	frontier_rv.pop();
	interior_rv.insert(current);

	// Get all predecessor nodes for this node
	neighbors_rv.clear();
	current.get_predecessors(neighbors_rv);

	// Insert new non-visted nodes to reverse priority queue
	std::for_each(neighbors_rv.begin(), neighbors_rv.end(),[&](Node & neighbor) {
		if (interior_rv.find(neighbor) == interior_rv.end()) {
		    previous_rv[neighbor] = current;
		    interior_rv.insert(neighbor);
		    frontier_rv.push(neighbor);
		}
	});						
}
void find_overlapping_neighbors(std::vector<Node> & neighbors, 
							std::unordered_set<Node> & interior,
							bool & found_rv, bool & found_fw,
							Node & target_rv, Node & target_fw) {
	// Find overlapping between forward and reverse
	std::for_each(neighbors.begin(), neighbors.end(),[&](Node & neighbor) {
		if (interior.find(neighbor) != interior.end()) {
			found_rv = found_fw = true;
			target_rv = target_fw = neighbor;
			return;
		} 		
	});
}
