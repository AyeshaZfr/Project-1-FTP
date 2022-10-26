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
#include "helper.userAuth.c"
#define DEFINE

void server_user_login(int server_sd);

int main()
{
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
	// if no error
	// setsock
	int value = 1;

	struct sockaddr_in server_addr;
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(6000);
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	// bind
	if (bind(server_sd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		perror("bind failed");
		exit(-1);
	}
	// listen
	if (listen(server_sd, 5) < 0)
	{
		perror("listen failed");
		close(server_sd);
		exit(-1);
	}

	while (1)
	{
		// accept returns socket used for this call
		int client_sd = accept(server_sd, 0, 0);
		printf("Connected \n");

		int pid = fork();
		if (pid == 0)
		{

			close(server_sd); // close the copy of server/master socket in child process
            server_user_login(client_sd);
			while (1)
			{
				char buffer[buffer_size];



				int bytes = recv(client_sd, buffer, sizeof(buffer), 0);

				printf("[%d:%d] Received Message: %s", server_addr.sin_addr.s_addr, server_addr.sin_port, buffer);

				int bytes_2 = send(client_sd, buffer, sizeof(buffer), 0);

			}
			break;
		}
		else
		{
			close(client_sd); // close the copy of client/secondary socket in parent process
		}
	}

	// close
	close(server_sd);
	setsockopt(server_sd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
}

void server_user_login(int client_sd){

    char pass_buffer[buffer_size];
    char* token2;
    int isloggedin = -1;
    int user_found = -1;
    read_file();
    
    while(isloggedin == -1)
    {
        char pass_buffer[buffer_size];

        int bytes = recv(client_sd, pass_buffer, sizeof(pass_buffer), 0);

        for (int i = 0; i<num_users; i++){

            if (sizeof(pass_buffer) == 0){
                send(client_sd, LOGIN_FAILED, sizeof(LOGIN_FAILED), 0);
                break;
            }
            
            if(strcmp(pass_buffer, users[i].username ) == 0){
                send(client_sd, LOGIN_NEED_PASS, sizeof(LOGIN_NEED_PASS), 0);
            }
            else{
                if (i == num_users-1){
                    send(client_sd, LOGIN_FAILED, sizeof(LOGIN_FAILED), 0);
                    break;
                }else{
                    continue;
                }
            }
            char pass_buffer[buffer_size];
            recv(client_sd, pass_buffer, sizeof(pass_buffer), 0);

            if(strcmp(pass_buffer, users[i].password) == 0){
                send(client_sd, LOGIN_SUCCESS, sizeof(LOGIN_SUCCESS), 0);
                isloggedin = 0;
                break;
            }
            else{
                send(client_sd, LOGIN_FAILED, sizeof(LOGIN_FAILED), 0);
                break;
            }
        }      
    }
    printf("exitted\n");
       
        // if(found_user == 0){
        //     printf("%s \n", LOGIN_NEED_PASS);
        //     bzero(pass_buffer, sizeof(pass_buffer));
        //     fgets(pass_buffer, buffer_size, stdin);
        //     token1 = strtok(pass_buffer, " ");
        //     token2 = strtok(0, "\n");

        //     if(strcmp( token1, "PASS") == 0 && strcmp(token2, users[index].password) == 0){
        //         printf("%s \n", LOGIN_SUCCESS);
        //         isloggedin = 0;
        //         break;
        //     }

        //     else{
        //         printf("%s \n", LOGIN_FAILED);
        //     }
        // }
        // else{
        //     printf("%s \n", LOGIN_FAILED);  
        // }

}