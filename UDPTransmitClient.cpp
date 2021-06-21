//相当于是把ws2_32.lib 这个库加入到工程文件中，这同setting中加入库效果一样，但是方便了代码共享时不必加入该库
#pragma comment(lib,"ws2_32.lib")
#pragma warning(disable:4996)

//热点时ip
//#define Ip_Slaver	"192.168.43.75"
//网线时ip
#define Ip_Slaver	"169.254.209.52"

#include <stdio.h>
#include <Winsock2.h>
#include <cv.h>  
#include <cxcore.h>  
#include <highgui.h>  
#include <opencv2/opencv.hpp>
#include <iostream>  
#include <windows.h>

//using namespace std;
//using namespace cv;

//一个包的数据大小
const int blocksize = 28880;
const int imgsize = 28880;
//  定义包结构  
struct recvbuf//包格式  
{
	char buf[blocksize];//存放数据的变量  
	int flag;//标志  
};
int nRecvBuf = 1024 * 1024 * 10;

void main()
{
	//开UDP通信
	//加载套接字库
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	wVersionRequested = MAKEWORD(2, 0);

	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) {
		return;
	}

	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 0) {
		WSACleanup();
		return;
	}
	SOCKET sockClient = socket(AF_INET, SOCK_DGRAM, 0);


	const char* ipAddr = Ip_Slaver;

	SOCKADDR_IN addrSrv;
	//addrSrv.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	addrSrv.sin_addr.S_un.S_addr = inet_addr(ipAddr);
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(40001);

	int nRecvBuf = 1024 * 1024 * 10;//接收缓存10M  
	setsockopt(sockClient, SOL_SOCKET, SO_RCVBUF, (const char*)&nRecvBuf, sizeof(int));


	//读摄像头
	cv::VideoCapture capture;
	capture.open(0);
	if (!capture.isOpened()) {
		std::cout<<("打开摄像头失败")<<std::endl;
		return;
	}
    
	cv::Mat frame;
	struct recvbuf data;
	int imgid = 0;

	while (1) {
		//memset(data.buf, 0, sizeof(char)*blocksize);
		capture >> frame;                        //取帧操作
		uchar* img = frame.data;

		cv::Size temp = frame.size();
		//压缩图片
		//std::vector<unsigned char> inImage;
		//cv::imencode(".jpg", frame, inImage);
		//size_t datalen = inImage.size();
		cv::Mat imgtst;
		
		int quality = 50; //压缩比率0～100
		std::vector<uint8_t> imageData;
		std::vector<int> compress_params;
		compress_params.push_back(cv::IMWRITE_JPEG_QUALITY);
		compress_params.push_back(quality);
		imencode(".jpg", frame, imageData, compress_params);

		uchar testvectorchar[imgsize];
		for (int i = 0; i < imageData.size(); i++) {
			testvectorchar[i] = imageData[i];
		}
/*		uchar* testvectoruchar = (uchar*)&imageData;
		uint8_t* testvectoruint = (uint8_t*)&imageData;
		char* testvectorchar = (char*)&imageData;
		std::cout << (int)imageData[0] << "    " << (int)testvectoruchar[0] << "   " << (int)testvectoruint[0] << "    "<<(int)testvectorchar[0] << std::endl;
		*///const char * testmsg = "i am a test message0.0";
		//发出去
		//int err = sendto(sockClient, (char*)(&imageData), imageData.size(), 0, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));
		int err = sendto(sockClient, (char*)testvectorchar, imageData.size(), 0, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));
		if (err <= 0) {
			std::cout << ("发送失败") << std::endl;
		}
		
		//解压缩
		std::vector<uint8_t> p_data ;
		for (int i = 0; i < imageData.size(); i++)
		{
			p_data.push_back(imageData[i]);
		}
		cv::Mat image = cv::imdecode(p_data, CV_LOAD_IMAGE_COLOR);
		cv::imshow("videodecode", frame);
		cv::waitKey(1);

		//***********************************************//
		////数据拆成32份
		//for (int i = 0; i < 32; i++)                    //640*480*3= 921600 1280*720 = 3*921600
		//{
		//	//一份data一个buff,存img的0到blocksize的内容
		//	for (int k = 0; k < blocksize; k++)
		//	{
		//		data.buf[k] = img[i * blocksize + k];
		//	}
		//	if (i == 31)                         //标识一帧中最后一个数据包             
		//	{
		//		data.flag = 2;
		//	}
		//	else
		//	{
		//		data.flag = 1;
		//	}
		//	sendto(sockClient, (char*)(&data), imgsize, 0, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));
		//}
		//imgid++;
		//printf("img %d", imgid);
		////显示图像
		//cv::imshow("video", frame);
		//cv::waitKey(1);
		//***********************************************//
	}
	capture.release();

	closesocket(sockClient);
	WSACleanup();
}
