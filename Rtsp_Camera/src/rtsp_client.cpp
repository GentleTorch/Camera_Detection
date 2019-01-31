#include "rtsp_client.h"
#include <thread>

using namespace std;

TaskToken taskToken;
EventTriggerId id;
void restart();
RtspThread* clientThread=nullptr;

// Implementation of "DummySink":

// Even though we're not going to be doing anything with the incoming data, we still need to receive it.
// Define the size of the buffer that we'll use:

DummySink* DummySink::createNew(UsageEnvironment& env, MediaSubsession& subsession, char const* streamId) 
{
    cout<<"createNew()"<<endl;
    return new DummySink(env, subsession, streamId);
}

DummySink::DummySink(UsageEnvironment& env, MediaSubsession& subsession, char const* streamId)
    : MediaSink(env), fSubsession(subsession)
{
    cout<<"DummySink()"<<endl;
    fStreamId = strDup(streamId);
    fReceiveBuffer = new u_int8_t[DUMMY_SINK_RECEIVE_BUFFER_SIZE];
}

void DummySink::SetFFmpeg(FFH264* ffmpeg)
{
    cout<<"SetFFmpeg()"<<endl;
    m_ffmpeg = ffmpeg;
}

DummySink::~DummySink() 
{
    cout<<"~DummySink()"<<endl;
    delete[] fReceiveBuffer;
    delete[] fStreamId;
}

// If you don't want to see debugging output for each received frame, then comment out the following line:
#define DEBUG_PRINT_EACH_RECEIVED_FRAME 1
SPropRecord* p_record = NULL;
void DummySink::afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime, unsigned durationInMicroseconds) 
{
    cout<<"afterGettingFrame()"<<endl;

    while(true)
    {
        if(NULL == p_record) {
            unsigned int Num = 0;
            unsigned int &SPropRecords = Num;
            p_record = parseSPropParameterSets(fSubsession.fmtp_spropparametersets(), SPropRecords);
            if(2 != Num) {
                p_record = NULL;
                break;
            }
        }
        SPropRecord &sps = p_record[0];
        SPropRecord &pps = p_record[1];
        //std::cout << sps.sPropBytes << "\t" << sps.sPropLength  << std::endl;
        cout<<"frame size "<<frameSize<<endl;

        m_ffmpeg->DecodeFrame(sps.sPropBytes, sps.sPropLength, pps.sPropBytes, pps.sPropLength, fReceiveBuffer, frameSize, presentationTime.tv_sec, presentationTime.tv_usec/1000);
        break;
    }
    this->continuePlaying();
}

Boolean DummySink::continuePlaying() 
{
    cout<<"continuePlaying()"<<endl;
    envir().taskScheduler().rescheduleDelayedTask(taskToken,15*1000000,(TaskFunc *)restart,nullptr);
    if (fSource == NULL) return False;
    fSource->getNextFrame(fReceiveBuffer, DUMMY_SINK_RECEIVE_BUFFER_SIZE, afterGettingFrame, this, onSourceClosure, this);
    return True;
}

static inline RtspClient *p_this(RTSPClient* rtspClient) { return static_cast<RtspClient*>(rtspClient); }
#define P_THIS p_this(rtspClient)


RtspClient::RtspClient(UsageEnvironment& env, char const* rtspURL, char const* sUser, char const* sPasswd, FFH264** ffmpeg) 
    : RTSPClient(env, rtspURL, 255, NULL, 0, -1), m_Authenticator(sUser, sPasswd)
{
    cout<<"RtspClient()"<<endl;
    m_ffmpeg = *ffmpeg;
    m_subSessionIter=NULL;
    m_subSession=NULL;
    m_session=NULL;

}	


void RtspClient::Play() 
{
    taskToken=envir().taskScheduler().scheduleDelayedTask(15*1000000,(TaskFunc *)restart,nullptr);
    id=envir().taskScheduler().createEventTrigger((TaskFunc *)restart);
    cout<<"Play()"<<endl;
    this->SendNextCommand();
}


RtspClient::~RtspClient()
{

}

void RtspClient::shutdownStream()
{
    UsageEnvironment& env=this->envir();

    if(nullptr!=this->m_subSession)
    {
        Boolean someSubsessionWereActive=False;
        MediaSubsession* subsession;

        while((subsession=this->m_subSessionIter->next())!=nullptr)
        {
            if(subsession->sink != nullptr)
            {
                Medium::close(subsession->sink);
                subsession->sink=nullptr;
            }

            if(subsession->rtcpInstance() != nullptr)
            {
                subsession->rtcpInstance()->setByeHandler(nullptr,nullptr);
            }

            someSubsessionWereActive=True;
        }

        if(someSubsessionWereActive)
        {
            this->sendTeardownCommand(*(this->m_subSession),nullptr);
        }
    }

   // env<<*this<<"Closing the stream.\n";

//    if(nullptr!=this)
//    {
//        Medium::close(this);
//    }

    if(nullptr != this->m_session)
    {
        //delete this->m_session;
        this->m_session=NULL;
    }

    if(nullptr != this->m_subSession)
    {
        //delete this->m_subSession;
        this->m_subSession=NULL;
    }

    if(nullptr != this->m_subSessionIter)
    {
        delete this->m_subSessionIter;
        this->m_subSessionIter=NULL;
    }

    cout<<"end of shutdown"<<endl;
}

void RtspClient::ContinueAfterDescribe(int resultCode, char* resultString) 
{
    cout<<"continueAfterDescribe()"<<endl;
    if (resultCode != 0)
    {
        std::cout << "Failed to DESCRIBE: " << resultString << std::endl;
        envir().taskScheduler().triggerEvent(id,nullptr);
    }
    else
    {
        std::cout << "Got SDP: " << resultString << std::endl;
        m_session = MediaSession::createNew(envir(), resultString);
        m_subSessionIter = new MediaSubsessionIterator(*m_session);
        this->SendNextCommand();
    }
    delete[] resultString;

}
void RtspClient::ContinueAfterSetup(int resultCode, char* resultString)
{
    cout<<"continueAfterSetup()"<<endl;
    if (resultCode != 0)
    {
        std::cout << "Failed to SETUP: " << resultString << std::endl;
        envir().taskScheduler().triggerEvent(id,nullptr);
    }
    else
    {
        //Live555CodecType codec = GetSessionCodecType(m_subSession->mediumName(), m_subSession->codecName());
        m_subSession->sink = DummySink::createNew(envir(), *m_subSession, this->url());
        ((DummySink*)(m_subSession->sink))->SetFFmpeg(m_ffmpeg);
        if (m_subSession->sink == NULL)
        {
            std::cout << "Failed to create a data sink for " << m_subSession->mediumName() << "/" << m_subSession->codecName() << " subsession: " << envir().getResultMsg() << "\n";
        }
        else
        {
            std::cout << "Created a data sink for the \"" << m_subSession->mediumName() << "/" << m_subSession->codecName() << "\" subsession";
            m_subSession->sink->startPlaying(*(m_subSession->readSource()), NULL, NULL);
        }
    }
    delete[] resultString;
    this->SendNextCommand();
}

void restart()
{
    cout<<"hello world"<<endl;

    clientThread->m_rtspClient->shutdownStream();
    clientThread->eventLoopWatchVariable=1;

}

void RtspClient::ContinueAfterPlay(int resultCode, char* resultString)
{
    cout<<"continueAfterPlay()"<<endl;
    if (resultCode != 0)
    {
        std::cout << "Failed to PLAY: \n" << resultString << std::endl;
        envir().taskScheduler().triggerEvent(id,nullptr);
    }
    else
    {
        std::cout << "PLAY OK\n" << std::endl;
    }

    //taskToken=envir().taskScheduler().scheduleDelayedTask(10*1000000,(TaskFunc *)func,nullptr);
    envir().taskScheduler().unscheduleDelayedTask(taskToken);
    delete[] resultString;
}

void RtspClient::continueAfterSetup(RTSPClient* rtspClient, int resultCode, char* resultString) {
    P_THIS->ContinueAfterSetup(resultCode, resultString);
}

void RtspClient::continueAfterPlay(RTSPClient* rtspClient, int resultCode, char* resultString) {
    P_THIS->ContinueAfterPlay(resultCode, resultString);
}

void RtspClient::continueAfterDescribe(RTSPClient* rtspClient, int resultCode, char* resultString) {
    P_THIS->ContinueAfterDescribe(resultCode, resultString);
}

void RtspClient::SendNextCommand() 
{
    cout<<"sendNextCommand()"<<endl;
    if (m_subSessionIter == NULL)
    {
        // no SDP, send DESCRIBE
        cout<<"send Describe"<<endl;
        this->sendDescribeCommand(continueAfterDescribe, &m_Authenticator);
    }
    else
    {
        cout<<"Iter is not null"<<endl;
        m_subSession = m_subSessionIter->next();
        if (m_subSession != NULL)
        {
            // still subsession to SETUP
            if (!m_subSession->initiate())
            {
                std::cout << "Failed to initiate " << m_subSession->mediumName() << "/" << m_subSession->codecName() << " subsession: " << envir().getResultMsg() << std::endl;
                this->SendNextCommand();
            }
            else
            {
                std::cout << "Initiated " << m_subSession->mediumName() << "/" << m_subSession->codecName() << " subsession" << std::endl;
            }

            /* Change the multicast here */
            cout<<"send Setup"<<endl;
            this->sendSetupCommand(*m_subSession, continueAfterSetup, False, m_isTcp, False, &m_Authenticator);
        }
        else
        {
            // no more subsession to SETUP, send PLAY
            cout<<"send Play"<<endl;
            this->sendPlayCommand(*m_session, continueAfterPlay, (double)0, (double)-1, (float)0, &m_Authenticator);
        }
    }
}

void RtspThread::Run() {
    cout<<"Run()"<<endl;
    OpenCameraPlay();
    //std::thread threadPlay(std::mem_fn(&RtspThread::OpenCameraPlay), this);
    //threadPlay.detach();
}

void RtspThread::OpenCameraPlay() {
    cout<<"OpenCameraPlay()"<<endl;
    ffmpegH264 = new FFH264();

    if(!ffmpegH264->InitH264DecodeEnv()) {
        std::cout << "Error:----> FFmpeg AV_CODEC_ID_H264 Init Error" << std::endl;
        return;
    }

    m_scheduler = BasicTaskScheduler::createNew();
    m_env = BasicUsageEnvironment::createNew(*m_scheduler);
    m_rtspClient = new RtspClient(*m_env, m_url.c_str(), m_user.c_str(), m_passwd.c_str(), &ffmpegH264);
    m_rtspClient->Play();

    m_env->taskScheduler().doEventLoop(&eventLoopWatchVariable);

}

RtspThread::~RtspThread()
{
    //eventLoopWatchVariable=1;

    if(nullptr!=m_rtspClient)
    {
        Medium::close(m_rtspClient);
        m_rtspClient=nullptr;
    }

    if(nullptr!=m_env)
    {
        m_env->reclaim();
        m_env=nullptr;
    }

    if(nullptr!=m_scheduler)
    {
        delete m_scheduler;
        m_scheduler=nullptr;
    }

    if(nullptr!=ffmpegH264)
    {
        delete ffmpegH264;
        ffmpegH264=nullptr;
    }
}


