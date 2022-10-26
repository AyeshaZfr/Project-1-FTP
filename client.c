#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>

#define PORT 6000
#define buffer_size 100

#include "helper.definations.h"
#include "helper.readFile.c"
#define DEFINE

int main()
{
    printf("Hello !! Please Authenticate to run server commands\n");
    printf( "1. type ""USER"" followed by a space and your username.\n");
    printf("2. type ""PASS"" followed by a space and your password.\n");
    printf("QUIT to close connection at any moment.\n");
    printf("Once Authenticated\n");
    printf("this is the list of commands.\n");
    printf("STOR+ space + filename Ito send a file to the server.\n");
    printf("'RETR' + space + filename |to download a file from the server.\n");
    printf("'LIST' |to to list all the files under the current server directory.\n");
    printf("'CWD' + space + directory |to change the current server directory.\n");
    printf("'PWD' to display the current server directory.\n");
    printf("Add '!' before the last three commands to apply them locally.\n");
	// create socket
	//  AF_INET for designating IP address (domain)
	//  SOCK_STREAM for TCP connection (type)
	int server_sd = socket(AF_INET, SOCK_STREAM, 0);

	//  if server_sd returns -1  indiciating error
	if (server_sd < 0)
	{
		perror("socket:");
		exit(-1);
	}
    

	// Address of server
	struct sockaddr_in server_addr;
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	// connect
	if (connect(server_sd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		perror("connect");
		exit(-1);
	}
	printf("%s \n", SERVER_OPEN);

    read_file();

    char buffer[buffer_size];
    char* token1;
    char* token2;
    int found_user  = -1;
    int isloggedin = -1;
    int index = 0;
    
    while(isloggedin == -1)
    {
        bzero(buffer, sizeof(buffer));
        fgets(buffer, buffer_size, stdin);
        token1 = strtok(buffer, " ");
        token2 = strtok(0, "\n");
        if(strcmp( token1, "USER") == 0){
            for (int i = 0; i<num_users; i++){
                if(strcmp( token2,users[i].username ) == 0){
                    found_user = 0;
                    index = i;
                    break;
                }
            }
        }
        else{
            printf("%s \n", LOGIN_FAILED);
            continue;
        }
        if(found_user == 0){
            printf("%s \n", LOGIN_NEED_PASS);
            bzero(buffer, sizeof(buffer));
            fgets(buffer, buffer_size, stdin);
            token1 = strtok(buffer, " ");
            token2 = strtok(0, "\n");

            if(strcmp( token1, "PASS") == 0 && strcmp(token2, users[index].password) == 0){
                printf("%s \n", LOGIN_SUCCESS);
                isloggedin = 0;
                break;
            }

            else{
                printf("%s \n", LOGIN_FAILED);
            }
        }
        else{
            printf("%s \n", LOGIN_FAILED);  
        }
    }

    int bytes = send(server_sd, buffer, sizeof(buffer), 0);

	while (1)
	{
		// accept
		


	}
}
