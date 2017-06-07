#include "DvbAssertions.h"
#include "DvbUtils.h"
#include <unistd.h>

void* onRun(void *obj)
{
    BaseThread* self = (BaseThread *)obj;
    if (self)
        self->Run((void*)self->mThreadID);
    self->setThreadState(BaseThread::eThreadIdle);
    return (void*)0;
}

BaseThread::BaseThread(): mThreadState(BaseThread::eThreadIdle)
{
    pthread_mutex_init(&mMutex, 0);
}

BaseThread::~BaseThread()
{
    stop();
    pthread_mutex_destroy(&mMutex);
}

void 
BaseThread::setThreadState(ThreadState_e s) 
{ 
    pthread_mutex_lock(&mMutex); 
    mThreadState = s; 
    pthread_mutex_unlock(&mMutex); 
}

int
BaseThread::start()
{
    pthread_attr_t attributes;
    pthread_attr_init(&attributes);
    pthread_attr_setdetachstate(&attributes, PTHREAD_CREATE_DETACHED);
    pthread_create(&mThreadID, &attributes, onRun, this);
    pthread_attr_destroy(&attributes);
    setThreadState(eThreadRun);
    return 0;
}

int
BaseThread::stop()
{
    pthread_mutex_lock(&mMutex); 
    if (mThreadState == eThreadIdle) {
        pthread_mutex_unlock(&mMutex); 
        return 0;
    }
    mThreadState = eThreadStop; 
    pthread_mutex_unlock(&mMutex); 
    while(mThreadState != eThreadIdle)
        usleep(500);
    return 0;
}

static unsigned int _UtilsKeyHash(unsigned int key)
{
    unsigned int hash = 1315423911;
    hash ^= ((hash << 5) + (key & 0x000000ff) + (hash >> 2));
    hash ^= ((hash << 5) + (key & 0x0000ff00) + (hash >> 2));
    hash ^= ((hash << 5) + (key & 0x00ff0000) + (hash >> 2));
    hash ^= ((hash << 5) + (key & 0xff000000) + (hash >> 2));
    return (hash & 0x7FFFFFFF);
}

unsigned int UtilsCreateKey(unsigned int p1, unsigned int p2, unsigned int p3, unsigned int p4)
{
    unsigned int key = _UtilsKeyHash(p1);
    key ^= _UtilsKeyHash(p2 << 8);
    key ^= _UtilsKeyHash(p3 << 16);
    key ^= _UtilsKeyHash(p4 << 24);
    return key;
}

unsigned int UtilsGetBits(const unsigned char *pBuf, unsigned int u32ByteOffset, unsigned int  u32Startbit, unsigned int  u32Bitlen /* <= 32 */)
{
    unsigned char *b;
    unsigned int  v;
    unsigned int  mask;
    unsigned int  tmp_long;
    unsigned int  bitHigh;
    unsigned int  offset;
    unsigned int  tmp_long2;
    if ( u32Bitlen > 32)
        return (unsigned int) 0xFEFEFEFE;

    if(pBuf == (unsigned char*) 0)
        return (unsigned int) 0xFEFEFEFE;

    b = (unsigned char*)&pBuf[u32ByteOffset + (u32Startbit >> 3)];

    u32Startbit %= 8;
    if ((unsigned int)0 == u32Bitlen )
        return (unsigned int)0;

    offset = (u32Bitlen-1) >> 3 ;
    switch (offset) {
        case 0:             /*-- 1..8 bit*/
            tmp_long = (unsigned int)((*(b  )<< 8) +  *(b+1) );
            bitHigh = 16;
            break;
        case 1:             /* -- 9..16 bit*/
            tmp_long = (unsigned int)((*(b  )<<16) + (*(b+1)<< 8) +  *(b+2) );
            bitHigh = 24;
            break;
        case 2:             /*-- 17..24 bit*/
            tmp_long = (unsigned int)((*(b  )<<24) + (*(b+1)<<16) + (*(b+2)<< 8) +  *(b+3) );
            bitHigh = 32;
            break;
        case 3:             /* -- 25..32 bit*/
            tmp_long = (unsigned int)(/*(*(b  )<<32)*+*/(*(b +1 )<<24) + (*(b+2)<<16)+(*(b+3)<< 8) + *(b+4) );
            tmp_long2 =(*(b));
            bitHigh = 40;
            break;
        default:    /* -- 33.. bits: fail, deliver constant fail value*/
            return (unsigned int) 0xFEFEFEFE;
    }

    if (offset == 3) {
        mask = (((unsigned int)0x00000001) << (8-u32Startbit)) - 1;
        tmp_long2 = tmp_long2 & mask;
        tmp_long = tmp_long>>(bitHigh-u32Bitlen-u32Startbit);
        v=(tmp_long2<<(u32Bitlen-(8-u32Startbit))) | tmp_long;
        return v;

    } else {
        u32Startbit = bitHigh - (u32Startbit + u32Bitlen);
        tmp_long = tmp_long >> u32Startbit;
    }

    if (u32Bitlen == 32 )
        mask = 0xFFFFFFFF;
    else
        mask = (((unsigned int)0x00000001) << u32Bitlen) - 1;

    v= tmp_long & mask;
    return v;
}

std::string UtilsCovertText(const unsigned char *uTextBuff, int length)
{
    if (length <= 0) 
        return "";
    int tag = 0;

    /* Learn More, Please read Annex A (normative): Coding of text characters (ETSI EN 300 468 V1.11.1 (2010-04)) */
    std::string codeSet = "";
    if (uTextBuff[tag] < 0x20) {
        switch (uTextBuff[tag]) {
            case 0x01: codeSet = "ISO-8859-5"  ; break ; /* Latin/Cyrillic alphabet        */
            case 0x02: codeSet = "ISO-8859-6"  ; break ; /* Latin/Arabic alphabe           */
            case 0x03: codeSet = "ISO-8859-7"  ; break ; /* Latin/Greek alphabet           */
            case 0x04: codeSet = "ISO-8859-8"  ; break ; /* Latin/Hebrew alphabet          */
            case 0x05: codeSet = "ISO-8859-9"  ; break ; /* Latin alphabet No. 5           */
            case 0x06: codeSet = "ISO-8859-10" ; break ; /* Latin alphabet No. 6           */
            case 0x07: codeSet = "ISO-8859-11" ; break ; /* Latin/Thai (draft only)        */
            case 0x08: codeSet = "ISO-8859-12" ; break ; /* reserved for future use        */
            case 0x09: codeSet = "ISO-8859-13" ; break ; /* Latin alphabet No. 7           */
            case 0x0A: codeSet = "ISO-8859-14" ; break ; /* Latin alphabet No. 8 (Celtic)  */
            case 0x0B: codeSet = "ISO-8859-15" ; break ; /* Latin alphabet No. 9           */
            case 0x0C: codeSet = "reserved"    ; break ; /* reserved for future use */
            case 0x0D: codeSet = "reserved"    ; break ; /* reserved for future use */
            case 0x0E: codeSet = "reserved"    ; break ; /* reserved for future use */
            case 0x0F: codeSet = "reserved"    ; break ; /* reserved for future use */
            case 0x10: /* ISO/IEC 8859 */
                {
                    if (uTextBuff[++tag] != 0x00) printf ("Ts stream is something wrong!\n");
                    switch (uTextBuff[++tag]) {
                        case 0x00: codeSet = "reserved"    ; break ; /* Reserved for future use        */
                        case 0x01: codeSet = "ISO-8859-1"  ; break ; /* West European                  */
                        case 0x02: codeSet = "ISO-8859-2"  ; break ; /* East European                  */
                        case 0x03: codeSet = "ISO-8859-3"  ; break ; /* South European                 */
                        case 0x04: codeSet = "ISO-8859-4"  ; break ; /* North and North-East European  */
                        case 0x05: codeSet = "ISO-8859-5"  ; break ; /* Latin/Cyrillic                 */
                        case 0x06: codeSet = "ISO-8859-6"  ; break ; /* Latin/Arabic                   */
                        case 0x07: codeSet = "ISO-8859-7"  ; break ; /* Latin/Greek                    */
                        case 0x08: codeSet = "ISO-8859-8"  ; break ; /* Latin/Hebrew                   */
                        case 0x09: codeSet = "ISO-8859-9"  ; break ; /* West European & Turkish        */
                        case 0x0A: codeSet = "ISO-8859-10" ; break ; /* North European                 */
                        case 0x0B: codeSet = "ISO-8859-11" ; break ; /* Thai                           */
                        case 0x0C: codeSet = "ISO-8859-12" ; break ; /* Reserved for future use        */
                        case 0x0D: codeSet = "ISO-8859-13" ; break ; /* Baltic                         */
                        case 0x0E: codeSet = "ISO-8859-14" ; break ; /* Celtic                         */
                        case 0x0F: codeSet = "ISO-8859-15" ; break ; /* West Europea                   */
                        default: break;
                    }
                }
                break;
            case 0x11: codeSet = "10646"   ; break ; /* Basic Multilingual Plane (BMP) */
            case 0x12: codeSet = "KSX1001" ; break ; /* Korean Character Set           */
            case 0x13: codeSet = "GB-2312" ; break ; /* Simplified Chinese Character   */
            case 0x14: codeSet = "Big5"    ; break ; /* Traditional Chinese            */
            case 0x15: codeSet = "UTF-8"   ; break ; /* Basic Multilingual Plane (BMP) */
            default: break;
        }
        tag++;
    }

    /* Here need covert but not completed yet! */
    /*-----------------------------------------------------------------------------
     * eg:   
     *      if (codeSet=="GB-2312")
     *      {
     *          result = Convert_BG2312( uTextBuff+tag, length-tag );
     *      }
     *-----------------------------------------------------------------------------*/
    /* 1. trip out control characters */
    while (uTextBuff[tag] < 0x20 && tag < length) tag++;
    /* 2. convert character, here do nothing. */
    std::string result("");
    //TODO
    return result.assign((const char*)&uTextBuff[tag], length - tag);
}

int UtilsCovertTimeMJD( unsigned int mjd, unsigned int *year, unsigned int *mon, unsigned int *day ) 
{
    if (!mjd) return -1;
    long y=0, m=0, d=0, k=0;
    y =  (long)((mjd - 15078.2) / 365.25);
    m =  (long)((mjd - 14956.1 - (long)(y * 365.25) ) / 30.6001);
    d =  (long)(mjd - 14956 - (long)(y * 365.25) - (long)(m * 30.6001));

    k =  (m==14 || m==15) ? 1 : 0;
    y = y + k + 1900;
    m = m - 1 - k*12;

    if (year) *year =(unsigned int)y;
    if (mon ) *mon  =(unsigned int)m;
    if (day ) *day  =(unsigned int)d;
    return 0;
}

int UtilsCovertTimeBCD(unsigned int pBcd, unsigned int *pHour, unsigned int *pMin, unsigned int *pSec)
{
    if (!pBcd)
        return -1;
    unsigned int h = 0, m = 0, s = 0;

    h = ((pBcd >> 20) & 0xf) * 10 + ((pBcd >> 16) & 0xf);
    m = ((pBcd >> 12) & 0xf) * 10 + ((pBcd >> 8) & 0xf);
    s = ((pBcd >> 4) & 0xf) * 10 + ((pBcd) & 0xf);

    if (pHour)
        *pHour = h;
    if (pMin)
        *pMin  = m;
    if (pSec)
        *pSec  = s;
    return 0;
}

unsigned long long UtilsMakeTime(const unsigned int year0, const unsigned int mon0,
    const unsigned int day, const unsigned int hour,
    const unsigned int pMin, const unsigned int sec)
{
    /* if the first param less than 1971 or 0, we only calculate the seconds controled by hour,pMin,sec paramerters. */
    if (year0<1971) return ((hour*60 + pMin) * 60 + sec);

    unsigned int mon = mon0, year = year0;

    /*  1..12 -> 11,12,1..10 */
    if (0 >= (int) (mon -= 2)) {
        mon += 12;  /*  Puts Feb last since it has leap day */
        year -= 1;
    }

    return ((((unsigned long long)
                (year/4 - year/100 + year/400 + 367*mon/12 + day) +
                year*365 - 719499
             )*24 + hour /*  now have hours */
            )*60 + pMin /*  now have minutes */
        )*60 + sec; /*  finally seconds */
}

std::string UtilsInt2Str(int value)
{
    char toStr[32] = { 0 };
    char *pStr     = toStr + 32;

    bool negativeSymbol = value < 0;
    if ( negativeSymbol ) {
        value = -value;
    }
    *--pStr = '\0';
    do{
        *--pStr = char(value % 10) + '0';
        value /= 10;
    }while ( value!=0 );
    if (negativeSymbol) *--pStr = '-';
    return pStr;
}  

bool UtilsIsLeapYear(int year)
{
	if (!(year % 400)) return true;
	if (!(year % 100)) return false;
	if (!(year % 4))   return true;
	return false;
}
