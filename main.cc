#include <iostream>
#include <cstdlib>
#include "csocket.h"

int main(int pArgC, char* pArgs[])
{
    if(pArgC<4)
    {
        std::cerr << "usage: " << pArgs[0] << " host port boardnum" << std::endl;
        return -1;
    }

    soko::CSocket lSocket(pArgs[1],pArgs[2]);

    lSocket.WriteLine(pArgs[3]);
    
    std::string lLine;

    //read number of rows
    lSocket.ReadLine(lLine);
    int lNumRows=atoi(lLine.c_str());
    
    //read each row
    for(int i=0;i<lNumRows;i++)
    {
        lSocket.ReadLine(lLine);
        //here, we would store the row somewhere, to build our board
        //in this demo, we just print it
        std::cout << lLine << "\n";
    }
    
    //now, we should find a path from the player to any goal

    //we've found our solution    
	std::string lMySol("U R R U U L D L L U L L D R R R R L D D R U R U D L L U R");
	//these formats are also valid:
	//std::string lMySol("URRUULDLLULLDRRRRLDDRURUDLLUR");
    //std::string lMySol("0 3 3 0 0 2 1 2 2 0 2 2 1 3 3 3 3 2 1 1 3 0 3 0 1 2 2 0 3");

    //send the solution to the server
    lSocket.WriteLine(lMySol);
    
    //read answer from the server
    lSocket.ReadLine(lLine);
    
    std::cout << lLine << std::endl;

    return 0;
}

