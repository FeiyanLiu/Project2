#include "head.h"

//#define PORT 8087



unsigned long GetFileProc(long long nCurrentPos, int client, std::string FilePath)
{
    FILE *file = fopen(FilePath.c_str(), "rb");

    if (file != NULL)
    {
        long long nChunkCount = 0; //文件块数
        fseek(file, 0, SEEK_END);
        long long FileLen = ftell(file);
        fseek(file, (long long)nCurrentPos * CHUNK_SIZE, 0);
        if (nCurrentPos != 0)
        {
            //fseek(file, nCurrentPos * CHUNK_SIZE, 0);
            std::cout << "WE GOT THE FILE " << nCurrentPos * CHUNK_SIZE << "\n";
        }
        //FileLen -= nCurrentPos * CHUNK_SIZE;
        nChunkCount = (FileLen + CHUNK_SIZE - 1) / CHUNK_SIZE; //文件块数

        send(client, (char*)&FileLen, sizeof(FileLen), 0); //发送文件长度
        char* date = new char[CHUNK_SIZE];

        bool error = 0;
        for (long long i = nCurrentPos; i < nChunkCount && !error; i++) //从断点处分块发送
        {
            long long nLength;
            if (i + 1 == nChunkCount) nLength = FileLen - CHUNK_SIZE * (nChunkCount - 1);
            else nLength = CHUNK_SIZE;
            
            long long index = 0;
            
            fread(date, sizeof(char), CHUNK_SIZE, file);
            while (nLength > 0)
            {
                long long ret = send(client, &date[index], nLength, 0);
                if (ret == -1)
                {
                    std::cout << "DATA TRANS ERROR \n";
                    error = 1;
                    break;
                }
                nLength -= ret;
                index += ret;
            }
        }
        fclose(file);
        delete[] date;
    }
    else printf("CAN NOT FIND THE FILE\n");
    return 0;
}



int main(int argc, char* argv[])
{
    if (argc != 4)
    {
        printf("please check your input\n");
        exit(1);
    }
    char* IP = argv[2];
    unsigned short PORT = atoi(argv[3]);
    std::string FilePath;
    FilePath = argv[1];

    std::string Trans_FilePath;//压缩文件位置
    Trans_FilePath = "abc.zip";

    bool solo_file = false;

    for (long long i = 0; i < FilePath.length(); i++)
    {
        if (FilePath[i] == '.')
        {
            solo_file = true;
            break;
        }
        
    }
    if (solo_file)//非文件夹
    {
        std::string command = "zip -r abc.zip ";
        command += FilePath;
        system(command.c_str());

    }
    else//文件夹
    {
        std::string command = "zip -r abc.zip ";
        if (FilePath[FilePath.length()] != '/')
        {
            FilePath += "/*";
        }
        else { FilePath += '*'; }
        command += FilePath;
        system(command.c_str());
    }


    struct sockaddr_in s_addr, c_addr;
    int s_sockfd, c_sockfd;
    int c_addr_len = sizeof(struct sockaddr_in);
    int s_addr_len = sizeof(struct sockaddr_in);
    s_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (s_sockfd == -1) strerr("socket error");
    memset(&s_addr, 0, s_addr_len);

    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(PORT);
    s_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int opt = 1;
    setsockopt(s_sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (bind(s_sockfd, (struct sockaddr*) & s_addr, s_addr_len) == -1)
        strerr("bind error");
    if (listen(s_sockfd, 5) == -1)
        strerr("listen error");


    while (1)
    {
        printf("waiting for command........\n");
        c_sockfd = accept(s_sockfd, (struct sockaddr*) & c_addr, (socklen_t*)&c_addr_len);
        if (c_sockfd == -1) strerr("accept error");


        long long nCurrentPos = 0;//接受断点值
        if (recv(c_sockfd, (char*)&nCurrentPos, sizeof(nCurrentPos), 0) == -1)
        {
            printf("client failed to get the data");
            break;
        }

        GetFileProc(nCurrentPos, c_sockfd, Trans_FilePath);

        std::cout << "file trans successfully\n";
    }
    close(c_sockfd);
    return 0;
}