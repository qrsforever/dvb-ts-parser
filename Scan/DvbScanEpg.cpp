#include "DvbAssertions.h"
#include "DvbScanEpg.h"
#include "DvbDemuxFilter.h"
#include "DvbSection.h"
#include "DvbUtils.h"
#include "DvbManager.h"
#include "DvbChannel.h"
#include "DvbEitParser.h"
#include "DvbEpg.h"
#include "DvbShortEventDescriptor.h"
#include "DvbExtendedEventDescriptor.h"
#include "DvbParentalRatingDescriptor.h"

DvbScanEpg::DvbScanEpg(std::string name, int id) : DvbScan(name), mTunerId(id)
{
    LogDvbDebug("contructor\n");
}

DvbScanEpg::~DvbScanEpg() 
{ 
    LogDvbDebug("destructor\n");
}

int 
DvbScanEpg::_ParseEIT(const uint8_t* data, uint16_t size)
{
    //LogDvbDebug("data = %p, size = %d\n", data, size);
    if (!isRun())
        return eSectionSuccess;
    DvbSectionEIT section(data, size);
    if (!section.isValid())
        return eSectionInvalid;
     
    DvbEitParser parser(&section);
    unsigned int hashKey = UtilsCreateKey(section.getServiceId(), parser.getTransportStreamId(), parser.getOriginalNetworkId(), 0);
    DvbChannel* channel = mManager.getDvbChannel(hashKey);
    if (!channel)
        return eSectionNotNeed;

    DvbEpg* epg = channel->getEpg();
    if (!epg)
        return -1;
    if (section.getVersionNumber() != epg->getVersion()) {
        epg->update();
        epg->setVersion(section.getVersionNumber());
    }
    for (DvbEitElement element = parser.elements(); isRun() && element.isValid(); element.advance()) {
        if (epg->isExist(element.getEventId()))
            continue;
        EpgEvent* event = new EpgEvent();
        event->mEventId   = element.getEventId();
        event->mSTime     = element.getStartTime();
        event->mETime     = event->mSTime + element.getDuration();
        event->mScrambled = element.getScrambled();
        for(DvbDescriptor descriptor = element.descriptors(); isRun() && descriptor.isValid(); descriptor.advance()) {
            switch (descriptor.tag()) {
                case 0x4D: {            /* Short descriptor */
                        DvbShortEventDescriptor shortEventDescriptor(descriptor);
                        std::string lan = shortEventDescriptor.getIso639Lan();
                        if (event->mTexts.end() == event->mTexts.find(lan)) {
                            EventText text;
                            text.mLangCode = lan;
                            event->mTexts[lan] = text;
                        }
                        EventText& text = event->mTexts[lan];
                        text.mEventName = shortEventDescriptor.getEventName();
                        text.mShortText = shortEventDescriptor.getShortText();
                    }
                    break;
                case 0x4E: {            /* Extend descriptor, warn-more parts,more lanuage */
                        DvbExtendedEventDescriptor extendEventDescriptor(descriptor);
                        std::string lan = extendEventDescriptor.getIso639Lan();
                        if (event->mTexts.end() == event->mTexts.find(lan)) {
                            EventText text;
                            text.mLangCode = lan;
                            event->mTexts[lan] = text;
                        }
                        EventText& text = event->mTexts[lan];
                        text.mExtendText += extendEventDescriptor.getExtendedText();
                    } 
                    break;
                case 0x55: {            /* Parent control */
                        DvbParentalRatingDescriptor rateDescriptor(descriptor);
                        event->mRatings = rateDescriptor.getRatings();
                    }
                    break;
                default:
                    ;
            }
        }
        epg->addEvent(event);

        if (epg->getMinUpdateTime() > event->mSTime)
            epg->setMinUpdateTime(event->mSTime);
        if (epg->getMaxUpdateTime() < event->mETime)
            epg->setMaxUpdateTime(event->mETime);
        mManager.removeMessages(DvbManager::eDvbMessageEpgUpdate);
        mManager.sendEmptyMessageDelayed(DvbManager::eDvbMessageEpgUpdate, 5000);
    }
    if (isRun())
        return eSectionIncomplete;
    return eSectionSuccess;
}

int 
DvbScanEpg::parseSection(const uint8_t* data, uint16_t size)
{
    if (DvbScan::eScanEit == getState())
         return _ParseEIT(data, size);
    return eSectionNotNeed;
}

void
DvbScanEpg::Run(void* threadID)
{
    LogDvbDebug("ScanEpg thread is running\n");
    setState(eScanIdle);
    DvbDemuxFilter* filter = new DvbDemuxFilter(this);
    if (!filter)
        return ;

    if (!mManager.getFrontend()->isTuned(mTunerId))
        LogDvbWarn("is not Tuned\n");

    setState(eScanEit); 
    filter->setParserBand(mManager.getFrontend()->getParserBand(mTunerId));
    filter->setPsisiPid(eEitPid);
    filter->start(); //block

    delete(filter);
    setState(eScanFinish);
}
