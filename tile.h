#ifndef DD2380_TILE_H
#define DD2380_TILE_H

#include <utility>

class Tile
{
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
        inline bool isWalkable() const { return walkable; }
    private:
    	bool walkable;
};
    
#endif
