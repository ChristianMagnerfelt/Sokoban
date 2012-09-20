#include <iostream>
#include <cstdlib>
#include "csocket.h"
#include "board.h"
#include "player.h"

int main(int pArgC, char* pArgs[])
{
	if(pArgC<4)
	{
		std::cerr << "usage: " << pArgs[0] << " host port boardnum" << std::endl;
		return -1;
	}

	soko::CSocket lSocket(pArgs[1],pArgs[2]);
	lSocket.WriteLine(pArgs[3]);
    
	Board startBoard;
	Player player;

	// Read number of rows
	std::string lNumRows;
	lSocket.ReadLine(lNumRows);
	int numRows = atoi(lNumRows.c_str());
    
	// Read each row
	for(int i = 0; i < numRows; ++i)
	{
		std::string lLine;
		lSocket.ReadLine(lLine);
		startBoard.addRowToBoard(std::move(lLine));
	}
	startBoard.initialize();
	//startBoard.printBoard();
	//startBoard.printIDMatrix();
	//startBoard.printPlayerPosition();
	//startBoard.printBoxes();
	//startBoard.printGoals();

	player.start(startBoard);
	startBoard.printVisitedMatrix();

	//send the solution to the server
	//lSocket.WriteLine(lMySol);
    
	//read answer from the server
	//lSocket.ReadLine(lLine);

	return 0;
}

