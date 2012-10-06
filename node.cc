
#include <sstream>

#include "node.h"

class Node::Implementation {
public:
    Maze                        const& maze;
    Maze::position              player_starting_pos;
    std::vector<Maze::position> crates_starting_pos;
    std::vector<Maze::position> crates_ending_pos;
    std::string                 path;
    
    Implementation(Maze                        const& maze,
                   Maze::position              const& player_starting_pos,
                   std::vector<Maze::position> const& crates_pos):
        maze(maze),
        player_starting_pos(player_starting_pos),
        crates_starting_pos(crates_pos),
        crates_ending_pos(crates_pos)
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
        path(path)
    {}
};

Node::Node():
    implementation(NULL)
{}

Node::Node(Maze                  const& maze,
     Maze::position              const& player_starting_pos,
     std::vector<Maze::position> const& crates_starting_pos):
    implementation(new Implementation(maze, player_starting_pos, crates_starting_pos))
{}

Node::Node(Node                  const& node,
     Maze::position              const& player_starting_pos,
     std::vector<Maze::position> const& crates_starting_pos,
     std::vector<Maze::position> const& crates_ending_pos,
     std::string                 const& path):
    implementation(new Implementation(node, player_starting_pos, crates_starting_pos,
                                            crates_ending_pos, path))
{}

bool Node::is_source() const {
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

bool Node::is_target() const {
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

void Node::get_predecessors(std::queue<Node>& nodes) const {
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
            if (implementation->maze.isWalkable(player, implementation->crates_starting_pos)
             && implementation->maze.reachable(player, get_player_starting_pos(), implementation->crates_starting_pos)) {
//                if (walkable(player) && is_marked(player)) {
                std::vector<Maze::position> new_crates_starting_pos
                                                = implementation->crates_starting_pos;
                player = (*dir)(player);
                crate  = (*dir)(crate);
                std::string path = code;
                while (implementation->maze.isWalkable(player, implementation->crates_starting_pos)) {
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
void Node::get_successors(std::queue<Node>& nodes) const {
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
            if (implementation->maze.isWalkable(player, implementation->crates_ending_pos)
             && implementation->maze.reachable(player, get_player_ending_pos(), implementation->crates_ending_pos)) {
                std::vector<Maze::position> new_crates_ending_pos
                                            = implementation->crates_ending_pos;
                player = (*dir)(player);
                crate  = (*dir)(crate);
                std::string path = code;
                while (implementation->maze.isWalkable(crate, implementation->crates_ending_pos)) {
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

bool Node::operator < (Node const& other) const {
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

bool Node::operator == (Node const& other) const {
    {
        std::vector<Maze::position> c1 = implementation->crates_starting_pos;
        std::vector<Maze::position> c2 = other.implementation->crates_starting_pos;
        std::sort(c1.begin(), c1.end());
        std::sort(c2.begin(), c2.end());
        
        for (size_t i = 0; i < c1.size(); i++) {
            if (c1[i] != c2[i]) return false;
        }
    }
    {
        std::vector<Maze::position> c1 = implementation->crates_ending_pos;
        std::vector<Maze::position> c2 = other.implementation->crates_ending_pos;
        std::sort(c1.begin(), c1.end());
        std::sort(c2.begin(), c2.end());
        
        for (size_t i = 0; i < c1.size(); i++) {
            if (c1[i] != c2[i]) return false;
        }
    }
    
    return get_path() == other.get_path();
}

Node::operator bool () const {
    return implementation != NULL;
}

size_t Node::hash () const {
    std::stringstream hash_string;
    {
        std::vector<Maze::position> c = implementation->crates_starting_pos;
        std::sort(c.begin(), c.end());
        
        for (Maze::position const& pos : c) hash_string << pos;
    }
    {
        std::vector<Maze::position> c = implementation->crates_ending_pos;
        std::sort(c.begin(), c.end());
        
        for (Maze::position const& pos : c) hash_string << pos;
    }
    return std::hash<std::string>()(hash_string.str());
}

Maze::position Node::get_player_starting_pos() const {
    return implementation->player_starting_pos;
}

Maze::position Node::get_player_ending_pos() const {
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

std::vector<Maze::position> const& Node::get_crates_starting_pos() const {
    return implementation->crates_starting_pos;
}

std::vector<Maze::position> const& Node::get_crates_ending_pos() const {
    return implementation->crates_ending_pos;
}

std::string const& Node::get_path() const {
    return implementation->path;
}

size_t Node::source_displacement() const {
    size_t value = 0;
    for (Maze::position const& pos : implementation->crates_starting_pos) {
        size_t c = implementation->maze(pos).source_displacement;
        if (c == std::numeric_limits<size_t>::max()) {
            value = c;
            break;
        } else
            value += c;
    }
    return value;
}

size_t Node::target_displacement() const {
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
}

std::ostream& operator << (std::ostream& stream, Node const& node) {
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

