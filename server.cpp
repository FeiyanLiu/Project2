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

#pragma warning (disable : 4996)

//����Ϊ�����������ڴ����ļ�
//#define PORT 3215   //netstat -ano��ѯ������127.0.0.1�����ö˿� Ĭ��3215����
//#define SERVER_IP "127.0.0.1" //ipconfig��ѯ����ipv4

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
            file.Seek((long long)nCurrentPos * CHUNK_SIZE, CFile::begin); //�ļ�ָ�������ϵ㴦
            std::cout << "�ҵ����ļ� " << nCurrentPos * CHUNK_SIZE << "\n";
        }
        int FileLen = file.GetLength();
        nChunkCount = (FileLen + CHUNK_SIZE - 1) / CHUNK_SIZE;

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


void show_file(char path[], char root[], HZIP hz, int level = 0)
{
    char find_path[FILE_NAME_MAX_SIZE];
    char dir_path[FILE_NAME_MAX_SIZE];
    sprintf(find_path, "%s*", path);
    WIN32_FIND_DATA FindFileData;
    HANDLE hFind;
    bool bContinue = true;
    hFind = FindFirstFile(find_path, &FindFileData);
    if (hFind == INVALID_HANDLE_VALUE)
        return;
    while (bContinue)
    {
        if (_stricmp(FindFileData.cFileName, "..") && _stricmp(FindFileData.cFileName, "."))
        {
            char* name = (char*)malloc(strlen(path) + strlen(FindFileData.cFileName));

            //cout << name<<endl;
            //cout <<FindFileData.cFileName << endl;
            for (int i = strlen(root); i < strlen(path); i++)
            {
                dir_path[i - strlen(root)] = path[i];
            }
            dir_path[strlen(path) - strlen(root)] = '\0';
            //cout << dir_path << endl;

            sprintf(name, "%s%s", path, FindFileData.cFileName);
            char* out_path = (char*)malloc(strlen(dir_path) + strlen(FindFileData.cFileName));
            sprintf(out_path, "%s%s", dir_path, FindFileData.cFileName);
            //cout << name << endl;
            ZipAdd(hz, out_path, name);

            if (FindFileData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
            {
                //cout<<path<<FindFileData.cFileName<<endl;
                sprintf(find_path, "%s%s\\", path, FindFileData.cFileName);
                show_file(find_path, root, hz, level + 1);
            }
        }
        bContinue = FindNextFile(hFind, &FindFileData);
    }
}

char* ZIP(char* FilePath)
{

    char* folderPath = new char[FILE_NAME_MAX_SIZE];
    char* zipPath = new char[FILE_NAME_MAX_SIZE];
    char* saveZip = new char[FILE_NAME_MAX_SIZE];

  
    bool should_change = true;
    char* szPath = FilePath;
    //char szPath[] = "G:/CN-project2/compress (2)/compress/compress/abc/";
    //char szPath[] = "G:/CN-project2/compress (2)/compress/compress/test.txt";
    int szLength = strlen(szPath);
    std::string directory;
    char zp[] = ".zip";
    bool tk = 1;
    for (int i = 0; i < 4; i++)
    {
        if (zp[i] != szPath[szLength - 4 + i])
        {
            tk = 0;
            break;
        }
    }

    if(tk)
    {
        std::string SzPath(szPath);
        const size_t last_slash_idx = SzPath.rfind('/');
        if (std::string::npos != last_slash_idx)
        {
            directory = SzPath.substr(0, last_slash_idx);
        }
        directory += "/";
        SzPath.erase(0, directory.length());

        directory += "transTEMP";
        char newFolder[FILE_NAME_MAX_SIZE];
        char newFile[FILE_NAME_MAX_SIZE];
        SzPath = directory + '/' + SzPath;
        //std::cout << SzPath << std::endl;
        strncpy(newFolder, directory.c_str(), directory.length() + 1);
        strncpy(newFile, SzPath.c_str(), SzPath.length() + 1);

        if (0 != access(directory.c_str(), 0))
        {
            // if this folder not exist, create a new one.
            mkdir(directory.c_str());   // ���� 0 ��ʾ�����ɹ���-1 ��ʾʧ��
            //���� ::_mkdir  ::_access Ҳ�У���֪��ʲô��˼
        }
        strncpy(folderPath, (directory + "/").c_str(), (directory + "/").length() + 1);

        directory += ".zip";
        CopyFile(szPath, newFile, false);


        std::cout << folderPath << std::endl;

        strncpy(zipPath, directory.c_str(), directory.length() + 1);
        std::cout << "ѹ����....." << std::endl;
        HZIP hz = CreateZip((TCHAR*)(zipPath), 0);
        show_file(folderPath, folderPath, hz);
        CloseZip(hz);
        return zipPath;

    }

 

    for (int i = 0; i < szLength; i++)
    {
        if (szPath[i] == '.') {
            should_change = false;
            break;
        }
    }
  
    if (should_change)
    {
           if (szPath[strlen(szPath) - 1] != '/')
           {
               strcpy(zipPath, szPath);
               int zipLength = strlen(zipPath);
               zipPath[zipLength + 4] = zipPath[zipLength];

               zipPath[zipLength] = '.';
               zipPath[zipLength + 1] = 'z';
               zipPath[zipLength + 2] = 'i';
               zipPath[zipLength + 3] = 'p';

               int temp = strlen(szPath);
               szPath[temp + 1] = szPath[temp];
               szPath[temp] = '/';


           }
           else
           {
               strcpy(zipPath, szPath);
               int zipLength = strlen(zipPath);
               zipPath[zipLength + 3] = zipPath[zipLength];

               zipPath[zipLength - 1] = '.';
               zipPath[zipLength] = 'z';
               zipPath[zipLength + 1] = 'i';
               zipPath[zipLength + 2] = 'p';
           }

           std::cout << "ѹ����....." << std::endl;
           HZIP hz = CreateZip((TCHAR*)(zipPath), 0);
           show_file(szPath, szPath, hz);
           CloseZip(hz);
           return zipPath;
       }

       else
       {
           int index = 0;
           int formatindex = 0;
           int szLength = strlen(szPath);
           for (int i = szLength - 1; i >= 0; i--)
           {
               if (szPath[i] == '.')
               {
                   index = i;
                   break;
               }
           }
           for (int i = szLength - 1; i >= 0; i--)
           {
               if (szPath[i] == '/')
               {
                   formatindex = i;
                   break;
               }
           }

           strcpy(zipPath, szPath);
           int zipLength = strlen(zipPath);
           //zipPath[] = zipPath[zipLength];


           zipPath[index + 1] = 'z';
           zipPath[index + 2] = 'i';
           zipPath[index + 3] = 'p';
           zipPath[index + 4] = '\0';
           for (int i = formatindex + 1; i < szLength; i++)
           {
               saveZip[i - formatindex - 1] = szPath[i];
           }
           saveZip[szLength - formatindex - 1] = '\0';

           std::cout << "ѹ����....." << std::endl;
           HZIP hz = CreateZip((TCHAR*)(zipPath), 0);
           ZipAdd(hz, (TCHAR*)(saveZip), (TCHAR*)(szPath));
           
           CloseZip(hz);
           return zipPath;
       }
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

    
    FilePath = ZIP(FilePath);
    std::cout << "ѹ�����" << std::endl;

    // ��������ʼ��һ�������(����)�ĵ�ַ�ṹ 
    std::cout << FilePath << std::endl;
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
        printf("���ڵȴ��ͻ��˷���ָ��...\n");
        sockaddr_in client_addr;
        int client_addr_len = sizeof(client_addr);
        
        SOCKET m_New_Socket = accept(m_Socket, (sockaddr*)&client_addr, &client_addr_len);
        
        if (SOCKET_ERROR == m_New_Socket)
        {
            printf("����������ʧ��: %d", WSAGetLastError());
            break;
        }
        

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