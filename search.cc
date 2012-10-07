#include "search.h"

#include <unordered_set>
#include <unordered_map>
#include <thread>

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
    
    std::priority_queue<Node,
                std::vector<Node>,
                Comp_Target_Displacement>        frontier_fw;
    std::unordered_set<Node>          interior_fw;
    std::unordered_map<Node, Node>    previous_fw;
    for (Node const& node : initial_nodes) {
        frontier_fw.push(node);
    }
    
    bool found_fw = false;
    Node target_fw;
    
    std::priority_queue<Node,
                std::vector<Node>,
                Comp_Source_Displacement>        frontier_rv;
    std::set<Node>          interior_rv;
    std::map<Node, Node>    previous_rv;
    for (Node const& node : terminal_nodes) {
        frontier_rv.push(node);
    }
    
    bool found_rv = false;
    Node target_rv;
    
    std::vector<Node> neighbors_fw;
    std::vector<Node> neighbors_rv;
      
    int i = 0;
    while ((!found_fw && !frontier_fw.empty()) && (!found_rv && !frontier_rv.empty())) {
		i++;
		if (frontier_fw.empty() || frontier_rv.empty()) return;

		// Forward
		std::thread th([&](){
        	Node current = frontier_fw.top();
            frontier_fw.pop();
            interior_fw.insert(current);

			// Get all successor nodes for this node
			neighbors_fw.clear();
            current.get_successors(neighbors_fw);
            
            // Insert new non-visted nodes to forward priority queue
            std::for_each(neighbors_fw.begin(), neighbors_fw.end(),[&](Node & neighbor) {
                if (interior_fw.find(neighbor) == interior_fw.end()) {
                    previous_fw[neighbor] = current;
                    interior_fw.insert(neighbor);
                    frontier_fw.push(neighbor);
                }
            });
        });
        
        // Reverse
		{
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
		
		th.join();
		
		// Find overlapping between forward and reverse
		std::for_each(neighbors_rv.begin(), neighbors_rv.end(),[&](Node & neighbor) {
			if (interior_fw.find(neighbor) != interior_fw.end()) {
				found_rv = found_fw = true;
				target_rv = target_fw = neighbor;
				return;
			}  		
		});
		
		// Find overlapping between forward and reverse
		std::for_each(neighbors_fw.begin(), neighbors_fw.end(),[&](Node & neighbor) {
			if (interior_rv.find(neighbor) != interior_rv.end()) {
				found_rv = found_fw = true;
				target_rv = target_fw = neighbor;
				return;
			} 		
		});	
//                    if (neighbor.is_target()) {
//                        found_fw = true;
//                        target_fw = neighbor;
//                        break;
//                    }              
//                    if (neighbor.is_source()) {
//                        found_rv = true;
//                        target_rv = neighbor;
//                        break;
//                    }
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


void bidirectional_search(Maze const& maze,
                          std::vector<Node> const& initial_nodes,
                          std::vector<Node> const& terminal_nodes,
                          std::vector<Node>& steps) {
    
    std::priority_queue<Node,
                std::vector<Node>,
                Comp_Target_Displacement>        frontier_fw;
    std::unordered_set<Node>          interior_fw;
    std::unordered_map<Node, Node>    previous_fw;
    for (Node const& node : initial_nodes) {
        frontier_fw.push(node);
    }
    
    bool found_fw = false;
    Node target_fw;
    
    std::priority_queue<Node,
                std::vector<Node>,
                Comp_Source_Displacement>        frontier_rv;
    std::set<Node>          interior_rv;
    std::map<Node, Node>    previous_rv;
    for (Node const& node : terminal_nodes) {
        frontier_rv.push(node);
    }
    
    bool found_rv = false;
    Node target_rv;
    
    std::vector<Node> neighbors_fw;
    std::vector<Node> neighbors_rv;
      
    int i = 0;
    while ((!found_fw && !frontier_fw.empty()) && (!found_rv && !frontier_rv.empty())) {
		i++;
		if (frontier_fw.empty() || frontier_rv.empty()) return;

		// Forward
		{ 
        	Node current = frontier_fw.top();
            frontier_fw.pop();
            interior_fw.insert(current);

			// Get all successor nodes for this node
			neighbors_fw.clear();
            current.get_successors(neighbors_fw);
            
            // Insert new non-visted nodes to forward priority queue
            std::for_each(neighbors_fw.begin(), neighbors_fw.end(),[&](Node & neighbor) {
                if (interior_fw.find(neighbor) == interior_fw.end()) {
                    previous_fw[neighbor] = current;
                    interior_fw.insert(neighbor);
                    frontier_fw.push(neighbor);
                }
            });
        }
        
        // Reverse
		{
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
		
		// Find overlapping between forward and reverse
		std::for_each(neighbors_rv.begin(), neighbors_rv.end(),[&](Node & neighbor) {
			if (interior_fw.find(neighbor) != interior_fw.end()) {
				found_rv = found_fw = true;
				target_rv = target_fw = neighbor;
				return;
			}  		
		});
		
		// Find overlapping between forward and reverse
		std::for_each(neighbors_fw.begin(), neighbors_fw.end(),[&](Node & neighbor) {
			if (interior_rv.find(neighbor) != interior_rv.end()) {
				found_rv = found_fw = true;
				target_rv = target_fw = neighbor;
				return;
			} 		
		});	
//                    if (neighbor.is_target()) {
//                        found_fw = true;
//                        target_fw = neighbor;
//                        break;
//                    }              
//                    if (neighbor.is_source()) {
//                        found_rv = true;
//                        target_rv = neighbor;
//                        break;
//                    }
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

