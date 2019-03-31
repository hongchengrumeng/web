#include <stdio.h>
#include <stdlib.h>
#include <Winsock2.h>
#include<Windows.h>
#include <Iphlpapi.h>
#include<time.h>
#include <iostream>
#include<string.h>
#include <iomanip>
#include<fstream>
#include <tchar.h>
#include <conio.h>
#include <ws2tcpip.h>
#include <mysql.h>
using namespace std;
 
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "libmysql.lib")
#pragma comment (lib,"Iphlpapi")

#pragma pack(1)
long TcpClientCount = 0;

#define MAX_CLIENT 10 //同时服务的并发连接数上限
#define MAX_BUF_SIZE 65535 //接收发送缓存区大小
#define UDP_SRV_PORT 9001 //server的UDP端口号
const char START_CMD[] = "START";
const char GETCURTIME_CMD[] = "GET CUR TIME";
struct TcpThreadParam   //传递给TCP线程的结构化参数
{
    SOCKET socket; //套接字号
    sockaddr_in addr;//地址信息
};
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
DWORD WINAPI TcpServeThread(LPVOID lpParam);
DWORD WINAPI UdpServer(LPVOID lpParam);
int main(int argc, char * argv[])
{
	WSADATA wsa;
    WSAStartup(MAKEWORD(2,2),&wsa);
    //创建用于侦听的TCP Socket
    SOCKET ListenSocket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    //获取本机名
    char hostname[256];
    gethostname(hostname,sizeof(hostname));
    //获取本机IP地址
    hostent *pHostent = gethostbyname(hostname);
    //填充本地TCP Listen Socket地址结构
    for(int i=0;(int)(pHostent->h_addr_list[i])!=0;i++)
    {
        unsigned char * hostip1=(unsigned char * )(pHostent->h_addr_list[i]);
        printf("ip:%d.%d.%d.%d\n",hostip1[0],hostip1[1],hostip1[2],hostip1[3]);
    }
    int j;
    printf("请选择ip：");
    scanf("%d",&j);
    SOCKADDR_IN ListenAddr;
    ListenAddr.sin_family = AF_INET;
    ListenAddr.sin_port = htons((u_short)atoi("3000"));
    ListenAddr.sin_addr = * (in_addr *)pHostent->h_addr_list[j];
    //绑定TCP侦听端口
    bind(ListenSocket,(sockaddr *)&ListenAddr,sizeof(ListenAddr));
    //监听
    printf("开始监听：\n");
    listen(ListenSocket,SOMAXCONN);
    //在一个主循环中接收客户端连接请求并创建服务进程
    SOCKET TcpSocket;
    SOCKADDR_IN TcpClientAddr;
    DWORD dwThreadId1;
	CreateThread(NULL,0,UdpServer,NULL,0,&dwThreadId1);
    while(TRUE)
    {
        //接受客户端连接请求
        int iSockAddrLen=sizeof(sockaddr);
        TcpSocket = accept(ListenSocket,(sockaddr *)&TcpClientAddr,&iSockAddrLen);
        //Tcp线程达到上限，停止接受新的Client
        if(TcpClientCount >=MAX_CLIENT)
        {
            closesocket(TcpSocket);
            continue;
        }
        //设置传递给线程结构变量参数，创建服务线程
        TcpThreadParam Param;
        Param.socket = TcpSocket; //与客户端实际连接的套接字号
        Param.addr =TcpClientAddr;//客户端地址结构
        DWORD dwThreadId;
        CreateThread(NULL,0,TcpServeThread,&Param,0,&dwThreadId);
    }
    closesocket(ListenSocket);
    WSACleanup();
	return 0;
}
   //***********子线程
DWORD WINAPI TcpServeThread(LPVOID lpParam)
{
    printf("连接成功\n");
	char ServerTCPBuf[MAX_BUF_SIZE];
    //从线程参数中获取TCP套接字
    SOCKET TcpSocket = ((TcpThreadParam *)lpParam)->socket;
    SOCKADDR_IN TcpClientAddr = ((TcpThreadParam *)lpParam)->addr;

    //发送UDP端口号+“START”
	sprintf(ServerTCPBuf,"%5d%s",UDP_SRV_PORT,"START");
	send(TcpSocket,ServerTCPBuf,strlen(ServerTCPBuf),0);
	//在主循环中提供时间服务
	int TCPBytesReceived;
	time_t CurSysTime;
	while(TRUE)
	{
		//读取client发来的时间请求命令:"GET CUR TIME"
		memset(ServerTCPBuf,'\0',sizeof(ServerTCPBuf));
		TCPBytesReceived=recv(TcpSocket,ServerTCPBuf,sizeof(ServerTCPBuf),0);
		//TCPBytes值为0表示客户端已正常关闭连接

		if(TCPBytesReceived ==0||TCPBytesReceived ==SOCKET_ERROR)
		{
			break;
		}
		//检查收到的字符串是否为 GET CUR TIME

		time(&CurSysTime);
		memset(ServerTCPBuf,'\0',sizeof(ServerTCPBuf));
		strftime(ServerTCPBuf,sizeof(ServerTCPBuf),"%Y- %m -%d %H:%M:%S",localtime(&CurSysTime));
		send(TcpSocket,ServerTCPBuf,strlen(ServerTCPBuf),0);
	}
	closesocket(TcpSocket);
	return 0;
	//Server端UDP服务进程

	//创建UDP server Socket
}
DWORD WINAPI UdpServer(LPVOID lpParam)
{
	char ServerUDPBuf[MAX_BUF_SIZE];
	int x,i;
	char a[10];
	SOCKADDR_IN UDPClientAddr;
	//USHORT sign = ((QWEHEADER *)lpParam)->choose;
	//QWEHEADER *pIpHdr = ((QWEHEADER *)lpParam);
     //pIpHdr=(QWEHEADER *)ClientBuf;
	SOCKET UDPSrvSocket = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
	//获取本机名
	char hostname[256];
	gethostname(hostname,sizeof(hostname));
	//获取本机IP
	hostent *pHostent = gethostbyname(hostname);
	//填充本地UDP socket 地址结构
	SOCKADDR_IN UDPSrvAddr;
	memset(&UDPSrvAddr,0,sizeof(SOCKADDR_IN));
	UDPSrvAddr.sin_family = AF_INET;
	UDPSrvAddr.sin_port = htons(UDP_SRV_PORT);
	UDPSrvAddr.sin_addr = * (in_addr *)pHostent->h_addr_list[0];
	//printf("%X\n",UDPSrvAddr.sin_port);
	//printf("%X\n",UDPSrvAddr.sin_addr.S_un.S_addr);
	//绑定UDP端口
	bind(UDPSrvSocket,(sockaddr *)&UDPSrvAddr,sizeof(UDPSrvAddr));
	//printf("getlasterror=%d",WSAGetLastError());
	//UDP回显服务
	memset(a,'\0',sizeof(a));
	//连接数据库
	MYSQL mysql;
	MYSQL_RES *res;
	MYSQL_ROW row;
	char str[100];
	char *query,*s="'",*xx,*xxx;
	int flag, t;
	mysql_init(&mysql);
	if(!mysql_real_connect(&mysql, "localhost", "root", "******", "test", 0, NULL, 0)) {
		printf("Failed to connect to Mysql!\n");
		return 0;
	}else {
		printf("Connected to Mysql successfully!\n");
	}
	/*query = "select user,password from user";
	//查询，成功则返回0
	flag = mysql_real_query(&mysql, query, (unsigned int)strlen(query));
	printf("flag=%d",flag);
	if(flag) {
		printf("Query failed!\n");
		return 0;
	}else {
		printf("[%s] made...\n", query);
	}*/




	while(TRUE)
	{
		memset(ServerUDPBuf,'\0',sizeof(ServerUDPBuf));
			//接收UDP回显数据
			printf("1");
		char copy1[]="login sucessful";
		char copy[]="login failed!";
		char copy2[]="register sucessful";
		int iSockAddrLen = sizeof(sockaddr);
		memset(str,'\0',sizeof(str));
		recvfrom(UDPSrvSocket,ServerUDPBuf,sizeof(ServerUDPBuf),0,(sockaddr *)&UDPClientAddr,&iSockAddrLen);
		printf("%s\n",ServerUDPBuf);
		iSockAddrLen = sizeof(sockaddr);
                                //QWEHEADER *pIpHdr;
                                //pIpHdr=(QWEHEADER *)ServerUDPBuf;
                                if(ServerUDPBuf[0]=='1')
                                {
									for(i=1;ServerUDPBuf[i]!='&';i++)
									{a[i-1] = ServerUDPBuf[i];}
									ServerUDPBuf[i]=='\0';
								printf("username:%s\n",a);
								printf("password:%s\n",&ServerUDPBuf[i+1]);
								strcat(a,s);
								printf("a =%s",a);

								xx = "select count(*) from user where user='";
								strcpy(str,xx);
								strcat(str,a);
								xxx ="and password='";
								strcat(str,xxx);
								strcat(str,&ServerUDPBuf[i+1]);
								strcat(str,s);
								query = str;
								printf("query = %s",query);
								
								
	/*sql查询，成功则返回0*/
	flag = mysql_real_query(&mysql, query, (unsigned int)strlen(query));
	res = mysql_store_result(&mysql);
	row = mysql_fetch_row(res);
		/*mysql_num_fields返回结果集中的字段数目*/
		for(t=0; t<mysql_num_fields(res); t++)
		{
			printf("%s\t", row[t]);
		}
		printf("\n");
	char *a;
	a=row[t-1];
	//printf("flag=%d",flag);
	if(flag) {
		printf("Query failed!\n");
	}else if(a[0]=='1'){
		printf("[%s] made...\n", query);
		res = mysql_store_result(&mysql);
		printf("res = %d\n",res);
		/*while(row = mysql_fetch_row(res)) {
		//mysql_num_fields返回结果集中的字段数目
		for(t=0; t<mysql_num_fields(res); t++)
		{
			printf("%s\t", row[t]);
		}
		printf("\n");
		}*/


		memset(ServerUDPBuf,'\0',sizeof(ServerUDPBuf));
		strcpy(ServerUDPBuf,copy1);
	}
	else
	{		memset(ServerUDPBuf,'\0',sizeof(ServerUDPBuf));
		strcpy(ServerUDPBuf,copy);
	}
								}
                                if(ServerUDPBuf[0]=='2')
                               {
                               memset(ServerUDPBuf,'\0',sizeof(ServerUDPBuf));
                               strcpy(ServerUDPBuf,copy2);
                               }
		sendto(UDPSrvSocket,ServerUDPBuf,strlen(ServerUDPBuf),0,(sockaddr *)&UDPClientAddr,iSockAddrLen);
		printf("getlasterro=%d",WSAGetLastError());
	}
	return 0;
}
