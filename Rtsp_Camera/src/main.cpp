#include "rtsp_client.h"
#include "ffmpeg_h264.h"
#include <opencv2/opencv.hpp>
#include <unistd.h>
#include <thread>

using namespace std;
int main() {	

    cout<<"main()"<<endl;
    //	RtspThread clientThread("rtsp://admin:pdio#123456@192.168.100.64:554/h264/ch1/main/av_stream", "", "");//admin:pdio#123456@
    //	clientThread.Run();
    while(true)
    {
        clientThread=new RtspThread("rtsp://admin:Pdio#179530!!@192.168.100.64:554/h264/ch1/main/av_stream", "", "");//admin:pdio#123456@
        clientThread->Run();
        delete clientThread;

        chrono::milliseconds dura(1000);
        this_thread::sleep_for(dura);
    }

    return 0;
}

