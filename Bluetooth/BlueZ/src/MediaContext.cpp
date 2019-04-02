#include <gio/gio.h>

#include "BlueZ/MediaContext.h"


namespace deviceClientSDK {
namespace bluetooth {
namespace blueZ {

MediaContext::~MediaContext() {
    if (m_mediaStreamFD != INVALID_FD) {
        close(m_mediaStreamFD);
    }
    if (m_isSBCInitialized) {
        sbc_finish(&m_sbcContext);
    }
}

MediaContext::MediaContext() : m_mediaStreamFD{INVALID_FD}, m_readMTU{0}, m_writeMTU{0}, m_isSBCInitialized{false} {
}

void MediaContext::setStreamFD(int streamFD) {
    m_mediaStreamFD = streamFD;
}

int MediaContext::getStreamFD() {
    return m_mediaStreamFD;
}

void MediaContext::setReadMTU(int readMTU) {
    m_readMTU = readMTU;
}

int MediaContext::getReadMTU() {
    return m_readMTU;
}

void MediaContext::setWriteMTU(int writeMTU) {
    m_writeMTU = writeMTU;
}

int MediaContext::getWriteMTU() {
    return m_writeMTU;
}

sbc_t* MediaContext::getSBCContextPtr() {
    return &m_sbcContext;
}

bool MediaContext::isSBCInitialized() {
    return m_isSBCInitialized;
}

void MediaContext::setSBCInitialized(bool initialized) {
    m_isSBCInitialized = initialized;
}

} // namespace blueZ
} // namespace bluetooth
} // namespace deviceClientSDK