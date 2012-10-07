
#ifndef DD2380_PROJECT_NODE_H
#define DD2380_PROJECT_NODE_H

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

/**
 * Represents a node in the direction-agnostic
 * sokoban problem search tree. That is to
 * say, the Node class can be used for nodes in
 * the search tree of both the forward and reverse
 * problem.
 * In order to do this, the node class must store
 * enough information to deduce:
 * - The player position before the move
 * - The player position after the move
 * - The crate positions before the move
 * - The crate positions after the move
 * - The path of the player during the move
 */
class Node {
    /**
     * Pimpl interior class for the nodes.
     * shared_ptr gives <em>aliasing</em> semantics.
     */
    class Implementation;
    std::shared_ptr<Implementation> implementation;
public:
    friend std::ostream& operator << (std::ostream&, Node const&);
    
    /**
     * Construct null node.
     */
    Node();
    
    /**
     * Construct root node (either initial or terminal).
     * Unlike real nodes, root doesn't describe the movement
     * of a crate. In the forward direction, the root node specifies
     * the initial board <em>state</em>. In the reverse direction
     * the root nodes specify the potential final board states
     * (i.e. where the player may end up).
     */
    Node(Maze                        const& maze,
         Maze::position              const& player_starting_pos,
         std::vector<Maze::position> const& crates_starting_pos);
private:
    /**
     * Extend node into a new node.
     */
    Node(Node                        const& node,
         Maze::position              const& player_starting_pos,
         std::vector<Maze::position> const& crates_starting_pos,
         std::vector<Maze::position> const& crates_ending_pos,
         std::string                 const& path);
public:
    /**
     * Check if all the crates are in an initial configuration,
     * including permutations, and that the player can reach the
     * player starting position.
     */
    bool is_source() const;
    /**
     * Check if all the crates are in a final configuration,
     * including permutations.
     */
    bool is_target() const;
    /**
     * Get all legal previous crate movements.
     * That is, all legal movements of a single crate after
     * which the movement described by this node is legal.
     */
    void get_predecessors(std::vector<Node>& nodes) const;
    /**
     * Get all legal next crate movements.
     */
    void get_successors(std::vector<Node>& nodes) const;
    /**
     * Get all legal previous crate movements.
     * That is, all legal movements of a single crate after
     * which the movement described by this node is legal.
     */
    void get_predecessors(std::queue<Node>& nodes) const;
    /**
     * Get all legal next crate movements.
     */
    void get_successors(std::queue<Node>& nodes) const;    
    /**
     * For std::set and std::map. Makes no semantic sense
     * whatsoever.
     */
    bool operator < (Node const& other) const;
    /**
     * For std::unordered_set and std::unordered_map.
     */
    bool operator == (Node const& other) const;
    /**
     * Is this node not null?
     */
    operator bool () const;
    
    size_t hash() const;
    
    Node const& get_parent() const;
    
    Maze::position get_player_starting_pos() const;
    Maze::position get_player_ending_pos() const;
    std::vector<Maze::position> const& get_crates_starting_pos() const;
    std::vector<Maze::position> const& get_crates_ending_pos() const;
    std::string const& get_path() const;
    
    size_t source_displacement() const;
    size_t target_displacement() const;
};

namespace std {
    template <>
    class hash<Node> {
    public:
        size_t operator () (Node const& node) const {
            return node.hash();
        }
    };
}

std::ostream& operator << (std::ostream& stream, Node const& node);

#endif

