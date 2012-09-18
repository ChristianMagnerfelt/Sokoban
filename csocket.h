#ifndef _SOKO_CSOCKET_H_
#define _SOKO_CSOCKET_H_

#include <stdint.h>
#include <stdexcept>
#include "ctime.h"

namespace soko {

class CSocket
{
public:
    static const int cBufSize=4096;

    CSocket()
        :   mFD(-1)
        ,	mBufLen(0)
        ,	mCheckLen(0)
    {}

    CSocket(int pFD) 
        :   mFD(pFD)
        ,	mBufLen(0)
        ,	mCheckLen(0)
    {}

    CSocket(const std::string &pHost,const std::string &pPort)
        :   mFD(-1)
        ,	mBufLen(0)
        ,	mCheckLen(0)
    {
        Init(pHost,pPort);
    }

    ~CSocket()
    {
        Close();
    }

    void Init(int pFD)
    {
        if(mFD!=-1) throw std::logic_error("socket already initialized");
        mFD=pFD;
    }
    
    void Init(const std::string &pHost,const std::string &pPort);
    void Close();

    void Buffer();

    void WriteLine(const std::string &pString);
    bool ReadLine(std::string &pString,bool pBlock=true);

    int GetFD()		{	return mFD;		}

private:
    bool CheckLine(std::string &pString);

private:
    int mFD;
    char mBuffer[cBufSize];
    int mBufLen;
    int mCheckLen;
};

/*namespace soko*/ }

#endif
