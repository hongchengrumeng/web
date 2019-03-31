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

#define MAX_CLIENT 10 //ͬʱ����Ĳ�������������
#define MAX_BUF_SIZE 65535 //���շ��ͻ�������С
#define UDP_SRV_PORT 9001 //server��UDP�˿ں�
const char START_CMD[] = "START";
const char GETCURTIME_CMD[] = "GET CUR TIME";
struct TcpThreadParam   //���ݸ�TCP�̵߳Ľṹ������
{
    SOCKET socket; //�׽��ֺ�
    sockaddr_in addr;//��ַ��Ϣ
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
    //��������������TCP Socket
    SOCKET ListenSocket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    //��ȡ������
    char hostname[256];
    gethostname(hostname,sizeof(hostname));
    //��ȡ����IP��ַ
    hostent *pHostent = gethostbyname(hostname);
    //��䱾��TCP Listen Socket��ַ�ṹ
    for(int i=0;(int)(pHostent->h_addr_list[i])!=0;i++)
    {
        unsigned char * hostip1=(unsigned char * )(pHostent->h_addr_list[i]);
        printf("ip:%d.%d.%d.%d\n",hostip1[0],hostip1[1],hostip1[2],hostip1[3]);
    }
    int j;
    printf("��ѡ��ip��");
    scanf("%d",&j);
    SOCKADDR_IN ListenAddr;
    ListenAddr.sin_family = AF_INET;
    ListenAddr.sin_port = htons((u_short)atoi("3000"));
    ListenAddr.sin_addr = * (in_addr *)pHostent->h_addr_list[j];
    //��TCP�����˿�
    bind(ListenSocket,(sockaddr *)&ListenAddr,sizeof(ListenAddr));
    //����
    printf("��ʼ������\n");
    listen(ListenSocket,SOMAXCONN);
    //��һ����ѭ���н��տͻ����������󲢴����������
    SOCKET TcpSocket;
    SOCKADDR_IN TcpClientAddr;
    DWORD dwThreadId1;
	CreateThread(NULL,0,UdpServer,NULL,0,&dwThreadId1);
    while(TRUE)
    {
        //���ܿͻ�����������
        int iSockAddrLen=sizeof(sockaddr);
        TcpSocket = accept(ListenSocket,(sockaddr *)&TcpClientAddr,&iSockAddrLen);
        //Tcp�̴߳ﵽ���ޣ�ֹͣ�����µ�Client
        if(TcpClientCount >=MAX_CLIENT)
        {
            closesocket(TcpSocket);
            continue;
        }
        //���ô��ݸ��߳̽ṹ�������������������߳�
        TcpThreadParam Param;
        Param.socket = TcpSocket; //��ͻ���ʵ�����ӵ��׽��ֺ�
        Param.addr =TcpClientAddr;//�ͻ��˵�ַ�ṹ
        DWORD dwThreadId;
        CreateThread(NULL,0,TcpServeThread,&Param,0,&dwThreadId);
    }
    closesocket(ListenSocket);
    WSACleanup();
	return 0;
}
   //***********���߳�
DWORD WINAPI TcpServeThread(LPVOID lpParam)
{
    printf("���ӳɹ�\n");
	char ServerTCPBuf[MAX_BUF_SIZE];
    //���̲߳����л�ȡTCP�׽���
    SOCKET TcpSocket = ((TcpThreadParam *)lpParam)->socket;
    SOCKADDR_IN TcpClientAddr = ((TcpThreadParam *)lpParam)->addr;

    //����UDP�˿ں�+��START��
	sprintf(ServerTCPBuf,"%5d%s",UDP_SRV_PORT,"START");
	send(TcpSocket,ServerTCPBuf,strlen(ServerTCPBuf),0);
	//����ѭ�����ṩʱ�����
	int TCPBytesReceived;
	time_t CurSysTime;
	while(TRUE)
	{
		//��ȡclient������ʱ����������:"GET CUR TIME"
		memset(ServerTCPBuf,'\0',sizeof(ServerTCPBuf));
		TCPBytesReceived=recv(TcpSocket,ServerTCPBuf,sizeof(ServerTCPBuf),0);
		//TCPBytesֵΪ0��ʾ�ͻ����������ر�����

		if(TCPBytesReceived ==0||TCPBytesReceived ==SOCKET_ERROR)
		{
			break;
		}
		//����յ����ַ����Ƿ�Ϊ GET CUR TIME

		time(&CurSysTime);
		memset(ServerTCPBuf,'\0',sizeof(ServerTCPBuf));
		strftime(ServerTCPBuf,sizeof(ServerTCPBuf),"%Y- %m -%d %H:%M:%S",localtime(&CurSysTime));
		send(TcpSocket,ServerTCPBuf,strlen(ServerTCPBuf),0);
	}
	closesocket(TcpSocket);
	return 0;
	//Server��UDP�������

	//����UDP server Socket
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
	//��ȡ������
	char hostname[256];
	gethostname(hostname,sizeof(hostname));
	//��ȡ����IP
	hostent *pHostent = gethostbyname(hostname);
	//��䱾��UDP socket ��ַ�ṹ
	SOCKADDR_IN UDPSrvAddr;
	memset(&UDPSrvAddr,0,sizeof(SOCKADDR_IN));
	UDPSrvAddr.sin_family = AF_INET;
	UDPSrvAddr.sin_port = htons(UDP_SRV_PORT);
	UDPSrvAddr.sin_addr = * (in_addr *)pHostent->h_addr_list[0];
	//printf("%X\n",UDPSrvAddr.sin_port);
	//printf("%X\n",UDPSrvAddr.sin_addr.S_un.S_addr);
	//��UDP�˿�
	bind(UDPSrvSocket,(sockaddr *)&UDPSrvAddr,sizeof(UDPSrvAddr));
	//printf("getlasterror=%d",WSAGetLastError());
	//UDP���Է���
	memset(a,'\0',sizeof(a));
	//�������ݿ�
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
	//��ѯ���ɹ��򷵻�0
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
			//����UDP��������
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
								
								
	/*sql��ѯ���ɹ��򷵻�0*/
	flag = mysql_real_query(&mysql, query, (unsigned int)strlen(query));
	res = mysql_store_result(&mysql);
	row = mysql_fetch_row(res);
		/*mysql_num_fields���ؽ�����е��ֶ���Ŀ*/
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
		//mysql_num_fields���ؽ�����е��ֶ���Ŀ
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
