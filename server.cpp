#define _CRT_SECURE_NO_WARNINGS
#undef UNICODE
#include <bits/stdc++.h>
#include <afx.h>
#include <WinSock2.h> 
#include "zip.h"
#include "unzip.h"
#include <windows.h>
#include <tchar.h>
#include <string.h>
#include<direct.h>
#include <io.h>
#include <shellapi.h>

#pragma warning (disable : 4996)

//本机为服务器，用于传输文件
//#define PORT 3215   //netstat -ano查询本机（127.0.0.1）可用端口 默认3215！！
//#define SERVER_IP "127.0.0.1" //ipconfig查询本机ipv4

#define FILE_NAME_MAX_SIZE 512 
#define CHUNK_SIZE 1024
#pragma comment(lib, "WS2_32") 

DWORD GetFileProc(int nCurrentPos, SOCKET client, std::string filePath)
{
    

    CFile file;
    CString FilePath(filePath.c_str());

    if (file.Open(FilePath, CFile::modeRead | CFile::typeBinary))
    {
        long long nChunkCount = 0; //文件块数
        if (nCurrentPos != 0)
        {
            file.Seek((long long)nCurrentPos * CHUNK_SIZE, CFile::begin); //文件指针移至断点处
            std::cout << "找到该文件 " << nCurrentPos * CHUNK_SIZE << "\n";
        }
        long long FileLen = file.GetLength();
        nChunkCount = (FileLen + CHUNK_SIZE - 1) / CHUNK_SIZE;

        send(client, (char*)&FileLen, sizeof(FileLen), 0); //发送文件长度
        char* date = new char[CHUNK_SIZE];

        bool error = 0;
        for (long long i = nCurrentPos; i < nChunkCount && !error; i++) //从断点处分块发送
        {
            long long nLength;
            if (i + 1 == nChunkCount) nLength = FileLen - CHUNK_SIZE * (nChunkCount - 1);
            else nLength = CHUNK_SIZE;

            long long index = 0;
            file.Read(date, CHUNK_SIZE);
            while (nLength > 0)
            {
                long long ret = send(client, &date[index], nLength, 0);
                if (ret == SOCKET_ERROR)
                {
                    std::cout << "数据传输异常 \n";
                    error = 1;
                    break;
                }
                nLength -= ret;
                index += ret;
            }
        }
        file.Close();
        delete[] date;
    }
    else std::cout << "找不到文件\n";

    return 0;
}




int main(int argc, char* argv[])
{
    //std::cout << argc << std::endl;
    if (argc != 4)
    {
        printf("usage : %s filename ip and port\n", argv[0]);
        exit(1);
    }
    USHORT PORT = atoi(argv[3]);
    char* SERVER_IP = argv[2];
    char* FilePath = argv[1];

    
    //FilePath = ZIP(FilePath);


    CString strRarPath = "\"WinRAR\\WinRAR.exe\" a -k -r -s -o+ -ep1 ";
    CString strfinal = strRarPath  + "abc.zip " + FilePath ;
    //printf("%s\n", strfinal);
    system(strfinal);

    FilePath = (char*)"abc.zip";
   
    std::cout << "压缩完成" << std::endl;

    // 声明并初始化一个服务端(本地)的地址结构 
    std::cout << FilePath << std::endl;
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.S_un.S_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // 初始化socket dll 
    WSADATA wsaData;
    WORD socketVersion = MAKEWORD(2, 0);
    if (WSAStartup(socketVersion, &wsaData) != 0)
    {
        printf("socket dll初始化失败!");
        exit(1);
    }

    // 创建socket 
    SOCKET m_Socket = socket(AF_INET, SOCK_STREAM, 0);
    if (SOCKET_ERROR == m_Socket)
    {
        printf("Socket创建失败!");
        exit(1);
    }

    //绑定socket和服务端(本地)地址 
    if (SOCKET_ERROR == bind(m_Socket, (LPSOCKADDR)&server_addr, sizeof(server_addr)))
    {
        printf("服务器绑定失败：%d", WSAGetLastError());
        exit(1);
    }

    //监听 
    if (SOCKET_ERROR == listen(m_Socket, 10))
    {
        printf("服务器监听失败：%d", WSAGetLastError());
        exit(1);
    }

    std::cout << "这是服务端\n";

   

    //等待指令，传输文件
    while (1)
    {
        printf("正在等待客户端发送指令...\n");
        sockaddr_in client_addr;
        int client_addr_len = sizeof(client_addr);
        
        SOCKET m_New_Socket = accept(m_Socket, (sockaddr*)&client_addr, &client_addr_len);
        
        if (SOCKET_ERROR == m_New_Socket)
        {
            printf("服务器接收失败: %d", WSAGetLastError());
            break;
        }
        

        long long nCurrentPos = 0;//接受断点值
        if (recv(m_New_Socket, (char*)&nCurrentPos, sizeof(nCurrentPos), 0) == SOCKET_ERROR)
        {
            printf("客户端接收数据失败");
            break;
        }
        

        GetFileProc(nCurrentPos, m_New_Socket, FilePath);

        closesocket(m_New_Socket);
        std::cout << "文件传输成功\n";
    }

    closesocket(m_Socket);
    WSACleanup();
    return 0;
}