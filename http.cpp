#include <stdio.h>
#include <stdlib.h>
#include <Winsock2.h>
#include <Iphlpapi.h>
#include <iostream>
#include <tchar.h>
#include <conio.h>
#pragma comment (lib,"Iphlpapi")
#pragma comment (lib,"Ws2_32")
#define IO_RCVALL _WSAIOW(IOC_VENDOR,1)
struct IPHEADER
{
    unsigned char Version_HeaderLength;
    unsigned char TypeOfService;
    unsigned short TotalLength;
    unsigned short Identification;
    unsigned short Flags_FragmentOffset;
    unsigned char TimeToLive;
    unsigned char Protocal;
    unsigned short HeaderChecksum;
    unsigned long SourceAddress;
    unsigned long DestAddress;
};
struct TCPHEADER
{
    USHORT usSourcePort;
    USHORT usDestPort;
    ULONG dwSeq;
    ULONG dwAck;
    UCHAR ucLength;
    UCHAR ucFlag;
    USHORT usWindow;
    USHORT usUrgent;
    UINT unMssOpt;
    USHORT usNopOpt;
    USHORT usSackOpt;
};
int main()
{
	int i;
    WSADATA wsData;
    WSAStartup(MAKEWORD(2,2),&wsData);
    SOCKET sock;
    sock=WSASocket(AF_INET,SOCK_RAW,IPPROTO_IP,NULL,0,WSA_FLAG_OVERLAPPED);
    char localName[256];
    gethostname(localName,256);
    HOSTENT *pHost;
    pHost=gethostbyname(localName);
    sockaddr_in addr_in;
    addr_in.sin_family=AF_INET;
    addr_in.sin_port=htons(8000);
    addr_in.sin_addr=*(in_addr *)pHost->h_addr_list[4];
    unsigned char * hostip=(unsigned char * )(&addr_in.sin_addr.S_un.S_addr);
    printf("ip:%d.%d.%d.%d\n",hostip[0],hostip[1],hostip[2],hostip[3]);
    bind(sock,(sockaddr*)&addr_in,sizeof(addr_in));
    DWORD dwBufferLen[10];
    DWORD dwBufferInLen=1;
    DWORD dwBytesReturned=0;
    WSAIoctl(sock,IO_RCVALL,&dwBufferInLen,sizeof(dwBufferInLen),dwBufferLen,sizeof(dwBufferLen),&dwBytesReturned,NULL,NULL);
    char Buffer[65535];
    while(1){
        int nPacketsize=recv(sock,Buffer,65535,0);
        if(nPacketsize>0){
            IPHEADER *pIpHdr;
            pIpHdr=(IPHEADER *)Buffer;
            if(pIpHdr->SourceAddress==addr_in.sin_addr.S_un.S_addr||pIpHdr->DestAddress==addr_in.sin_addr.S_un.S_addr)
            {
                TCPHEADER *tcphead;
                tcphead=(TCPHEADER *)(Buffer+20);
                USHORT destport=ntohs(tcphead->usDestPort);
                if(destport==80){
                    unsigned char * destip=(unsigned char *)(&pIpHdr->DestAddress);
                    printf("destip:%d.%d.%d.%d:\n",destip[0],destip[1],destip[2],destip[3]);
					printf("IP size:%d\n",sizeof(IPHEADER));
					printf("%s",(Buffer+20+sizeof(IPHEADER)));

                }
            }
        }
    }
    return 0;
}