#include "functions.h"

int main(int argc, char *argv[])
{
    /*reading arguments*/
    if(argc < 5 || argc > 6)
        error("[ERROR]Wrong arguments!!!\n");
    // tcp/udp
    char protocal[3];
    strncpy(protocal, argv[1], 3);
    // send/recv
    char terminal[4];
    strncpy(terminal, argv[2], 4);
    // ip
    in_addr_t ip = inet_addr(argv[3]);
    if(ip == INADDR_NONE)
        error("[ERROR]Invalid argument (ip)!!!\n");
    // port
    int port = atoi(argv[4]);
    // file path
    char file_path[256];
    if(strncmp(terminal, "send", 4) == 0)
    {
        if(argc != 6) error("[ERROR]Wrong arguments!!!\n");
        strcpy(file_path, argv[5]);
    }

    arguments param = {
        .ip = ip,
        .port = port,
        .file_path = ""
    };
    strncpy(param.file_path, file_path, strlen(file_path));

    /*transfer/receive file via given protocal*/
    if(strncmp(protocal, "tcp", 3) == 0)
    {
        if(strncmp(terminal, "send", 4) == 0)
        {
            tcp_send(param);
        }
        else if(strncmp(terminal, "recv", 4) == 0)
        {
            tcp_recv(param);
        }
        else error("[ERROR]Invalid argument (send/recv)!!!\n");
    }
    else if(strncmp(protocal, "udp", 3) == 0)
    {
        if(strncmp(terminal, "send", 4) == 0)
        {
            udp_send(param);
        }
        else if(strncmp(terminal, "recv", 4) == 0)
        {
            udp_recv(param);
        }
        else error("[ERROR]Invalid argument (send/recv)!!!\n");
    }
    else error("[ERROR]Invalid argument (tcp/udp)!!!\n");

    return 0;
}