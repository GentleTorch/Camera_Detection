#include "ffmpeg_h264.h"
#include <iostream>
#include <opencv2/opencv.hpp>
#include <unistd.h>
using namespace std;

//初始化
FFH264::FFH264() {
	cout<<"FFH264()"<<endl;
	m_videondex = -1;
	m_isRecord = -1;
	m_swsContext = nullptr;
	m_codecContext = nullptr;
	av_register_all();
	m_frame = av_frame_alloc();
	m_frameBGR = av_frame_alloc();
}
FFH264::~FFH264() {
	cout<<"~FFH264()"<<endl;
	av_frame_free(&m_frame);
	av_frame_free(&m_frameBGR);
}
bool FFH264::InitH264DecodeEnv() {

	cout<<"InitH264DecodeEnv()"<<endl;
	do {
		m_codec = avcodec_find_decoder(AV_CODEC_ID_H264);
		if(!m_codec) {
			break;
		}
		m_codecContext = avcodec_alloc_context3(m_codec);
		if(avcodec_open2(m_codecContext, m_codec, nullptr) < 0) {
			break;
		}
		return true;
	} while(0);
	return false;
}

void FFH264::SetPlayState(bool pause) {
	cout<<"SetPlayState()"<<endl;
	if(pause) {
		m_playMutex.lock();
	}
	else {
		m_playMutex.unlock();
	}
}

void FFH264::DecodeFrame(unsigned char* sPropBytes, int sPropLength, unsigned char* ppsPropBytes, int ppsPropLength, uint8_t* frameBuffer, int frameLength, long second, long microSecond) {
	cout<<"DecodeFrame()" << sPropLength << ppsPropLength <<endl;
	if(frameLength <= 0) return;

cout<<"DecodeFrame()" << frameBuffer[0] << "-" << frameBuffer[1] << "-" << frameBuffer[2] << "-" << frameBuffer[3]  << "-" << frameBuffer[4] <<endl;

	unsigned char nalu_header[4] = {0x00, 0x00, 0x00, 0x01};
	int totalSize = 4 + sPropLength + 4 + ppsPropLength + 4 + frameLength;
	unsigned char* tmp = new unsigned char[totalSize];
	int idx = 0;
	memcpy(tmp + idx, nalu_header, 4);
        idx += 4;
	memcpy(tmp + idx, sPropBytes, sPropLength);
        idx += sPropLength;
	memcpy(tmp + idx, nalu_header, 4);
        idx += 4;
	memcpy(tmp + idx, ppsPropBytes, ppsPropLength);
        idx += ppsPropLength;
	memcpy(tmp + idx, nalu_header, 4);
        idx += 4;
	memcpy(tmp + idx,  frameBuffer, frameLength);
	int frameFinished = 0;
	AVPacket framePacket;
	av_init_packet(&framePacket);
	framePacket.size = totalSize;
	framePacket.data = tmp;
    	//framePacket.size=frameLength;
	//framePacket.data=frameBuffer;

	int ret = avcodec_decode_video2(m_codecContext, m_frame, &frameFinished, &framePacket);
	//if(ret < 0) {
		//std::cout << "Decodec Error!" << std::endl;
	//}
	if(frameFinished) {
std::cout << "Decodec succ!" << std::endl;
		m_playMutex.lock();
		VideoWidth = m_frame->width;
		VideoHeight = m_frame->height;
		if(m_swsContext == nullptr) {
			m_swsContext = sws_getCachedContext(m_swsContext, VideoWidth, VideoHeight, AVPixelFormat::AV_PIX_FMT_YUV420P, VideoWidth, VideoHeight, 
                AVPixelFormat::AV_PIX_FMT_BGR24, SWS_BICUBIC, NULL, NULL, NULL);
            int size = avpicture_get_size(AV_PIX_FMT_BGR24, m_codecContext->width, m_codecContext->height);
			out_buffer = (uint8_t* )av_malloc(size);
            avpicture_fill((AVPicture*)m_frameBGR, out_buffer, AV_PIX_FMT_BGR24, m_codecContext->width, m_codecContext->height);
		}
		sws_scale(m_swsContext, (const uint8_t* const* )m_frame->data, m_frame->linesize, 0, VideoHeight, m_frameBGR->data, m_frameBGR->linesize);
		m_playMutex.unlock();

		cv::Mat m_frameMat;
		if(VideoWidth>0&&VideoHeight>0){
			if(m_frameMat.empty()) {
				m_frameMat.create(cv::Size(VideoWidth, VideoHeight), CV_8UC3);
			}
			int length;

			GetDecodedFrameData(m_frameMat.data,length);
			// for(int i = 0; i < length; i += 3) {
			// 	uchar temp = m_frameMat.data[i];
			// 	m_frameMat.data[i] = m_frameMat.data[i+2]; 
			// 	m_frameMat.data[i+2] = temp;
			// }
			cv::imshow("Camera", m_frameMat);
			cv::waitKey(1);
		}
		
	}
	av_free_packet(&framePacket);
	delete[] tmp;
	tmp = nullptr; //防止产生悬垂指针使程序产生没必要的错误
}

void FFH264::GetDecodedFrameData(unsigned char* data, int& length) {
	cout<<"GetDecodeFrameData()"<<endl;
	m_playMutex.lock();
	length = VideoWidth* VideoHeight * 3 ;
	memcpy(data, out_buffer, length);
	m_playMutex.unlock();
}

void FFH264::GetDecodedFrameInfo(int& width, int& heigth) {
	cout<<"GetDecodedFrameInfo()"<<endl;
	width = VideoWidth;
	heigth = VideoHeight;
}

