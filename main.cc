
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <map>
#include <queue>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "csocket.h"
#include "maze.h"
#include "node.h"

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

void reverse_search(Maze const& maze,
                    std::vector<Node> const& root_nodes,
                    std::vector<Node>& steps) {
    
    std::queue<Node>        frontier;
    std::set<Node>          interior;
    std::map<Node, Node>    previous;
    for (Node const& node : root_nodes) {
        if (node.is_source()) { // Not very likely, but...
            steps.push_back(node);
            return;
        }
        frontier.push(node);
    }
    
    bool found = false;
    Node target;
    while (!found && !frontier.empty()) {
        if (frontier.empty()) return;
        Node current = frontier.front();
        frontier.pop();
        interior.insert(current);
        std::queue<Node> neighbors;
        current.get_predecessors(neighbors);
        while (!neighbors.empty()) {
            Node neighbor = neighbors.front();
            neighbors.pop();
            if (interior.find(neighbor) == interior.end()) {
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

void reverse_best_first_search(Maze const& maze,
                               std::vector<Node> const& root_nodes,
                               std::vector<Node>& steps) {
    
    std::priority_queue<Node,
                std::vector<Node>,
                Comp_Source_Displacement>   frontier;
    std::unordered_set<Node>                          interior;
    //std::unordered_map<Node, Node>                    previous;
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
                //previous[neighbor] = current;
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
        //current = previous[current];
        current = current.get_parent();
    }
}

void forward_best_first_search(Maze const& maze,
                               std::vector<Node> const& root_nodes,
                               std::vector<Node>& steps) {
    
    std::priority_queue<Node,
                std::vector<Node>,
                Comp_Target_Displacement>   frontier;
    std::set<Node>          interior;
    std::map<Node, Node>    previous;
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

void bidirectional_search(Maze const& maze,
                          std::vector<Node> const& initial_nodes,
                          std::vector<Node> const& terminal_nodes,
                          std::vector<Node>& steps) {
    
    std::priority_queue<Node,
                std::vector<Node>,
                Comp_Target_Displacement>        frontier_fw;
    std::set<Node>          interior_fw;
    std::map<Node, Node>    previous_fw;
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
    
    int i = 0;
    while ((!found_fw && !frontier_fw.empty())
        && (!found_rv && !frontier_rv.empty())) {
        i++;
        if (frontier_fw.empty() || frontier_rv.empty()) return;
        
//        if (i % 2 == 0)
        {   // Forward
            Node current = frontier_fw.top();
            frontier_fw.pop();
            interior_fw.insert(current);
            std::queue<Node> neighbors;
            current.get_successors(neighbors);
            while (!neighbors.empty()) {
                Node neighbor = neighbors.front();
                neighbors.pop();
                if (interior_fw.find(neighbor) == interior_fw.end()) {
                    previous_fw[neighbor] = current;
//                    if (neighbor.is_target()) {
//                        found_fw = true;
//                        target_fw = neighbor;
//                        break;
//                    }
                    if (interior_rv.find(neighbor) != interior_rv.end()) {
                        found_rv = found_fw = true;
                        target_rv = target_rv = neighbor;
                        break;
                    }
                    interior_fw.insert(neighbor);
                    frontier_fw.push(neighbor);
                }
            }
        }
        
        {   // Reverse
            Node current = frontier_rv.top();
            frontier_rv.pop();
            interior_rv.insert(current);
            std::queue<Node> neighbors;
            current.get_predecessors(neighbors);
            while (!neighbors.empty()) {
                Node neighbor = neighbors.front();
                neighbors.pop();
                if (interior_rv.find(neighbor) == interior_rv.end()) {
                    previous_rv[neighbor] = current;
//                    if (neighbor.is_source()) {
//                        found_rv = true;
//                        target_rv = neighbor;
//                        break;
//                    }
                    if (interior_fw.find(neighbor) != interior_fw.end()) {
                        found_rv = found_fw = true;
                        target_rv = target_rv = neighbor;
                        break;
                    }
                    interior_rv.insert(neighbor);
                    frontier_rv.push(neighbor);
                }
            }
        }
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

int main(int pArgC, char* pArgs[]) {
    
    if (pArgC < 4) {
        std::cerr << "usage: " << pArgs[0] << " host port boardnum" << std::endl;
        return -1;
    }

    soko::CSocket lSocket(pArgs[1], pArgs[2]);

    lSocket.WriteLine(pArgs[3]);
    
    int height;
    
    std::string line;

    //read number of rows
    lSocket.ReadLine(line);
    height = atoi(line.c_str());
    
    std::vector<Maze::position> player_ending_pos;
    bool player_in_maze = false; // Safety measure
    
    Maze maze;
    
    // ~~~~~~~~~ Generate maze ~~~~~~~~~
    // At the moment we generate the maze manually
    // from the outside. This part can and should be
    // moved to the maze class
    for (int y = 0; y < height; y++) {
        lSocket.ReadLine(line);
//        std::cout << line << std::endl;
        maze.add_row(line.size());
        for (unsigned x = 0; x < line.size(); x++) {
            if (line[x] == '@') {
                maze.set_player_starting_pos(Maze::position(x, y));
                player_in_maze = true;
                player_ending_pos.push_back(Maze::position(x, y));
            }
            if (line[x] == '+') {
                maze.set_player_starting_pos(Maze::position(x, y));
                player_in_maze = true;
                maze(x, y).type = Maze::Tile::Dest;
                maze.add_crates_ending_pos(Maze::position(x, y));
            }
            if (line[x] == '.') {
                maze(x, y).type = Maze::Tile::Dest;
                maze.add_crates_ending_pos(Maze::position(x, y));
            }
            if (line[x] == '*') {
                maze(x, y).type = Maze::Tile::Dest;
                maze.add_crates_ending_pos(Maze::position(x, y));
                maze.add_crates_starting_pos(Maze::position(x, y));
            }
            if (line[x] == '$') {
                maze.add_crates_starting_pos(Maze::position(x, y));
                player_ending_pos.push_back(Maze::position(x, y));
            }
            if (line[x] == '#') {
                maze(x, y).type = Maze::Tile::Obstacle;
            }
            if (line[x] == ' ') {
                maze(x, y).type = Maze::Tile::Floor;
                player_ending_pos.push_back(Maze::position(x, y));
            }
        }
    }
    maze.calculate_displacement_mapping();
    
    if (!player_in_maze) {
        // Assume it never happens
    }
    
//    if (crates_starting_pos.size() != dest_pos.size()) {
//        // Assume it never happens
//    }
    
    /* Create root nodes for the reverse problem */
    std::vector<Node> terminal_nodes;
    for (Maze::position const& pos : player_ending_pos) {
            terminal_nodes.push_back(Node(maze, pos, maze.get_crates_ending_pos()));
    }
    
    /* Create root nodes for the forward problem */
    std::vector<Node> initial_nodes;
    initial_nodes.push_back(Node(maze, maze.get_player_starting_pos(),
                                       maze.get_crates_starting_pos()));
    
    /* Solve the problem */
    std::vector<Node> steps;
    reverse_best_first_search(maze, terminal_nodes, steps);
//    forward_best_first_search(maze, initial_nodes, steps);
//    bidirectional_search(maze, initial_nodes, terminal_nodes, steps);
    
    /* We now have a sequence of nodes that leads to the goal 
     * Translate to player movements.
     */
    
    std::string solution_string = "";
    Maze::position current_pos = maze.get_player_starting_pos();
    for (Node const& node : steps) {
//        std::cout << node << std::endl;
//        std::cout << node.get_player_pos() << " to " << current_pos << std::endl;
        std::vector<Maze::position> path;
        auto const& dest = node.get_player_starting_pos();
        auto const& crates = node.get_crates_starting_pos();
        maze.find_path(dest, current_pos, crates, path);
        
        solution_string += path_to_string(path);
        current_pos = node.get_player_ending_pos();
        solution_string += " " + node.get_path() + " ";
    }
    
    lSocket.WriteLine(solution_string);
    lSocket.ReadLine(line);
    std::cout << line << std::endl;
    
}

