#include "functions.h"

/*print error messages and end the process*/
void error(const char *msg)
{
    perror(msg);
    exit(1);
}

/*send file via tcp*/
void tcp_send(arguments arg)
{  
    /*variable declaration*/ 
    int sockfd, cli_sockfd;
    struct sockaddr_in serv_info, cli_info;
    socklen_t cli_info_len = sizeof(cli_info);
    struct stat file_stat;
    FILE *file_ptr;
    int n_bytes, trans_bytes = 0, quarter = 0;
    char buf[BUFFER_SIZE] = {};  
    time_t now;
    clock_t timer_start, timer_end;

    /*create tcp socket*/
    if((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
        error("[ERROR]Can't create socket (server side)!!!\n");

    bzero(&serv_info, sizeof(serv_info)); // initialize
    serv_info.sin_family = AF_INET; // ipv4
    serv_info.sin_addr.s_addr = INADDR_ANY; // ip address
    serv_info.sin_port = htons(arg.port); // port

    /*bind*/
    if(bind(sockfd, (struct sockaddr *)&serv_info, sizeof(serv_info)) < 0) 
        error("[ERROR]Failed on binding!!!\n");
    
    /*listen*/
    if(listen(sockfd, BACKLOG) < 0)
        error("[ERROR]Failed on listening!!!\n");

    /*accept*/
    if((cli_sockfd = accept(sockfd, (struct sockaddr *)&cli_info, &cli_info_len)) < 0)
        error("[ERROR]Failed on accepting!!!\n");

    /*prepare file for transfer*/
    if(lstat(arg.file_path, &file_stat) < 0)
        error("[ERROR]Failed to get file status!!!\n");

    /*open file*/
    if((file_ptr = fopen(arg.file_path, "rb")) == NULL)
        error("[ERROR]Failed to open file!!!\n");

    /*send file info*/
    fileInfo file_info = {
        .size = (uint64_t)file_stat.st_size
    };
    strncpy(file_info.path, arg.file_path, sizeof(arg.file_path));
    if((n_bytes = write(cli_sockfd, &file_info, sizeof(fileInfo))) < 0)
        error("[ERROR]Failed to send file info(path)!!!\n");

    time(&now);
    printf("0%%, %s", ctime(&now));
    timer_start = clock();
    /*send file by cutting into serveral parts*/
    while(!feof(file_ptr))
    {   
        // read data from file
        n_bytes = fread(buf, sizeof(char), sizeof(buf), file_ptr);
		// send data to client
        n_bytes = write(cli_sockfd, buf, n_bytes);

        // calculate transfer progress
        trans_bytes += n_bytes;
        int percent = ((double)trans_bytes/(double)file_stat.st_size)*100 + 0.5;

        if(percent == 25 && quarter == 0){
            time(&now);
            printf("25%%, %s", ctime(&now));
            quarter++;    
        }
        if(percent == 50 && quarter == 1){
            time(&now);
            printf("50%%, %s", ctime(&now));
            quarter++;    
        }
        if(percent == 75 && quarter == 2){
            time(&now);
            printf("75%%, %s", ctime(&now));
            quarter++;    
        }
    }

    timer_end = clock();
    if(quarter == 3 || file_stat.st_size < BUFFER_SIZE){
        time_t now;
        time(&now);
        printf("100%%, %s", ctime(&now));    
    }

    /*print transfer info*/
    printf("Total trans time: %ldms\n", (timer_end - timer_start) / (CLOCKS_PER_SEC/1000));
    printf("File size: %ldMb\n", file_stat.st_size / 1048576);

    /*end of connection*/
    close(cli_sockfd);
	close(sockfd);
    fclose(file_ptr);
}

/*receive file via tcp*/
void tcp_recv(arguments arg)
{
    /*variable declaration*/
    int sockfd;
    struct sockaddr_in serv_info;
    int n_bytes, trans_bytes = 0, quarter = 0;
    char buf[BUFFER_SIZE] = {};
    char file_path[256];
    uint64_t file_size;
    fileInfo file_info;
    FILE *file_ptr;
    time_t now;
    clock_t timer_start, timer_end;

    /*create tcp socket*/
    if((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
        error("[ERROR]Can't create socket (client side)!!!\n");

    bzero(&serv_info, sizeof(serv_info)); // initialize
    serv_info.sin_family = AF_INET; // ipv4
    serv_info.sin_addr.s_addr = arg.ip; // ip address
    serv_info.sin_port = htons(arg.port); // port

    /*connect to server*/
    if(connect(sockfd, (struct sockaddr *)&serv_info, sizeof(serv_info)) < 0)
        error("[ERROR]Failed to connect!!!\n");
    
    /*receive file info*/
    if((n_bytes = read(sockfd, &file_info, sizeof(fileInfo))) < 0)
        error("[ERROR]Failed to receive file info(path)!!!\n");

    strncpy(file_path, file_info.path, sizeof(file_info.path));
    file_size = file_info.size;

    /*create file*/
    strcat(file_path, "_receive");
    if((file_ptr = fopen(file_path, "wb")) == NULL)
        error("[ERROR]Failed to open file!!!\n");

    
    time(&now);
    printf("0%%, %s", ctime(&now));
    timer_start = clock();
    /*receive file*/
    while(1){
        // get data
		n_bytes = read(sockfd, buf, sizeof(buf));
		if(n_bytes == 0){
			break;
		}
        // write data to file
		n_bytes = fwrite(buf, sizeof(char), n_bytes, file_ptr);

        // calculate transfer progress
		trans_bytes += n_bytes;
        int percent = ((double)trans_bytes/(double)file_size)*100 + 0.5;

        if(percent == 25 && quarter == 0){
            time(&now);
            printf("25%%, %s", ctime(&now));
            quarter++;    
        }
        if(percent == 50 && quarter == 1){
            time(&now);
            printf("50%%, %s", ctime(&now));
            quarter++;    
        }
        if(percent == 75 && quarter == 2){
            time(&now);
            printf("75%%, %s", ctime(&now));
            quarter++;    
        }
	}

    timer_end = clock();
    if(quarter == 3 || file_size < BUFFER_SIZE){
        time(&now);
        printf("100%%, %s", ctime(&now));    
    }

    /*print transfer info*/
    printf("Total trans time: %ldms\n", (timer_end - timer_start) / (CLOCKS_PER_SEC/1000));
    printf("File size: %ldMb\n", file_size / 1048576);

    /*end of connection*/
    close(sockfd);
    fclose(file_ptr);
}

/****
TODO: solve the stuck problem when increasing buffer size (due to loss of package)
****/
void udp_send(arguments arg)
{
    /*variable declaration*/
    int sockfd;
    struct sockaddr_in info;
    struct stat file_stat;
    FILE *file_ptr;
    int n_bytes, trans_bytes = 0, quarter = 0;
    char buf[BUFFER_SIZE] = {};
    socklen_t info_len = sizeof(info);
    time_t now;
    clock_t timer_start, timer_end;

    /*create udp socket*/
    if((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
        error("[ERROR]Can't create socket (server side)!!!\n");

    bzero(&info, sizeof(info)); // initialize
    info.sin_family = AF_INET; // ipv4
    info.sin_addr.s_addr = INADDR_ANY; // ip address
    info.sin_port = htons(arg.port); // port

    /*bind*/
    if(bind(sockfd, (struct sockaddr *)&info, sizeof(info)) < 0) 
        error("[ERROR]Failed on binding!!!\n");

    /*prepare file for transfer*/
    if(lstat(arg.file_path, &file_stat) < 0)
        error("[ERROR]Failed to get file status!!!\n");
    
    /*open file*/
    if((file_ptr = fopen(arg.file_path, "rb")) == NULL)
        error("[ERROR]Failed to open file!!!\n");

    /*catch transfer request*/
    n_bytes = recvfrom(sockfd, buf, BUFFER_SIZE, 0, (struct sockaddr *)&info, &info_len);
    
    /*send file info*/
    if(strcmp(buf, "Hi") == 0)
    {
        fileInfo file_info = {
            .size = (uint64_t)file_stat.st_size
        };
        strncpy(file_info.path, arg.file_path, sizeof(arg.file_path));

        sendto(sockfd, &file_info, sizeof(fileInfo), 0, (struct sockaddr *)&info, sizeof(info));
    }
    else 
        error("[ERROR]Wrong transfer request(info)!!!\n");
    
    
    time(&now);
    printf("0%%, %s", ctime(&now));
    timer_start = clock();
    /*send file by cutting into serveral parts*/
    while(!feof(file_ptr))
    {
        // read data from file
        n_bytes = fread(buf, sizeof(char), sizeof(buf), file_ptr);
        // send data
		sendto(sockfd, buf, n_bytes, 0, (struct sockaddr *)&info, sizeof(info));
        // printf("%d %ld\n", n_bytes, sizeof(buf));
            
        // calculate transfer progress
        trans_bytes += n_bytes;
        int percent = ((double)trans_bytes/(double)file_stat.st_size)*100 + 0.5;

        if(percent == 25 && quarter == 0){
            time(&now);
            printf("25%%, %s", ctime(&now));
            quarter++;    
        }
        if(percent == 50 && quarter == 1){
            time(&now);
            printf("50%%, %s", ctime(&now));
            quarter++;    
        }
        if(percent == 75 && quarter == 2){
            time(&now);
            printf("75%%, %s", ctime(&now));
            quarter++;    
        }    
        
        bzero(buf, BUFFER_SIZE);
    }    

    /*send transfer end request*/    
    sendto(sockfd, "End", 3, 0, (struct sockaddr *)&info, sizeof(info));

    timer_end = clock();
    if(quarter == 3 || file_stat.st_size < BUFFER_SIZE){
        time_t now;
        time(&now);
        printf("100%%, %s", ctime(&now));    
    }

    /*print transfer info*/
    printf("Total trans time: %ldms\n", (timer_end - timer_start) / (CLOCKS_PER_SEC/1000));
    printf("File size: %ldMb\n", file_stat.st_size / 1048576);

    close(sockfd);
    fclose(file_ptr);
}

void udp_recv(arguments arg)
{
    /*variable declaration*/
    int sockfd;
    struct sockaddr_in serv_info;
    int n_bytes, trans_bytes = 0, quarter = 0;
    char buf[BUFFER_SIZE] = {};
    char file_path[256];
    uint64_t file_size;
    fileInfo file_info;
    FILE *file_ptr;
    socklen_t info_len = sizeof(serv_info);
    time_t now;
    clock_t timer_start, timer_end;

    /*create udp socket*/
    if((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
        error("[ERROR]Can't create socket (client side)!!!\n");

    bzero(&serv_info, sizeof(serv_info)); // initialize
    serv_info.sin_family = AF_INET; // ipv4
    serv_info.sin_addr.s_addr = INADDR_ANY; // ip address
    serv_info.sin_port = htons(arg.port); // port

    /*request data transfer*/
    sendto(sockfd, "Hi", 2, 0, (struct sockaddr *)&serv_info, info_len);

    /*receive file info*/
    n_bytes = recvfrom(sockfd, &file_info, sizeof(fileInfo), 0, (struct sockaddr *)&serv_info, &info_len);
    strncpy(file_path, file_info.path, sizeof(file_info.path));
    file_size = file_info.size;

    /*open file*/
    strcat(file_path, "_receive");
    if((file_ptr = fopen(file_path, "wb")) == NULL)
        error("[ERROR]Failed to open file!!!\n");

    time(&now);
    printf("0%%, %s", ctime(&now));
    timer_start = clock();
    /*receive file*/
    while(1){
        bzero(buf, BUFFER_SIZE);
        
        // receive data
		n_bytes = recvfrom(sockfd, buf, BUFFER_SIZE, 0, (struct sockaddr *)&serv_info, &info_len);    
        //printf("%d %ld\n", n_bytes, sizeof(buf));

        if(strcmp(buf, "End") == 0){
            /*End of transfer*/
            // printf("End \n");
			break;
		}

        // write into file
		n_bytes = fwrite(buf, sizeof(char), n_bytes, file_ptr);
		
        // calculate transfer progress
        trans_bytes += n_bytes;
        int percent = ((double)trans_bytes/(double)file_size)*100 + 0.5;

        if(percent == 25 && quarter == 0){
            time(&now);
            printf("25%%, %s", ctime(&now));
            quarter++;    
        }
        if(percent == 50 && quarter == 1){
            time(&now);
            printf("50%%, %s", ctime(&now));
            quarter++;    
        }
        if(percent == 75 && quarter == 2){
            time(&now);
            printf("75%%, %s", ctime(&now));
            quarter++;    
        }
	}

    timer_end = clock();
    if(quarter == 3 || file_size < BUFFER_SIZE){
        time(&now);
        printf("100%%, %s", ctime(&now));    
    }

    /*print transfer info*/
    printf("Total trans time: %ldms\n", (timer_end - timer_start) / (CLOCKS_PER_SEC/1000));
    printf("File size: %dMb\n", trans_bytes / 1048576);
    printf("Packet loss rate: %2f %%\n", 100 - ((double)trans_bytes / (double)file_size * 100));
    
    /*end of connection*/
    close(sockfd);
    fclose(file_ptr);
}
