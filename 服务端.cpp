#include <bits/stdc++.h>
#include <afx.h>
#include <WinSock2.h> 
#pragma warning (disable : 4996)

//����Ϊ�����������ڴ����ļ�
#define PORT 3214   //netstat -ano��ѯ������127.0.0.1�����ö˿�
#define SERVER_IP "127.0.0.1" //ipconfig��ѯ����ipv4
#define BUFFER_SIZE 1024
#define FILE_NAME_MAX_SIZE 512 
#define CHUNK_SIZE 1024
#pragma comment(lib, "WS2_32") 

DWORD GetFileProc(int nCurrentPos, SOCKET client, std::string filePath)
{
    CFile file;
    CString FilePath(filePath.c_str());
    if (file.Open(FilePath, CFile::modeRead | CFile::typeBinary))
    {
        int nChunkCount = 0; //�ļ�����
        if (nCurrentPos != 0)
        {
            file.Seek(nCurrentPos * CHUNK_SIZE, CFile::begin); //�ļ�ָ�������ϵ㴦
            std::cout << "�ҵ����ļ� " << nCurrentPos * CHUNK_SIZE << "\n";
        }
        int FileLen = file.GetLength();
        nChunkCount = FileLen / CHUNK_SIZE; //�ļ�����

        if (FileLen % nChunkCount != 0)
            nChunkCount++;

        send(client, (char*)&FileLen, sizeof(FileLen), 0); //�����ļ�����
        char* date = new char[CHUNK_SIZE];

        bool error = 0;
        for (int i = nCurrentPos; i < nChunkCount && !error; i++) //�Ӷϵ㴦�ֿ鷢��
        {
            int nLength;
            if (i + 1 == nChunkCount) nLength = FileLen - CHUNK_SIZE * (nChunkCount - 1);
            else nLength = CHUNK_SIZE;

            int index = 0;
            file.Read(date, CHUNK_SIZE);
            while (nLength > 0)
            {
                int ret = send(client, &date[index], nLength, 0);
                if (ret == SOCKET_ERROR)
                {
                    std::cout << "���ݴ����쳣 \n";
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
    else std::cout << "�Ҳ����ļ�\n";

    return 0;
}

int main()
{
    // ��������ʼ��һ�������(����)�ĵ�ַ�ṹ 
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.S_un.S_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // ��ʼ��socket dll 
    WSADATA wsaData;
    WORD socketVersion = MAKEWORD(2, 0);
    if (WSAStartup(socketVersion, &wsaData) != 0)
    {
        printf("socket dll��ʼ��ʧ��!");
        exit(1);
    }

    // ����socket 
    SOCKET m_Socket = socket(AF_INET, SOCK_STREAM, 0);
    if (SOCKET_ERROR == m_Socket)
    {
        printf("Socket����ʧ��!");
        exit(1);
    }

    //��socket�ͷ����(����)��ַ 
    if (SOCKET_ERROR == bind(m_Socket, (LPSOCKADDR)&server_addr, sizeof(server_addr)))
    {
        printf("��������ʧ�ܣ�%d", WSAGetLastError());
        exit(1);
    }

    //���� 
    if (SOCKET_ERROR == listen(m_Socket, 10))
    {
        printf("����������ʧ�ܣ�%d", WSAGetLastError());
        exit(1);
    }

    std::cout << "���Ƿ����\n";

    //�ȴ�ָ������ļ�
    while (1)
    {  
        std::string FilePath;
        std::cout << "�����봫���ļ�·��\n";
        std::cin >> FilePath;

        printf("���ڵȴ��ͻ��˷���ָ��...\n");
        sockaddr_in client_addr;
        int client_addr_len = sizeof(client_addr);

        SOCKET m_New_Socket = accept(m_Socket, (sockaddr*)&client_addr, &client_addr_len);
        if (SOCKET_ERROR == m_New_Socket)
        {
            printf("����������ʧ��: %d", WSAGetLastError());
            break;
        }

        char buffer[BUFFER_SIZE];
        memset(buffer, 0, BUFFER_SIZE);

        int nCurrentPos = 0;//���ܶϵ�ֵ
        if (recv(m_New_Socket, (char*)&nCurrentPos, sizeof(nCurrentPos), 0) == SOCKET_ERROR)
        {
            printf("�ͻ��˽�������ʧ��");
            break;
        }

        GetFileProc(nCurrentPos, m_New_Socket, FilePath);

        closesocket(m_New_Socket);
        std::cout << "�ļ�����ɹ�\n";
    }

    closesocket(m_Socket);
    WSACleanup();
    return 0;
}