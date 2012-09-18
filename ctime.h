#ifndef _SOKO_CTIME_H_
#define _SOKO_CTIME_H_

#include <stdint.h>
#include <stdlib.h>

#ifdef _WIN32
#include <WinSock2.h>
#include <Windows.h>
#include <time.h>
#else
#include <sys/time.h>
#endif

#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif

#ifdef _WIN32
    static inline int gettimeofday(struct timeval *tv,void *tz)
    {
        // Define a structure to receive the current Windows filetime
        FILETIME ft;

        // Initialize the present time to 0 and the timezone to UTC
        unsigned __int64 tmpres = 0;
        static int tzflag = 0;

        if (NULL != tv)
        {
            GetSystemTimeAsFileTime(&ft);

            // The GetSystemTimeAsFileTime returns the number of 100 nanosecond
            // intervals since Jan 1, 1601 in a structure. Copy the high bits t
            // the 64 bit tmpres, shift it left by 32 then or in the low 32 bit
            tmpres |= ft.dwHighDateTime;
            tmpres <<= 32;
            tmpres |= ft.dwLowDateTime;

            // Convert to microseconds by dividing by 10
            tmpres /= 10;
            // The Unix epoch starts on Jan 1 1970.  Need to subtract the diffe
            // in seconds from Jan 1 1601.
            tmpres -= DELTA_EPOCH_IN_MICROSECS;

            // Finally change microseconds to seconds and place in the seconds 
            // The modulus picks up the microseconds.
            tv->tv_sec = (long)(tmpres / 1000000UL); 
            tv->tv_usec = (long)(tmpres % 1000000UL);
        }
		return 0;
	}
#endif

namespace soko {

///encapsulates a time
class CTime
{
public:
    ///initializes the time to \p pTime (in microseconds since the epoch)
    explicit CTime(int64_t pTime=-1)
        :   mTime(pTime)
    {
    }

    ///returns true if it is valid
    
    ///Times are invalid when they are default initialized.
    bool IsValid() const		{	return mTime!=-1;	}

    ///converts the time to a struct timeval
    void ToTimeval(struct timeval &pTime) const
    {
        pTime.tv_sec=mTime/1000000;
        pTime.tv_usec=mTime%1000000;
    }

    ///converts the time from this time to \p pUntil to a struct timeval
    void ToTimevalUntil(const CTime &pUntil,struct timeval &pDiff) const
    {
        int64_t lDiff=pUntil.mTime-mTime;
        if(lDiff<=0)
        {
            pDiff.tv_sec=0;
            pDiff.tv_usec=0;
        }
        else
        {
            pDiff.tv_sec=lDiff/1000000;
            pDiff.tv_usec=lDiff%1000000;
        }
    }

    ///returns a CTime object representing the current time
    static CTime GetCurrent()
    {
        struct timeval lTime;
        gettimeofday(&lTime,NULL);
        return CTime(int64_t(lTime.tv_sec)*1000000L+lTime.tv_usec);
    }
    
    ///returns the value of this time object in microseconds since the epoch
    int64_t Get() const	{	return mTime;	}
    
    ///comparison operator
    bool operator<(const CTime &pOther) const
    {
        return mTime<pOther.mTime;
    }
    
    ///comparison operator
    bool operator<=(const CTime &pOther) const
    {
        return mTime<=pOther.mTime;
    }
    
    ///comparison operator
    bool operator>(const CTime &pOther) const
    {
        return mTime>pOther.mTime;
    }
    
    ///comparison operator
    bool operator>=(const CTime &pOther) const
    {
        return mTime>=pOther.mTime;
    }
    
    ///comparison operator
    bool operator==(const CTime &pOther) const
    {
        return mTime==pOther.mTime;
    }
    
    ///comparison operator
    bool operator!=(const CTime &pOther) const
    {
        return mTime!=pOther.mTime;
    }
    
    ///adds a delta (in microseconds) to this time object
    CTime operator+(int64_t pDelta) const
    {
        return CTime(mTime+pDelta);
    }
    
    ///subtracts a delta (in microseconds) to this time object
    CTime operator-(int64_t pDelta) const
    {
        return CTime(mTime-pDelta);
    }
    
    ///subtracts two time objects, returning the difference in microseconds
    int64_t operator-(const CTime &pRH) const
    {
        return mTime-pRH.mTime;
    }
    
private:
    int64_t mTime;
};

/*namespace soko*/ }

#endif
