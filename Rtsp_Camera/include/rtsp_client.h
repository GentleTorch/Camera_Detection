#pragma once

#include <string.h>
#include <vector>
#include <iostream>

#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"
#include "DigestAuthentication.hh"
#include "H264VideoRTPSource.hh"

#include "ffmpeg_h264.h"

using namespace std;

class DummySink: public MediaSink {
public:
    static DummySink* createNew(UsageEnvironment& env, MediaSubsession& subsession,  char const* streamId = NULL);

    void SetFFmpeg(FFH264* ffmpeg);

private:
    DummySink(UsageEnvironment& env, MediaSubsession& subsession, char const* streamId);

    virtual ~DummySink();

    static void afterGettingFrame(void* clientData, unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime, unsigned durationInMicroseconds) {
        static_cast<DummySink*>(clientData)->afterGettingFrame(frameSize, numTruncatedBytes, presentationTime, durationInMicroseconds);
    }

    void afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime, unsigned durationInMicroseconds);

private:
    // redefined virtual functions:
    virtual Boolean continuePlaying();

private:
    u_int8_t* fReceiveBuffer;
    MediaSubsession& fSubsession;
    char* fStreamId;
    FFH264* m_ffmpeg;
};


class RtspClient : public RTSPClient {
public:

    RtspClient(UsageEnvironment& env, char const* rtspURL, char const* sUser, char const* sPasswd, FFH264** ffmpeg);

    void Play();

    void ContinueAfterSetup(int resultCode, char* resultString);

    void ContinueAfterPlay(int resultCode, char* resultString);

    void ContinueAfterDescribe(int resultCode, char* resultString);

    void SendNextCommand();

    inline void SetStreamTCP(bool istcp) {m_isTcp = istcp;}

    virtual ~RtspClient();

    void shutdownStream();

protected:

    static void continueAfterSetup(RTSPClient* rtspClient, int resultCode, char* resultString);

    static void continueAfterPlay(RTSPClient* rtspClient, int resultCode, char* resultString);

    static void continueAfterDescribe(RTSPClient* rtspClient, int resultCode, char* resultString);

    // called only by createNew();

private:
    FFH264* 			m_ffmpeg;
    Authenticator 			m_Authenticator;
    bool 				m_isTcp = true;
    MediaSession* 			m_session;
    MediaSubsession*         	m_subSession;
    MediaSubsessionIterator* 	m_subSessionIter;

};



class RtspThread {

public:
    RtspThread(std::string pUrl, std::string pUser, std::string pPasswd) {
        cout<<"RtspThread()"<<endl;
        m_url = pUrl;
        m_user = pUser;
        m_passwd = pPasswd;
        m_scheduler=NULL;
        m_env=NULL;
    }
    ~RtspThread();

    void Run();

    void OpenCameraPlay();

    FFH264* 				ffmpegH264;
    char eventLoopWatchVariable = 0;
    RtspClient* m_rtspClient;
private:
    TaskScheduler* 				m_scheduler;
    UsageEnvironment*		 	m_env;

    std::string 				m_url;
    std::string 				m_user;
    std::string 				m_passwd;


};

extern RtspThread* clientThread;
