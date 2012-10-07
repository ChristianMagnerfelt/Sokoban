
#include <cassert>
#include <limits>
#include <algorithm>
#include <iostream>
#include <unordered_map>
#include <unordered_set>

#include "direction.h"
#include "maze.h"

Maze::Maze():
    tiles()
{}

Tile const& Maze::operator () (Maze::position pos) const {
    return (*this)(pos.first, pos.second);
}

Tile & Maze::operator () (Maze::position pos) {
    return (*this)(pos.first, pos.second);
}

Tile const& Maze::operator () (size_t x, size_t y) const {
    assert(y < this->height() && x < tiles[y].size());
    return tiles[y][x];
}

Tile & Maze::operator () (size_t x, size_t y) {
    assert(y < this->height() && x < tiles[y].size());
    return tiles[y][x];
}

inline bool Maze::isTileWalkable(const Maze::position & pos) const {
	return tiles[pos.second][pos.first].isWalkable();
}

size_t Maze::height() const {
    return tiles.size();
}

size_t Maze::width(size_t row) const {
    return tiles[row].size();
}

void Maze::add_row(size_t length) {
    tiles.push_back(std::vector<Tile>(length));
}

bool Maze::valid(Maze::position const& pos) const {
    return pos.second < height() && pos.first < width(pos.second);
}

bool Maze::isWalkable(const Maze::position & pos, const std::vector<Maze::position> & obstacles) const {
    if(!isTileWalkable(pos))
    	return false;
    else
    	return (std::find(obstacles.begin(), obstacles.end(), pos) == obstacles.end())?true:false;
}

void Maze::get_neighbors(std::queue<Maze::position>& neighbors, 
						Maze::position const& current, 
						std::vector<Maze::position> const& obstacles) const {
    if (valid(up(current)) && isWalkable(up(current), obstacles))
        neighbors.emplace(up(current));
    if (valid(right(current)) && isWalkable(right(current), obstacles))
        neighbors.emplace(right(current));
    if (valid(down(current)) && isWalkable(down(current), obstacles))
        neighbors.emplace(down(current));
    if (valid(left(current)) && isWalkable(left(current), obstacles))
        neighbors.emplace(left(current));
}

namespace {
    size_t current_mark = 0;
}

bool Maze::reachable(Maze::position const& source,
                     Maze::position const& target,
                     std::vector<Maze::position> const& obstacles) const {
    
    typedef Maze::position T;
    
    current_mark++;
    
    if (source == target) return true;
    
    std::queue<T>  frontier;
    //std::unordered_set<T>    interior;
    frontier.push(source);
    
    while (!frontier.empty()) {
        if (frontier.empty()) return false;
        T current = frontier.front();
        frontier.pop();
//            if (interior.find(current) != interior.end()) continue;
        //interior.insert(current);
        (*this)(current).mark = current_mark;
        std::queue<T> neighbors;
        get_neighbors(neighbors, current, obstacles);
        while (!neighbors.empty()) {
            T neighbor = neighbors.front();
            neighbors.pop();
            if ((*this)(neighbor).mark != current_mark) {
                if (neighbor == target) return true;
                frontier.push(neighbor);
                //interior.insert(neighbor);
                (*this)(neighbor).mark = current_mark;
            }
        }
    }
    return false;
}

void Maze::mark_reachable(Maze::position const& source,
                          std::vector<Maze::position> const& obstacles) const {
    
    typedef Maze::position T;
    
    current_mark++;
    
    std::queue<T>  frontier;
    //std::unordered_set<T>    interior;
    frontier.push(source);
    
    while (!frontier.empty()) {
        if (frontier.empty()) return;
        T current = frontier.front();
        frontier.pop();
//            if (interior.find(current) != interior.end()) continue;
        //interior.insert(current);
        (*this)(current).mark = current_mark;
        std::queue<T> neighbors;
        get_neighbors(neighbors, current, obstacles);
        while (!neighbors.empty()) {
            T neighbor = neighbors.front();
            neighbors.pop();
            if ((*this)(neighbor).mark != current_mark) {
                frontier.push(neighbor);
                //interior.insert(neighbor);
                (*this)(neighbor).mark = current_mark;
            }
        }
    }
}

bool Maze::is_marked(Maze::position const& pos) const {
    return (*this)(pos).mark == current_mark;
}

void Maze::find_path(Maze::position const& source,
                     Maze::position const& target,
                     std::vector<Maze::position> const& obstacles,
                     std::vector<Maze::position>& output) const {
    
    typedef Maze::position T;
    
    if (source == target) return;
    
    std::queue<T>  frontier;
    std::unordered_set<T>    interior;
    std::unordered_map<T, T> previous;
    frontier.push(source);
    
    bool found = false;
    while (!found && !frontier.empty()) {
        if (frontier.empty()) return;
        T current = frontier.front();
        frontier.pop();
        if (interior.find(current) != interior.end()) continue;
        interior.insert(current);
        std::queue<T> neighbors;
        get_neighbors(neighbors, current, obstacles);
        while (!neighbors.empty()) {
            T neighbor = neighbors.front();
            neighbors.pop();
            if (interior.find(neighbor) == interior.end()) {
                previous[neighbor] = current;
                if (neighbor == target) {
                    found = true;
                    break;
                }
                frontier.push(neighbor);
            }
        }
    }
    
    // Output solution
    T current = target;
    while (current != source) {
        output.push_back(current);
        current = previous[current];
    }
    output.push_back(current);
    
}

void Maze::calculate_displacement_mapping() {
    typedef Maze::position T;
    
    {
        std::queue<T>       frontier;
        std::unordered_set<T>         interior;
        std::unordered_map<T, size_t> cost;
        for (Maze::position source : crates_starting_pos) {
            frontier.push(source);
            interior.insert(source);
            cost[source] = 0;
            (*this)(source).source_displacement = 0;
        }
        
        while (!frontier.empty()) {
            T current = frontier.front();
            frontier.pop();
    //        if (interior.find(current) != interior.end()) continue;
    //        interior.insert(current);
            std::queue<T> neighbors;
            if ((*this)(up(current)).type != Tile::Obstacle &&
                            (*this)(down(current)).type != Tile::Obstacle) {
                neighbors.push(up(current));
                neighbors.push(down(current));
            }
            if ((*this)(left(current)).type != Tile::Obstacle && 
                            (*this)(right(current)).type != Tile::Obstacle) {
                neighbors.push(left(current));
                neighbors.push(right(current));
            }
//            if ((*this)(up(current)).type != Tile::Obstacle) {
//                neighbors.push(up(current));
//            }
//            if ((*this)(right(current)).type != Tile::Obstacle) {
//                neighbors.push(right(current));
//            }
//            if ((*this)(down(current)).type != Tile::Obstacle) {
//                neighbors.push(down(current));
//            }
//            if ((*this)(left(current)).type != Tile::Obstacle) {
//                neighbors.push(left(current));
//            }
            while (!neighbors.empty()) {
                T neighbor = neighbors.front();
                neighbors.pop();
                if (interior.find(neighbor) == interior.end()) {
                    cost[neighbor] = cost[current] + 1;
                    frontier.push(neighbor);
                    interior.insert(neighbor);
                    (*this)(neighbor).source_displacement = cost[neighbor];
                }
            }
        }
    }
    
    {
        std::queue<T>       frontier;
        std::unordered_set<T>         interior;
        std::unordered_map<T, size_t> cost;
        for (Maze::position source : crates_ending_pos) {
            frontier.push(source);
            interior.insert(source);
            cost[source] = 0;
            (*this)(source).target_displacement = 0;
        }
        
        while (!frontier.empty()) {
            T current = frontier.front();
            frontier.pop();
    //        if (interior.find(current) != interior.end()) continue;
    //        interior.insert(current);
            std::queue<T> neighbors;
            if ((*this)(up(current)).type != Tile::Obstacle && 
                        (*this)(up(up(current))).type != Tile::Obstacle) {
                neighbors.push(up(current));
            }
            if ((*this)(right(current)).type != Tile::Obstacle && 
                        (*this)(right(right(current))).type != Tile::Obstacle) {
                neighbors.push(right(current));
            }
            if ((*this)(down(current)).type != Tile::Obstacle && 
                        (*this)(down(down(current))).type != Tile::Obstacle) {
                neighbors.push(down(current));
            }
            if ((*this)(left(current)).type != Tile::Obstacle && 
                        (*this)(left(left(current))).type != Tile::Obstacle) {
                neighbors.push(left(current));
            }
            while (!neighbors.empty()) {
                T neighbor = neighbors.front();
                neighbors.pop();
                if (interior.find(neighbor) == interior.end()) {
                    cost[neighbor] = cost[current] + 1;
                    frontier.push(neighbor);
                    interior.insert(neighbor);
                    (*this)(neighbor).target_displacement = cost[neighbor];
                }
            }
        }
    }
}

void Maze::set_player_starting_pos(Maze::position const& pos) {
    player_starting_pos = pos;
}

Maze::position const& Maze::get_player_starting_pos() const {
    return player_starting_pos;
}

void Maze::add_crates_starting_pos(Maze::position const& pos) {
    crates_starting_pos.push_back(pos);
}

std::vector<Maze::position> const& Maze::get_crates_starting_pos() const {
    return crates_starting_pos;
}

void Maze::add_crates_ending_pos(Maze::position const& pos) {
    crates_ending_pos.push_back(pos);
}

std::vector<Maze::position> const& Maze::get_crates_ending_pos() const {
    return crates_ending_pos;
}

