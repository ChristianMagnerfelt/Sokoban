
#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <map>
#include <memory>
#include <queue>
#include <set>
#include <vector>
#include "csocket.h"
#include "maze.h"

class Node {
    class Implementation {
    public:
        Maze                        const& maze;
        Maze::position              player_starting_pos;
        std::vector<Maze::position> crates_starting_pos;
        std::vector<Maze::position> crates_ending_pos;
        std::string                 path;
        
        size_t source_displacement;
        bool source_displacement_calculated;
        
        Implementation(Maze                        const& maze,
                       Maze::position              const& player_starting_pos,
                       std::vector<Maze::position> const& crates_pos):
            maze(maze),
            player_starting_pos(player_starting_pos),
            crates_starting_pos(crates_pos),
            crates_ending_pos(crates_pos),
            source_displacement(0),
            source_displacement_calculated(false)
        {}
        Implementation(Node                        const& node,
                       Maze::position              const& player_starting_pos,
                       std::vector<Maze::position> const& crates_starting_pos,
                       std::vector<Maze::position> const& crates_ending_pos,
                       std::string                 const& path):
            maze(node.implementation->maze),
            player_starting_pos(player_starting_pos),
            crates_starting_pos(crates_starting_pos),
            crates_ending_pos(crates_ending_pos),
            path(path),
            source_displacement(0),
            source_displacement_calculated(false)
        {}
    };
    std::shared_ptr<Implementation> implementation;
public:
    friend std::ostream& operator << (std::ostream&, Node const&);
    Node(): implementation(NULL) {}
    Node(Maze                        const& maze,
         Maze::position              const& player_starting_pos,
         std::vector<Maze::position> const& crates_starting_pos):
        implementation(new Implementation(maze, player_starting_pos, crates_starting_pos))
    {}
    Node(Node                        const& node,
         Maze::position              const& player_starting_pos,
         std::vector<Maze::position> const& crates_starting_pos,
         std::vector<Maze::position> const& crates_ending_pos,
         std::string                 const& path):
        implementation(new Implementation(node, player_starting_pos, crates_starting_pos, crates_ending_pos, path))
    {}
    bool is_source() const {
        if (!implementation) return false;
        for (Maze::position const& p1 : implementation->maze.crates_starting_pos) {
            bool match = false;
            for (Maze::position const& p2 : implementation->crates_starting_pos) {
                if (p1 == p2) {
                    match = true;
                    break;
                }
            }
            if (!match) return false;
        }
        return implementation->maze.reachable(
            implementation->maze.get_player_starting_pos(),
            get_player_starting_pos(), implementation->crates_starting_pos);
    }
    bool is_target() const {
        if (!implementation) return false;
        for (Maze::position const& p1 : implementation->maze.crates_ending_pos) {
            bool match = false;
            for (Maze::position const& p2 : implementation->crates_starting_pos) {
                if (p1 == p2) {
                    match = true;
                    break;
                }
            }
            if (!match) return false;
        }
        return true;
    }
    void get_predecessors(std::queue<Node>& nodes) const {
        typedef Maze::position (*Dir) (Maze::position const&);
        static const Dir dirs[4] = { up, right, down, left };
        static const std::string codes[4] = { "D", "L", "U", "R" };
//        mark_reachable();
        for (size_t i = 0; i < implementation->crates_starting_pos.size(); i++) {
            for (size_t j = 0; j < 4; j++) {
                Dir dir = dirs[j];
                std::string code = codes[j];
                Maze::position crate = implementation->crates_starting_pos[i];
                Maze::position player = (*dir)(crate);
                if (implementation->maze.walkable(player, implementation->crates_starting_pos)
                 && implementation->maze.reachable(player, get_player_starting_pos(), implementation->crates_starting_pos)) {
//                if (walkable(player) && is_marked(player)) {
                    std::vector<Maze::position> new_crates_starting_pos
                                                    = implementation->crates_starting_pos;
                    player = (*dir)(player);
                    crate  = (*dir)(crate);
                    std::string path = code;
                    while (implementation->maze.walkable(player, implementation->crates_starting_pos)) {
                        new_crates_starting_pos[i] = crate;
                        nodes.push(Node(*this, player, new_crates_starting_pos, implementation->crates_starting_pos, path));
                        player = (*dir)(player);
                        crate  = (*dir)(crate);
                        path += code;
                    }
                }
            }
        }
    }
    void get_successors(std::queue<Node>& nodes) const {
        typedef Maze::position (*Dir) (Maze::position const&);
        static const Dir dirs[4] = { up, right, down, left };
        static const std::string codes[4] = { "U", "R", "D", "L" };
        for (size_t i = 0; i < implementation->crates_ending_pos.size(); i++) {
            for (size_t j = 0; j < 4; j++) {
                Dir dir = dirs[j];
                std::string code = codes[j];
                Maze::position crate = implementation->crates_ending_pos[i];
                Maze::position player;
                if (j == 0)
                    player = down(crate);
                else if (j == 1)
                    player = left(crate);
                else if (j == 2)
                    player = up(crate);
                else if (j == 3)
                    player = right(crate);
                Maze::position player_start_pos = player;
                if (implementation->maze.walkable(player, implementation->crates_ending_pos)
                 && implementation->maze.reachable(player, get_player_ending_pos(), implementation->crates_ending_pos)) {
                    std::vector<Maze::position> new_crates_ending_pos
                                                = implementation->crates_ending_pos;
                    player = (*dir)(player);
                    crate  = (*dir)(crate);
                    std::string path = code;
                    while (implementation->maze.walkable(crate, implementation->crates_ending_pos)) {
                        new_crates_ending_pos[i] = crate;
                        if ((implementation->maze(up(crate)).type == Maze::Tile::Obstacle && implementation->maze(right(crate)).type == Maze::Tile::Obstacle)
                         || (implementation->maze(right(crate)).type == Maze::Tile::Obstacle && implementation->maze(down(crate)).type == Maze::Tile::Obstacle)
                         || (implementation->maze(down(crate)).type == Maze::Tile::Obstacle && implementation->maze(left(crate)).type == Maze::Tile::Obstacle)
                         || (implementation->maze(left(crate)).type == Maze::Tile::Obstacle && implementation->maze(up(crate)).type == Maze::Tile::Obstacle)) {
                        } else {
                            nodes.push(Node(*this, player_start_pos, implementation->crates_ending_pos, new_crates_ending_pos, path));
                        }
                        player = (*dir)(player);
                        crate  = (*dir)(crate);
                        path += code;
                    }
                }
            }
        }
    }
    bool operator < (Node const& other) const {
        {
            std::vector<Maze::position> c1 = implementation->crates_starting_pos;
            std::vector<Maze::position> c2 = other.implementation->crates_starting_pos;
            std::sort(c1.begin(), c1.end());
            std::sort(c2.begin(), c2.end());
            
            for (size_t i = 0; i < c1.size(); i++) {
                if (c1[i] != c2[i]) return c1[i] < c2[i];
            }
        }
        {
            std::vector<Maze::position> c1 = implementation->crates_ending_pos;
            std::vector<Maze::position> c2 = other.implementation->crates_ending_pos;
            std::sort(c1.begin(), c1.end());
            std::sort(c2.begin(), c2.end());
            
            for (size_t i = 0; i < c1.size(); i++) {
                if (c1[i] != c2[i]) return c1[i] < c2[i];
            }
        }
        
        if (implementation->maze.reachable(get_player_starting_pos(),
                      other.get_player_starting_pos(),
                      implementation->crates_starting_pos)) return false;
        if (get_player_starting_pos() != other.get_player_starting_pos())
            return get_player_starting_pos() < other.get_player_starting_pos();
        return get_path() < other.get_path();
    }
    operator bool () const {
        return implementation != NULL;
    }
    Maze::position get_player_starting_pos() const {
        return implementation->player_starting_pos;
    }
    Maze::position get_player_ending_pos() const {
        Maze::position current_pos = get_player_starting_pos();
        for (char c : get_path()) {
            if (c == 'U') {
                current_pos = up(current_pos);
            } else if (c == 'R') {
                current_pos = right(current_pos);
            } else if (c == 'D') {
                current_pos = down(current_pos);
            } else if (c == 'L') {
                current_pos = left(current_pos);
            }
        }
        return current_pos;
    }
    std::vector<Maze::position> const& get_crates_starting_pos() const {
        return implementation->crates_starting_pos;
    }
    std::vector<Maze::position> const& get_crates_ending_pos() const {
        return implementation->crates_ending_pos;
    }
    std::string const& get_path() const {
        return implementation->path;
    }
    size_t source_displacement(Maze::position const& source) const {
        
        typedef Maze::position T;
        
        if (std::find(implementation->maze.crates_starting_pos.begin(),
                      implementation->maze.crates_starting_pos.end(),
                      source)
                    != implementation->maze.crates_starting_pos.end()) {
            return 0;
        }
        
        std::queue<T>       frontier;
        std::set<T>         interior;
        std::map<T, size_t> cost;
        frontier.push(source);
        cost[source] = 0;
        
        while (!frontier.empty()) {
            T current = frontier.front();
            frontier.pop();
            if (interior.find(current) != interior.end()) continue;
            interior.insert(current);
            std::queue<T> neighbors;
//            implementation->maze.get_neighbors(neighbors,
//                                               current,
//                                               implementation->crates_starting_pos);
            implementation->maze.get_neighbors(neighbors,
                                               current,
                                               std::vector<Maze::position>());
            while (!neighbors.empty()) {
                T neighbor = neighbors.front();
                neighbors.pop();
                if (interior.find(neighbor) == interior.end()) {
                    cost[neighbor] = cost[current] + 1;
                    if (std::find(implementation->maze.crates_starting_pos.begin(),
                                  implementation->maze.crates_starting_pos.end(),
                                  source)
                                != implementation->maze.crates_starting_pos.end()) {
                        return cost[neighbor];
                    }
                    frontier.push(neighbor);
                }
            }
        }
        
        return std::numeric_limits<size_t>::max();
//        return 0;
//        return -1;
    }
    size_t source_displacement() const {
        if (implementation->source_displacement_calculated)
            return implementation->source_displacement;
        implementation->source_displacement_calculated = true;
        size_t value = 0;
        for (Maze::position const& pos : implementation->crates_starting_pos) {
            size_t c = implementation->maze(pos).source_displacement;
//            size_t c = source_displacement(pos);
            if (c == std::numeric_limits<size_t>::max()) {
                value = c;
                break;
            } else
                value += c;
        }
        return implementation->source_displacement = value;
    }
    size_t target_displacement() const {
//        if (implementation->source_displacement_calculated)
//            return implementation->source_displacement;
//        implementation->source_displacement_calculated = true;
        size_t value = 0;
        for (Maze::position const& pos : implementation->crates_starting_pos) {
            size_t c = implementation->maze(pos).target_displacement;
            if (c == std::numeric_limits<size_t>::max()) {
                value = c;
                break;
            } else
                value += c;
        }
        return value;
//        return implementation->source_displacement = value;
    }
//    size_t source_displacement() const {
//        if (implementation->source_displacement_calculated)
//            return implementation->source_displacement;
//        if (is_source()) return 0;
//        implementation->source_displacement_calculated = true;
//        size_t value = std::numeric_limits<size_t>::max();
//        for (Maze::position const& pos : implementation->crates_starting_pos) {
//            size_t c = source_displacement(pos);
//            if (c != 0 && c < value) value = c;
//        }
//        return implementation->source_displacement = value;
//    }
//    size_t source_displacement() const {
//        if (is_source()) return 0;
//        if (implementation->source_displacement_calculated)
//            return implementation->source_displacement;
//        implementation->source_displacement_calculated = true;
//        std::vector<int> disps;
//        for (Maze::position const& pos : implementation->crates_starting_pos) {
//            disps.push_back(source_displacement(pos));
//        }
////        std::sort(disps.begin(), disps.end(), std::greater<int>());
//        std::sort(disps.begin(), disps.end(), std::less<int>());
//        int value = 0;
//        size_t factor = 1;
//        static double breadth = 0;
////        breadth = 1 - 0.000001*(breadth - 1);
//        if (breadth < 1) breadth += 0.000001;
//        else breadth = 1;
//        for (int disp : disps) {
//            value += disp / pow(2, breadth * factor);
//            factor++;
//        }
//        return implementation->source_displacement = value;
//    }
//    size_t source_displacement() const {
//        if (is_source()) return 0;
//        if (implementation->source_displacement_calculated)
//            return implementation->source_displacement;
//        implementation->source_displacement_calculated = true;
//        size_t value = rand();
//        return implementation->source_displacement = value;
//    }
};

inline std::ostream& operator << (std::ostream& stream, Node const& node) {
    for (size_t y = 0; y < node.implementation->maze.height(); y++) {
        for (size_t x = 0; x < node.implementation->maze.width(y); x++) {
            bool is_crate = false;
            for (Maze::position const& pos : node.implementation->crates_ending_pos) {
                if (Maze::position(x, y) == pos) {
                    is_crate = true;
                    break;
                }
            }
            bool is_player = Maze::position(x, y)
                                    == node.get_player_ending_pos();
            if (node.implementation->maze(x, y).type == Maze::Tile::Floor) {
                if (is_player)
                    stream << '@';
                else if (is_crate)
                    stream << '$';
                else
                    stream << ' ';
            } else if (node.implementation->maze(x, y).type == Maze::Tile::Obstacle) {
                stream << '#';
            } else if (node.implementation->maze(x, y).type == Maze::Tile::Dest) {
                if (is_player)
                    stream << '+';
                else if (is_crate)
                    stream << '*';
                else
                    stream << '.';
            }
        }
        stream << std::endl;
    }
    return stream;
    
}

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
    std::set<Node>                          interior;
    std::map<Node, Node>                    previous;
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
    std::set<Node>          interior;
    std::map<Node, Node>    previous;
    for (Node const& root : root_nodes) {
        frontier.push(root);
//        interior.insert(root);
        
//        std::queue<Node> succ;
//        root.get_successors(succ);
//        Node node;
//        while (!succ.empty()) {
//            node = succ.front();
//            succ.pop();
//            frontier.push(node);
//            previous[node] = root;
//        }
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
    
    // Generate maze
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
    
//    std::cout << maze << std::endl;
    
    if (!player_in_maze) {
        // Handle
    }
    
    // Assume crates_starting_pos.size() >= dest_pos.size()
    
//    if (crates_starting_pos.size() != dest_pos.size()) {
//        // Handle
//    }
    
    // if ( multiple islands) Handle
    
//    Node root(maze, safe_player_pos,
//                    maze.get_crates_ending_pos());
//    std::cout << root << std::endl;
//    std::cout << "Source: " << root.is_source() << std::endl;
//    std::vector<Node> prev;
//    root.get_predecessors(prev);
//    for (Node const& node : prev) {
//        std::cout << node << std::endl;
//        std::cout << "Source: " << node.is_source() << std::endl;
//    }
    
//    std::cout << "From: " << maze.get_crates_starting_pos().size() << std::endl;
//    std::cout << "To: " << maze.get_crates_ending_pos().size() << std::endl;
//    
    std::vector<Node> terminal_nodes;
//    if (maze.get_crates_ending_pos().size() == maze.get_crates_starting_pos().size()) {
        for (Maze::position const& pos : player_ending_pos) {
//            if (maze(pos).source_displacement != std::numeric_limits<size_t>::max())
                terminal_nodes.push_back(Node(maze, pos, maze.get_crates_ending_pos()));
        }
//    } else if (maze.get_crates_ending_pos().size() == maze.get_crates_starting_pos().size() + 1) {
//        std::vector<Maze::position> temp_1 = maze.get_crates_ending_pos();
//        for (size_t i = 0; i < temp_1.size(); i++) {
//            std::vector<Maze::position> temp_2 = temp_1;
//            temp_2.erase(temp_2.begin() + i);
//            for (Maze::position const& pos : player_ending_pos) {
//                terminal_nodes.push_back(Node(maze, pos, temp_2));
//            }
//        }
//    } else {
//        assert(false);
//    }
    
    std::vector<Node> initial_nodes;
    initial_nodes.push_back(Node(maze, maze.get_player_starting_pos(),
                                       maze.get_crates_starting_pos()));
    Node root(maze, maze.get_player_starting_pos(),
                                    maze.get_crates_starting_pos());
//    std::queue<Node> succ;
//    root.get_successors(succ);
//    Node node;
//    while (!succ.empty()) {
//        node = succ.front();
//        succ.pop();
//        std::cout << node << std::endl;
//    }
    std::vector<Node> steps;
    reverse_best_first_search(maze, terminal_nodes, steps);
//    forward_best_first_search(maze, initial_nodes, steps);
//    bidirectional_search(maze, initial_nodes, terminal_nodes, steps);
    
    std::string solution_string = "";
    Maze::position current_pos = maze.get_player_starting_pos();
//    for (Node const& node : steps) {
//        std::cout << node << std::endl;
////        std::cout << node.get_player_pos() << " to " << current_pos << std::endl;
//        std::vector<Maze::position> path;
//        auto const& dest = node.get_player_starting_pos();
//        auto const& crates = node.get_crates_starting_pos();
//        maze.find_path(dest, current_pos, crates, path);
//        
//        solution_string += path_to_string(path);
//        current_pos = node.get_player_ending_pos();
//        solution_string += " " + node.get_path() + " ";
//    }
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

