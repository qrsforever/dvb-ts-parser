#ifndef __DvbDemuxFilter__H_
#define __DvbDemuxFilter__H_

#include "DvbEnum.h"
#include <stdint.h>

#define FILTER_OPTION_LENGTH    16
#define DEFAULT_BUFFER_SIZE     131072 // 1024 * 128
#define DEFAULT_WAITDATA_TIME   300

class DvbScan;
class DvbDemuxFilter {
public :
    DvbDemuxFilter(DvbScan* scan);
    ~DvbDemuxFilter();
    void setDemuxFilterType(DemuxFilterType_e type) { mType = type; }
    void setParserBand(int band) { mParserBand = band; }
    void setPsisiPid(int pid) { mPsisiPid = pid; }
    void setTimeout(int msec) { mTimeout = msec; }
    void setEnableCrc(bool val) { mIsEnableCrc = val; }
    void setUseMessage(bool val) { mUseMessage = val; }
    void setFilterOption(uint8_t* mask, uint8_t* coef, uint8_t* excl);
    int start();
    enum DvbDemuxResult {
        eErr = -1,
        eOk = 0,
        eTimeout = 1,
        eInterrupt = 2,
    };
    static int mDemuxFilterCount;
private:
    DvbScan* mScan;
    DemuxFilterType_e mType;
    int mParserBand;
    int mPsisiPid;
    uint8_t mMask[FILTER_OPTION_LENGTH];
    uint8_t mCoef[FILTER_OPTION_LENGTH];
    uint8_t mExcl[FILTER_OPTION_LENGTH];
    int mIsEnableCrc;
    int mUseMessage;
    int mTimeout; //unit: ms
};

#endif
