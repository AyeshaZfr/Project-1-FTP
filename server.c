#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>

#define DEFINE


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
		perror("Bind failed");
		exit(-1);
	}
	// listen
	if (listen(server_sd, 5) < 0)
	{
		perror("Listen failed");
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
			while (1)
			{
				char buffer[500];

				int bytes = recv(client_sd, buffer, sizeof(buffer), 0);

				printf("[%d:%d] Received Message: %s", server_addr.sin_addr.s_addr, server_addr.sin_port, buffer);

				int bytes_2 = send(client_sd, buffer, sizeof(buffer), 0);

				if (strncmp(buffer, "BYE!\n", 5) == 0)
				{
					printf("Disconnected\n");
					break;
				}
				else
				{
					printf("[%d:%d] Send Message: %s", server_addr.sin_addr.s_addr, server_addr.sin_port, buffer);
				}
			}
			break;
		
		else
		{
			close(client_sd); // close the copy of client/secondary socket in parent process
		}
	}

	// close
	close(server_sd);
	setsockopt(server_sd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
}