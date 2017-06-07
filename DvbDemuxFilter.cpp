#include "DvbAssertions.h"
#include "DvbDemuxFilter.h"
#include "DvbScan.h"
#include "DvbEnum.h"
#include "libzebra.h"
#include <string.h>

static const uint32_t kDefaultBufferSize = 131072; // 131072 = 1024*128
static const int kWaitDataTimeout = 300;
int DvbDemuxFilter::mDemuxFilterCount = 0;

DvbDemuxFilter::DvbDemuxFilter(DvbScan* scan) : mScan(scan), 
    mType(eDemuxPsiFilter), mParserBand(0), mPsisiPid(ePatPid), 
    mIsEnableCrc(0), mUseMessage(1), mTimeout(0)
{
    memset(mMask, 0xff, sizeof(mMask)); 
    memset(mCoef, 0xff, sizeof(mCoef)); 
    memset(mExcl, 0x00, sizeof(mExcl)); 
    DvbDemuxFilter::mDemuxFilterCount++;
}

DvbDemuxFilter::~DvbDemuxFilter()
{
    DvbDemuxFilter::mDemuxFilterCount--;
}

void 
DvbDemuxFilter::setFilterOption(uint8_t* mask, uint8_t* coef, uint8_t* excl) 
{
    memcpy(mMask, mask, FILTER_OPTION_LENGTH);
    memcpy(mCoef, coef, FILTER_OPTION_LENGTH);
    memcpy(mExcl, excl, FILTER_OPTION_LENGTH);
}

int 
DvbDemuxFilter::start()
{
    LogDvbDebug("Start Pid[0x%02x] Timeout[%d]\n", mPsisiPid, mTimeout);
    if (!mParserBand) {
        LogDvbError("band is zero\n");
        return eErr;
    }
    YS_MESSAGE_SETTINGS params;
    params.parse_band = mParserBand;
    params.pid = mPsisiPid;
    switch (mType) {
        case eDemuxTsFilter:  params.type = YS_MESSAGE_TYPE_TS;  break;
        case eDemuxPsiFilter: params.type = YS_MESSAGE_TYPE_PSI; break;
        case eDemuxPesFilter: params.type = YS_MESSAGE_TYPE_PES; break;
        default: params.type = YS_MESSAGE_TYPE_PSI; 
    }
    params.buffer_size = DEFAULT_BUFFER_SIZE;  
    params.DisablePsiCrc = mIsEnableCrc;
    params.use_message_filter = mUseMessage;
    memcpy(params.filter.mask, mMask, FILTER_OPTION_LENGTH);
    memcpy(params.filter.coefficient, mCoef, FILTER_OPTION_LENGTH);
    memcpy(params.filter.exclusion, mExcl, FILTER_OPTION_LENGTH);

    int message_handle = ys_dvb_message_start(&params);
    int size = 0;
    unsigned char* data = 0;
    const int tryCount = mTimeout / DEFAULT_WAITDATA_TIME + 1;
    int waitCount = tryCount;
    int demuxResult = eOk; //-1: err, 0: ok, 1:timeout
    while (mScan->isRun()) {		
        if (DvbScan::eScanSuspend == mScan->getState()) {
            LogDvbDebug("filter interrrupt!\n");
            demuxResult = eInterrupt; 
            goto End;
        }
        size = 0;
        if (ys_dvb_message_get_buffer(message_handle ,(char**)(&data), &size)) {
            LogDvbDebug("err ys_dvb_message_get_buffer\n");
            break;
        }
        if (0 == size) {
            ys_dvb_message_wait_dataready(message_handle, DEFAULT_WAITDATA_TIME);
            if (mTimeout && waitCount > 0) {
                if (0 == --waitCount) {
                    LogDvbDebug("Wait time out, No data to filter out... \n");
                    demuxResult = eTimeout;
                    goto End;
                }
            }
            continue;
        }
        if (DvbScan::eSectionSuccess == mScan->parseSection(data, size))
            goto End;
        ys_dvb_message_release_buffer(message_handle, size);	 
        waitCount = tryCount; 
    }
End:
    LogDvbDebug("Stop Pid[0x%02x] Timeout[%d]\n", mPsisiPid, mTimeout);
    ys_dvb_message_stop(message_handle);
    return demuxResult;  
}

#if 0 //brcm_7405

typedef struct _demux{
    unsigned char packet[10];
    unsigned char sec_buf[4*1025];
    int packet_len;
    int sec_header;
	int synced;
	int sec_len;
	int sec_pos;
}section_demux;

    int datalen,buffpos;
    section_demux sec_demux;
    memset(&sec_demux, 0, sizeof(section_demux));
    sec_demux.sec_header = 8;        
        if (mType == eDemuxTsFilter) { //Ts data process
            if (DvbScan::eSectionSuccess == mScan->parseSection(data, size))
                goto End;
            ys_dvb_message_release_buffer(message_handle, size);	 
        } else if (mType == eDemuxPsiFilter) {//Psi data preprocess                
            datalen = size;
            buffpos = 0;
            while(datalen > 0){						
                if(sec_demux.synced==0) {  //section header
                    if(sec_demux.packet_len > 0){  // section buffed last loop
                        if(datalen >= sec_demux.sec_header - sec_demux.packet_len){
                            memcpy(sec_demux.packet + sec_demux.packet_len, data + buffpos, sec_demux.sec_header - sec_demux.packet_len);
                            datalen -= (sec_demux.sec_header - sec_demux.packet_len);
                            buffpos += (sec_demux.sec_header - sec_demux.packet_len);
                            sec_demux.packet_len = sec_demux.sec_header;
                        }else{  //
                            memcpy(sec_demux.packet + sec_demux.packet_len, data + buffpos, datalen);
                            buffpos += datalen;
                            sec_demux.packet_len += datalen;
                            datalen -= datalen;
                        }
                    }else if(datalen <= sec_demux.sec_header ){
                        memcpy(sec_demux.packet + sec_demux.packet_len, data + buffpos, datalen);
                        sec_demux.packet_len += datalen;
                        buffpos += datalen;
                        datalen -= datalen;
                    }else{
                        memcpy(sec_demux.packet + sec_demux.packet_len,data + buffpos, sec_demux.sec_header);
                        buffpos += sec_demux.sec_header;
                        datalen -= sec_demux.sec_header;
                        sec_demux.packet_len += sec_demux.sec_header;
                    }

                    if(sec_demux.packet_len < sec_demux.sec_header)
                        break;
                    /** section header is ok **/
                    sec_demux.sec_len = (unsigned int) sec_demux.packet[1];
                    sec_demux.sec_len = (((sec_demux.sec_len&0x0f) <<8) | ((unsigned int)sec_demux.packet[2])) + 3;
                    memcpy(sec_demux.sec_buf, sec_demux.packet, sec_demux.sec_header);
                    sec_demux.sec_pos = sec_demux.sec_header;
                    sec_demux.synced = 1;
                    sec_demux.packet_len = 0;
                }

                if((datalen + sec_demux.sec_pos) < sec_demux.sec_len) {
                    memcpy(sec_demux.sec_buf + sec_demux.sec_pos, data + buffpos, datalen);
                    sec_demux.sec_pos += datalen;
                    buffpos += datalen;
                    datalen -= datalen;
                    break;
                } else {
                    memcpy(sec_demux.sec_buf+sec_demux.sec_pos,data+buffpos,sec_demux.sec_len-sec_demux.sec_pos);
                    datalen -= (sec_demux.sec_len-sec_demux.sec_pos);
                    buffpos += (sec_demux.sec_len-sec_demux.sec_pos);
                    sec_demux.synced =0;
                    sec_demux.sec_pos=0;
                    if (DvbScan::eSectionSuccess == mScan->parseSection(sec_demux.sec_buf, sec_demux.sec_len))
                        goto End;
                }
            }//end  while(datalen > 0)
            ys_dvb_message_release_buffer(message_handle, size);  
        } else if (mType == eDemuxPesFilter) { //Pes data process
            if (DvbScan::eSectionSuccess == mScan->parseSection(data, size))
                goto End;
            ys_dvb_message_release_buffer(message_handle, size);     
        }
#endif
