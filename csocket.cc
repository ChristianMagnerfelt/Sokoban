//  -*- mode:c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4  -*-
//  (The above are formatting setting for Emacs)

#include "csocket.h"

// ----------------- OS-dependent settings -------------------

#ifdef __APPLE__
#include "TargetConditionals.h"

#ifdef TARGET_OS_MAC

// -- MacOS X --

#include <sys/socket.h>
#include <netdb.h>

#define CSOCKET_SEND_OPTIONS (MSG_DONTWAIT)
#define CSOCKET_DO_NOSIGPIPE 1

// -- End MacOS X --

#else  // not TARGET_OS_MAC
// Apple, but not MacOS X
#error "Unsupported Apple platform"
#endif

#elif defined _WIN32 || defined _WIN64

// -- Windows specific --

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#define _WIN32_WINNT 0x0501

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>

// Set the socket in blocking or non-bloking mode
// iMode != 0 => non-blocking
void SetNonBlocking(int mFD, u_long iMode)
{
    ioctlsocket(mFD,FIONBIO,&iMode);
}

#define CSOCKET_SEND_OPTIONS 0
#undef CSOCKET_DO_NOSIGPIPE

// -- End Windows specific --

#elif __linux__

// -- Linux specific --

#include <sys/socket.h>
#include <netdb.h>

#define CSOCKET_SEND_OPTIONS (MSG_DONTWAIT|MSG_NOSIGNAL)
#undef CSOCKET_DO_NOSIGPIPE

// -- End Linux specific --

#else
#error "Unsupported platform"
#endif

#ifndef CSOCKET_SEND_OPTIONS
#error "CSOCKET_SEND_OPTIONS must be defined, check OS-dependent settings!"
#endif

// ------------- End OS-dependent settings -------------------


#include <time.h>
#include <string.h>
#include <errno.h>

namespace soko {

void CSocket::Init(const std::string &pHost,const std::string &pPort)
{
    if(mFD!=-1) throw std::logic_error("socket already initialized");

    struct addrinfo lHints,*lResult;

#ifdef _WIN32
    WSADATA wsaData;
    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
        throw std::runtime_error("WSAStartup failed");
    ZeroMemory( &lHints, sizeof(lHints) );
    lHints.ai_protocol = IPPROTO_TCP;        
#else                    
    memset(&lHints,0,sizeof(lHints));
    lHints.ai_flags=AI_NUMERICSERV;
    lHints.ai_protocol=0;
#endif
    lHints.ai_family=AF_UNSPEC;
    lHints.ai_socktype=SOCK_STREAM;
    
    if(getaddrinfo(pHost.c_str(),pPort.c_str(),&lHints,&lResult)!=0)
        throw std::runtime_error("host lookup failed");
    
    for(struct addrinfo *lR=lResult;lR;lR=lR->ai_next)
    {
        mFD=socket(lR->ai_family,lR->ai_socktype,lR->ai_protocol);
        if(mFD==-1) continue;
        if(connect(mFD,lR->ai_addr,lR->ai_addrlen)==0)
            break;
#ifdef _WIN32
        closesocket(mFD);
#else
        close(mFD);
#endif
        mFD=-1;
    }
    
    freeaddrinfo(lResult);

    if(mFD==-1)
        throw std::runtime_error("can't connect to host");

#ifdef CSOCKET_DO_NOSIGPIPE
    {
        int on=1;
        setsockopt(mFD,SOL_SOCKET,SO_NOSIGPIPE,(void *)&on,sizeof(int));
    }
#endif

}

void CSocket::Close()
{
    if(mFD==-1) return;

#ifdef _WIN32
    closesocket(mFD);
    WSACleanup();
#else
    close(mFD);
#endif
    mFD=-1;
    mBufLen=0;
    mCheckLen=0;
}

void CSocket::WriteLine(const std::string &pLine)
{
    if(mFD==-1) throw std::runtime_error("trying to write to uninitialized socket");

    std::string lLine=pLine;
    if(lLine.empty()||lLine[lLine.size()-1]!='\n') lLine+='\n';

    int lWritten=0;
    
    while(lWritten<lLine.size())
    {
#ifdef _WIN32
        SetNonBlocking(mFD,1);
        int lRet=send(mFD,lLine.data()+lWritten,lLine.size()-lWritten,
                          CSOCKET_SEND_OPTIONS);
#else
        ssize_t lRet=send(mFD,lLine.data()+lWritten,lLine.size()-lWritten,
                          CSOCKET_SEND_OPTIONS);
#endif
        
        if(lRet==-1)
        {
            if(errno==EINTR)
                continue;

            throw std::runtime_error("can't write to socket");
        }
        
        lWritten-=lRet;
    }
}

bool CSocket::CheckLine(std::string &pLine)
{
    for(;mCheckLen<mBufLen;++mCheckLen)
    {
        if(mBuffer[mCheckLen]=='\n')
        {
            pLine.assign(mBuffer,mCheckLen++);
            if(!pLine.empty()&&pLine[pLine.size()-1]=='\r')
                pLine.resize(pLine.size()-1);
            if(mCheckLen<mBufLen)
                memmove(mBuffer,mBuffer+mCheckLen,mBufLen-mCheckLen);
            mBufLen-=mCheckLen;
            mCheckLen=0;
            return true;
        }
    }
    
    return false;
}

bool CSocket::ReadLine(std::string &pLine,bool pBlock)
{
    if(mFD==-1) throw std::runtime_error("trying to read from uninitialized socket");

    if(CheckLine(pLine)) return true;

#ifdef _WIN32
    SetNonBlocking(mFD,pBlock?0:1);
#endif

    while(true)
    {
        if(mBufLen==cBufSize) throw std::runtime_error("buffer full in CSocket::ReadLine");
    
#ifdef _WIN32
        int lRet=recv(mFD,mBuffer+mBufLen,cBufSize-mBufLen,0);
#else
        ssize_t lRet=recv(mFD,mBuffer+mBufLen,cBufSize-mBufLen,
                          pBlock?0:MSG_DONTWAIT);
#endif
        
        if(lRet==-1)
        {
#ifdef _WIN32
            if(WSAGetLastError()==WSAEINTR)
                continue;
            else if(WSAGetLastError() != WSAEWOULDBLOCK)
                throw std::runtime_error("can't read from socket");
#else
            if(errno==EINTR)
                continue;
            else if(errno!=EAGAIN)
                throw std::runtime_error("can't read from socket");
#endif
            else
                lRet=0;
        }
        else if(lRet==0)
            throw std::runtime_error("can't read from socket: EOF");

        mBufLen+=lRet;
        
        if(CheckLine(pLine)) return true;
        
        if(pBlock) continue;
        return false;
    }
}

void CSocket::Buffer()
{
    if(mBufLen==cBufSize) throw std::runtime_error("buffer full in CSocket::ReadLine");

#ifdef _WIN32
    SetNonBlocking(mFD, 0);
    int lRet=recv(mFD,mBuffer+mBufLen,cBufSize-mBufLen,0);
#else
    ssize_t lRet=recv(mFD,mBuffer+mBufLen,cBufSize-mBufLen,MSG_DONTWAIT);
#endif

    if(lRet==-1)
    {
        if(errno==EINTR)
            return;
        else if(errno!=EAGAIN)
            throw std::runtime_error("can't read from socket");
        else
            lRet=0;
    }
    else if(lRet==0)
        throw std::runtime_error("can't read from socket: EOF");
    mBufLen+=lRet;
}

/*namespace soko*/ }
