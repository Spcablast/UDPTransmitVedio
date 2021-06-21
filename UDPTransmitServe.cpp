#include <stdio.h>
#include <Winsock2.h>
#include <cv.h>  
#include <cxcore.h>  
#include <highgui.h>  

#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
//#include <opencv2/imgcodecs/legacy/constants_c.h>

#include <iostream>  
#include <windows.h>

//相当于是把ws2_32.lib 这个库加入到工程文件中，这同setting中加入库效果一样，但是方便了代码共享时不必加入该库
#pragma comment(lib,"ws2_32.lib")
#pragma warning(disable:4996)

const int blocksize = 28880;
const int imgSize = 28880;

//  定义包结构  
struct recvbuf//包格式  
{
	char buf[blocksize];//存放数据的变量  
	int flag;//标志  

};
int nRecvBuf = 1024 * 1024 * 10;

int main()
{
	//开客户端线程发图片
	//HANDLE hThread = CreateThread(NULL, 0, VedioClient, NULL, 0, NULL);
	//CloseHandle(hThread);

	//接收图片 
	struct recvbuf data;

	//开UDP通信
	//加载套接字库
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	wVersionRequested = MAKEWORD(2, 0);

	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) {
		return 0;
	}

	if (LOBYTE(wsaData.wVersion) != 2 ||
		HIBYTE(wsaData.wVersion) != 0) {
		WSACleanup();
		return 0;
	}

	//创建用于监听的套接字
	SOCKET sockSrv = socket(AF_INET, SOCK_DGRAM, 0);//失败会返回 INVALID_SOCKET
	if (INVALID_SOCKET == sockSrv) {
		printf("Socket 创建失败，Exit!");
		return 0;
	}

	//定义sockSrv发送和接收数据包的地址
	SOCKADDR_IN addrSrv;
	//addrSrv.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	addrSrv.sin_addr.S_un.S_addr = inet_addr("169.254.209.52");
	addrSrv.sin_family = AF_INET;

	addrSrv.sin_port = htons(40001);
	//绑定地址和socket,在用了namespace的时候，bind的返回值就不是int了，就无法和-1进行比较
	if (bind(sockSrv, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR)) == -1) {
		printf("Server bind erro!\n");
		return 0;
	}

	//listen(sockSrv,5);

	SOCKADDR_IN addrClient;
	int len = sizeof(SOCKADDR);

	//声明IplImage指针  
	IplImage* pFrame = NULL;

	cvNamedWindow("recvideo", 1);

	//接收缓存10M  
	setsockopt(sockSrv, SOL_SOCKET, SO_RCVBUF, (const char*)&nRecvBuf, sizeof(int));
	int COUNT = 0;
	int imgid = 0;
	char img[blocksize * 32] = { 0 };
	int n;

	
	unsigned char recImg[imgSize];

	char recImgData[imgSize];

	while (1)
	{
		std::vector<uint8_t> imgData;
		//存数据长度到n，
		n = recvfrom(sockSrv, recImgData, imgSize, 0, (SOCKADDR*)&addrClient, &len);
		//如果接收到的数据长度小于0，那么说明没收到，循环跳下一次
		if ( n <= 0) { 
			printf("receive error!");
			continue;
		}
		//数据转移到vector中，准备解码
		for (int i = 0;i < n;i++) {
			imgData.push_back((uchar)recImgData[i]);
		}

		cv::Mat image = cv::imdecode(imgData, CV_LOAD_IMAGE_COLOR);
		cv::imshow("videorec", image);
		cv::waitKey(1);

		//************************************//
		//memset(data.buf, 0, sizeof(char)*blocksize);
		//for (int i = 0; i < 32; i++)
		//{
		//	n = recvfrom(sockSrv, (char*)(&data), blocksize + 4, 0, (SOCKADDR*)&addrClient, &len);
		//	COUNT = COUNT + data.flag;
		//	for (int k = 0; k < blocksize; k++)
		//	{
		//		img[i * blocksize + k] = data.buf[k];
		//	}
		//	if (data.flag == 2)  //data.flag==2是一帧中的最后一个数据块  
		//	{
		//		if (COUNT == 33)
		//		{
		//			pFrame = cvCreateImageHeader(cvSize(640, 480), IPL_DEPTH_8U, 3);
		//			cvSetData(pFrame, img, 640 * 3);//由收到的数据建立一帧图像  
		//			printf("Frame %d/n", imgid);
		//			imgid++;
		//			cvShowImage("video", pFrame);
		//			char c = cvWaitKey(1000 / 30);
		//		}
		//		else
		//		{
		//			COUNT = 0;
		//			i = -1;
		//		}
		//	}
		//}
		//************************************//
	}
	
	cvDestroyWindow("video");
	return 0;
}

