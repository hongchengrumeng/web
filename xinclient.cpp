#include <stdio.h>
#include <stdlib.h>
#include <Winsock2.h>
#include <Iphlpapi.h>
#include <iostream>
#include <tchar.h>
#include<string.h>
#include <conio.h>
#include<tchar.h>
#include<iomanip>
#include<ws2tcpip.h>
#pragma oncemm9*kj9n
#pragma comment (lib,"Iphlpapi")
#pragma comment (lib,"Ws2_32")

#define MAX_BUF_SIZE 65535
#define UDP_SRV_PORT 2345
struct QWEHEADER
{
    USHORT choose;
    char usSourcePort;
    USHORT usDestPort;
    ULONG dwSeq;
    ULONG dwAck;
    UCHAR ucLength;
    UCHAR ucFlag;
};
int main(int argc,char* argv[])
{
    char a,b;
	int x,i;
	char copy1[10],copy2[10];
    char ClientBuf[1024];
    char GetBuf[1024];
    char portnum[5];
	char string[100];
    char sta[6];
    USHORT chose;
    int BytesReceived,ServerUDPPort,Getlen;
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2),&wsa);
    SOCKET CSocket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    SOCKADDR_IN TCPServer;
    TCPServer.sin_family=AF_INET;
    TCPServer.sin_port=htons((u_short)atoi("3000"));
    TCPServer.sin_addr.S_un.S_addr=inet_addr("169.254.225.99");
    connect(CSocket,(sockaddr*)&TCPServer,sizeof(TCPServer));
    BytesReceived=recv(CSocket,ClientBuf,sizeof(ClientBuf),0);
    memcpy(sta,ClientBuf+5,5);
    sta[5]='\0';
    if(strcmp("START",sta)!=0)
    {
        printf("连接失败\n");
    }
    memcpy(portnum,ClientBuf,5);
    ServerUDPPort=(u_short)atoi(portnum);
    SOCKADDR_IN UDPClientAddr;
    UDPClientAddr.sin_family=AF_INET;
    UDPClientAddr.sin_port=htons(9001);
    UDPClientAddr.sin_addr.S_un.S_addr=inet_addr("169.254.225.99");
    SOCKET UDPSrvSocket = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
    /*//获取本机名
    char hostname[256];
    gethostname(hostname,sizeof(hostname));
    //获取本机IP
    hostent *pHostent = gethostbyname(hostname);
    //填充本地UDP socket 地址结构
    SOCKADDR_IN UDPSrvAddr;
    memset(&UDPSrvAddr,0,sizeof(SOCKADDR_IN));
    UDPSrvAddr.sin_family = AF_INET;
    UDPSrvAddr.sin_port = htons(ServerUDPPort);
    UDPSrvAddr.sin_addr = * (in_addr *)pHostent->h_addr_list[2];
    //绑定UDP端口x
    bind(UDPSrvSocket,(sockaddr *)&UDPSrvAddr,sizeof(UDPSrvAddr));*/
     QWEHEADER *pIpHdr;
     pIpHdr=(QWEHEADER *)ClientBuf;
	 memset(string,'\0',sizeof(string));
    while(TRUE)
    {
        printf("1 通过Tcp得到Server的系统时间\n");
        printf("2 通过Udp实现各种功能\n");
        printf("3 释放资源，退出程序\n");
        printf("请输入编号以选择服务：");
        scanf("%c",&a);
        scanf("%c",&b);
        switch(a)
        {
        case '1':
            send(CSocket,"GET CUR TIME",sizeof("GET CUR TIME"),0);
            recv(CSocket,ClientBuf,sizeof(ClientBuf),0);
            ClientBuf[21]='\0';
            printf("%s\n",ClientBuf);
            memset(ClientBuf,'\0',sizeof(ClientBuf));
            break;
        case '2':
            printf("选择一个数字输入\n");
            printf("1是登录\n");
            printf("2是注册\n");
            scanf("%c",&chose);
			getchar();
			ClientBuf[0]= chose;
            printf("\n请输入提交给服务器的信息，按回车发送\n");
			//gets(&ClientBuf[1]);
			if(ClientBuf[0] =='1'||ClientBuf[0] =='2')
			{
			printf("Input username(<10):\n");
			gets(copy1);
			printf("Input password(<10):\n");
			gets(copy2);
			strcpy(&ClientBuf[1],copy1);
			for(i=0;ClientBuf[i]!='\0';i++)
			{x=1;}
				ClientBuf[i] ='&';
			//printf("x=%d",x);
			strcpy(&ClientBuf[i+1],copy2);
			}
			else
				gets(&ClientBuf[1]);
			//strcpy(pIpHdr->usSourcePort,string);
			//printf("发送数据：%d\n",ClientBuf);
            sendto(UDPSrvSocket,ClientBuf,strlen(ClientBuf),0,(sockaddr *)&UDPClientAddr,sizeof(UDPClientAddr));
            Getlen=sizeof(UDPClientAddr);
			memset(GetBuf,'\0',sizeof(GetBuf));
            recvfrom(UDPSrvSocket,GetBuf,1024,0,(sockaddr *)&UDPClientAddr,&Getlen);
            //printf("getlasterro=%d",WSAGetLastError());
            printf("%s\n",GetBuf);
            break;
        case '3':
            closesocket(CSocket);
            closesocket(UDPSrvSocket);
            WSACleanup();
            return 0;
        }
    }
    return 0;
}
