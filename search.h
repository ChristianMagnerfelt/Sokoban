#ifndef DD2380_SEARCH_H
#define DD2380_SEARCH_H

#include "node.h"

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

#endif
