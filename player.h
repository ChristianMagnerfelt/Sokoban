#pragma once 

#ifndef PLAYER_H
#define PLAYER_H

#include "board.h"

class Player
{
	public:
		void findPossiblePushes();
		void findAllPaths();
		void pushBox(Board &);
		void start(Board &);

	private:
		
};

#endif
