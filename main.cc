
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
#include "tile.h"
#include "maze.h"
#include "node.h"

std::string path_to_string(std::vector<Maze::position> const& path);

void reverse_search(Maze const& maze,
                    std::vector<Node> const& root_nodes,
                    std::vector<Node>& steps);

void reverse_best_first_search(Maze const& maze,
                               std::vector<Node> const& root_nodes,
                               std::vector<Node>& steps);

void forward_best_first_search(Maze const& maze,
                               std::vector<Node> const& root_nodes,
                               std::vector<Node>& steps);

void bidirectional_search(Maze const& maze,
                          std::vector<Node> const& initial_nodes,
                          std::vector<Node> const& terminal_nodes,
                          std::vector<Node>& steps);


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
                maze(x, y).setType(Tile::Dest);
                maze.add_crates_ending_pos(Maze::position(x, y));
            }
            if (line[x] == '.') {
                maze(x, y).setType(Tile::Dest);
                maze.add_crates_ending_pos(Maze::position(x, y));
            }
            if (line[x] == '*') {
                maze(x, y).setType(Tile::Dest);
                maze.add_crates_ending_pos(Maze::position(x, y));
                maze.add_crates_starting_pos(Maze::position(x, y));
            }
            if (line[x] == '$') {
                maze.add_crates_starting_pos(Maze::position(x, y));
                player_ending_pos.push_back(Maze::position(x, y));
            }
            if (line[x] == '#') {
                maze(x, y).setType(Tile::Obstacle);
            }
            if (line[x] == ' ') {
                maze(x, y).setType(Tile::Floor);
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

