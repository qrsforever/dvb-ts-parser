#ifndef __DVBUTIL__H_
#define __DVBUTIL__H_

#include <pthread.h>
#include <iostream>
#include <string>

void* onRun(void *);
class BaseThread {
public:
    typedef enum ThreadState_ {
        eThreadIdle  = 0,
        eThreadRun   = 1,
        eThreadStop  = 2,
    }ThreadState_e;
    friend void* onRun(void*);
    int start(); 
    int stop();
    bool isRun() { return  mThreadState == eThreadRun; }
    void setThreadState(ThreadState_e s);
    ThreadState_e getThreadState() { return mThreadState; }
    ~BaseThread();
protected:
    BaseThread();
    virtual void Run(void* param) = 0;
    pthread_t mThreadID;
private:
    pthread_mutex_t mMutex;
    ThreadState_e mThreadState;
};

unsigned int UtilsCreateKey(unsigned int p1, unsigned int p2, unsigned int p3, unsigned int p4);
unsigned int UtilsGetBits(const unsigned char *pBuf, unsigned int u32ByteOffset, unsigned int  u32Startbit, unsigned int  u32Bitlen);
std::string UtilsCovertText(const unsigned char *uTextBuff, int length);
int UtilsCovertTimeMJD(unsigned int mjd, unsigned int *year, unsigned int *mon, unsigned int *day );
int UtilsCovertTimeBCD(unsigned int pBcd, unsigned int *pHour, unsigned int *pMin, unsigned int *pSec);
unsigned long long UtilsMakeTime(const unsigned int year0, const unsigned int mon0, const unsigned int day, const unsigned int hour, const unsigned int pMin, const unsigned int sec);
std::string UtilsInt2Str(int value);
bool UtilsIsLeapYear(int year);
#endif
