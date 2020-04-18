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

int main()
{
	std::string filePath;
	std::cout << "���ǿͻ��ˣ������������ļ�·��\n";
	std::cin >> filePath;
	CString FilePath(filePath.c_str());

	std::string TMPF = "PosFile.temp";
	CString TMPFile(TMPF.c_str());

	// ��ʼ��socket dll 
	WSADATA wsaData;
	WORD socketVersion = MAKEWORD(2, 0);
	if (WSAStartup(socketVersion, &wsaData) != 0)
	{
		printf("����Socket��ʼ��ʧ��!");
		exit(1);
	}

	while (1)
	{
		//����socket 
		SOCKET c_Socket = socket(AF_INET, SOCK_STREAM, 0);
		if (SOCKET_ERROR == c_Socket)
		{
			printf("���󣺴���Socket�쳣!");
			exit(1);
		}

		//ָ������˵ĵ�ַ 
		sockaddr_in server_addr;
		server_addr.sin_family = AF_INET;
		server_addr.sin_addr.S_un.S_addr = inet_addr(SERVER_IP);
		server_addr.sin_port = htons(PORT);


		if (SOCKET_ERROR == connect(c_Socket, (LPSOCKADDR)&server_addr, sizeof(server_addr)))
		{
			printf("�����޷����ӵ������IP��\n");
			exit(1);
		}


		int FileLen = 0;
		int nCurrentPos = 0; //�ϵ�λ��
		UINT OpenFlags;
		CFile PosFile;
		if (PosFile.Open(TMPFile, CFile::modeRead | CFile::typeBinary))//�������ʱ�ļ����ȡ�ϵ�
		{
			PosFile.Read((char*)&nCurrentPos, sizeof(nCurrentPos)); //��ȡ�ϵ�λ��
			std::cout << "�ļ�λ���� " << nCurrentPos << "\n";
			nCurrentPos = nCurrentPos + 1;
			PosFile.Close();
			send(c_Socket, (char*)&nCurrentPos, sizeof(nCurrentPos), 0); //���Ͷϵ�ֵ
			OpenFlags = CFile::modeWrite | CFile::typeBinary;
		}
		else
		{
			send(c_Socket, (char*)&nCurrentPos, sizeof(nCurrentPos), 0);
			OpenFlags = CFile::modeWrite | CFile::typeBinary | CFile::modeCreate;
		}
		if (recv(c_Socket, (char*)&FileLen, sizeof(FileLen), 0) != 0)
		{
			int nChunkCount;
			CFile file;
			nChunkCount = FileLen / CHUNK_SIZE;
			if (FileLen % nChunkCount != 0)
			{
				nChunkCount++;
			}
			if (file.Open((LPCTSTR)FilePath, OpenFlags))
			{
				file.Seek(nCurrentPos * CHUNK_SIZE, CFile::begin);
				char* date = new char[CHUNK_SIZE];
				for (int i = nCurrentPos; i < nChunkCount; i++)
				{
					int nLeft;
					if (i + 1 == nChunkCount)
						nLeft = FileLen - CHUNK_SIZE * (nChunkCount - 1);
					else
						nLeft = CHUNK_SIZE;
					int idx = 0;
					while (nLeft > 0)
					{
						int ret = recv(c_Socket, &date[idx], nLeft, 0);
						if (ret == SOCKET_ERROR)
						{
							std::cout << "�ļ������쳣";
							return 0;
						}
						idx += ret;
						nLeft -= ret;
					}
					file.Write(date, CHUNK_SIZE);


					CFile PosFile; //���ϵ�д��PosFile.temp�ļ�
					int seekpos = i + 1;
					if (PosFile.Open((LPCTSTR)TMPFile, CFile::modeWrite | CFile::typeBinary | CFile::modeCreate));
					{
						PosFile.Write((char*)&seekpos, sizeof(seekpos));
						PosFile.Close();
					}
				}
				file.Close();
				delete[] date;
			}
			if (DeleteFile((LPCTSTR)TMPFile) != 0)
				std::cout << "�ļ��������";
		}

		WSACleanup();
		return 0;
	}
}