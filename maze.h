
#ifndef DD2380_PROJECT_MAZE_H
#define DD2380_PROJECT_MAZE_H

#include <cstdlib>
#include <iostream>
#include <limits>
#include <queue>
#include <vector>

class Maze {
public:
    typedef std::pair<std::size_t, std::size_t> position;
    /*
    class position: public std::pair<std::size_t, std::size_t> {
    public:
        position(size_t x, size_t y):
            std::pair<std::size_t, std::size_t>(x, y)
        {}
        position():
            std::pair<std::size_t, std::size_t>()
        {}
        bool operator == (Maze::position p2) const {
            return first == p2.first && second == p2.second;
        }
    };
    */
    
    class Tile {
    public:
        enum Type {
            Floor,
            Obstacle,
            Dest
        } type;
        void setType(Type value);
        
        std::size_t source_displacement;
        std::size_t target_displacement;
        
        Tile();
        inline bool isWalkable() const;
    private:
    	bool walkable;
    };
    
    position                player_starting_pos;
    std::vector<position>   crates_starting_pos;
    std::vector<position>   crates_ending_pos;
    
private:
    std::vector<std::vector<Tile> > tiles;
public:
    
    Maze();
    
    Tile const& operator () (position) const;
    Tile& operator () (position);
    Tile const& operator () (size_t, size_t) const;
    Tile& operator () (size_t, size_t);
    
    inline bool isTileWalkable(const position & pos) const;
    bool isWalkable(const Maze::position & pos, const std::vector<Maze::position> & obstacles) const;
    
    friend std::ostream& operator << (std::ostream&, Maze const&);
    
    size_t height() const;
    size_t width(size_t) const;
    void add_row(size_t);
    
    /**
     * Is pos on the board?
     */
    bool valid(position const& pos) const;
    
    /**
     * Determine all neighboring positions of pos on the
     * board, given obstacles.
     */
    void get_neighbors(std::queue<position>& output,
                       position const& pos,
                       std::vector<position> const& obstacles) const;
    
    /**
     * Find a path from source to target on the
     * board, given obstacles. Returns the path in
     * output from <em>target</em> to <em>source</em>
     * (because that's more logical) including the
     * source and target.
     */
    void find_path(position const& source,
                   position const& target,
                   std::vector<position> const& obstacles,
                   std::vector<position>& output) const;
    
    /**
     * Check if there's a path from source to target on the
     * board, given obstacles.
     */
    bool reachable(Maze::position const& source,
                   Maze::position const& target,
                   std::vector<Maze::position> const& obstacles) const;
    
    void calculate_displacement_mapping();
    
    void                  set_player_starting_pos(Maze::position const&);
    
    Maze::position const& get_player_starting_pos() const;
    
    void                  add_crates_starting_pos(Maze::position const&);
    std::vector<Maze::position> const& get_crates_starting_pos() const;
    
    void                  add_crates_ending_pos(Maze::position const&);
    std::vector<Maze::position> const& get_crates_ending_pos() const;
};

namespace std {
    template <>
    class hash<Maze::position> {
    public:
        size_t operator () (Maze::position const& pos) const {
            return std::hash<size_t>()(200*pos.first + pos.second);
        }
    };
}

inline Maze::position operator - (Maze::position p1, Maze::position p2) {
    return Maze::position(p1.first - p2.first, p1.second - p2.second);
}

inline Maze::position operator + (Maze::position p1, Maze::position p2) {
    return Maze::position(p1.first + p2.first, p1.second + p2.second);
}

inline std::ostream& operator << (std::ostream& stream, Maze::position pos) {
    stream << "(" << pos.first << ", " << pos.second << ")";
    return stream;
}

inline std::ostream& operator << (std::ostream& stream, Maze const& maze) {
    for (size_t y = 0; y < maze.tiles.size(); y++) {
        for (size_t x = 0; x < maze.tiles[y].size(); x++) {
            if (maze.tiles[y][x].type != Maze::Tile::Obstacle) {
                if (maze.tiles[y][x].target_displacement == std::numeric_limits<size_t>::max())
                    stream << 'X';
                else
                    stream << maze.tiles[y][x].target_displacement % 10;
            } else {
                stream << '#';
            }
            
        }
        stream << std::endl;
    }
    return stream;
}

#endif
