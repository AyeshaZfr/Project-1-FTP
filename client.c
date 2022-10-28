#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>

// https://iq.opengenus.org/ls-command-in-c/
// Used for handling directory files
#include <dirent.h>

#define PORT 6000
#define directory_size 500

#include "helper.definations.h"
#include "helper.userAuth.c"
#define DEFINE

void client_user_login(int server_sd);
int list_client_files(char *current_directory);
int change_client_directory(char *current_directory, char *new_directory);
int list_client_directory(char *current_directory);
void send_to_server(char *command, char *params, int sock_fd, char *buffer);

int main()
{
    printf("Hello !! Please Authenticate to run server commands\n");
    printf("1. type "
           "USER"
           " followed by a space and your username.\n");
    printf("2. type "
           "PASS"
           " followed by a space and your password.\n");
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

    // int auth = client_user_login(server_sd);
    // while (auth != 0)
    // {
    client_user_login(server_sd);
    // }

    // get cwd
    char current_directory[1000];
    getcwd(current_directory, sizeof(current_directory));

    char buffer[buffer_size];
    char *command;
    char *params;

    while (1)
    {
        bzero(buffer, sizeof(buffer));
        printf("ftp> ");
        fgets(buffer, buffer_size, stdin);

        if (strstr(buffer, " ") == NULL)
        {
            command = strtok(buffer, "\n");
            params = "";
        }
        else
        {
            command = strtok(buffer, " ");
            params = strtok(0, "\n");
        }

        if (strcmp(command, "!LIST") == 0)
        {
            list_client_files(current_directory);
        }
        else if (strcmp(command, "!CWD") == 0)
        {
            if (strcmp(params, "") == 0)
            {
                printf("%s\n", INVALID_SEQUENCE);
                continue;
            }
            else
            {
                change_client_directory(current_directory, params);
            }
        }
        else if (strcmp(command, "!PWD") == 0)
        {
            list_client_directory(current_directory);
        }
        else if (strcmp(command, "QUIT") == 0)
        {
            break;
        }
        else if (strcmp(command, "CWD") == 0)
        {
            if (strcmp(params, "") == 0)
            {
                printf("%s\n", INVALID_SEQUENCE);
                continue;
            }
            else
            {
                send_to_server(command, params, server_sd, buffer);
            }
        }
        else if (strcmp(command, "PWD") == 0)
        {
            if (strcmp(params, "") == 0)
            {
                send_to_server(command, params, server_sd, buffer);
            }
            else
            {
                printf("%s\n", INVALID_SEQUENCE);
                continue;
            }
        }
        else
        {
            printf("%s\n", INVALID_COMMAND);
        }
    }
}

void client_user_login(int server_sd)
{

    char pass_buffer[buffer_size];
    char *token1;
    char *token2;
    int isloggedin = 1;
    int index = 0;

    while (isloggedin == 1)
    {
        bzero(pass_buffer, sizeof(pass_buffer));
        printf("ftp> ");
        fgets(pass_buffer, buffer_size, stdin);
        token1 = strtok(pass_buffer, " ");
        token2 = strtok(0, "\n");

        // if correct command
        if (strcmp(token1, "USER") == 0)
        {
            send(server_sd, token2, sizeof(token2), 0);
            bzero(pass_buffer, sizeof(pass_buffer));
            int bytes1 = recv(server_sd, pass_buffer, sizeof(pass_buffer), 0);
            printf("%s \n", pass_buffer);
        }
        // incorrect command means err thrown
        else
        {
            printf("%s\n", LOGIN_FAILED);
            continue;
        }

        if (strcmp(pass_buffer, LOGIN_NEED_PASS) == 0)
        {
            bzero(pass_buffer, sizeof(pass_buffer));
            printf("ftp> ");
            fgets(pass_buffer, buffer_size, stdin);
            token1 = strtok(pass_buffer, " ");
            token2 = strtok(0, "\n");

            if (strcmp(token1, "PASS") == 0)
            {
                // sends password
                send(server_sd, token2, sizeof(token2), 0);

                bzero(pass_buffer, sizeof(pass_buffer));

                // receives response
                recv(server_sd, pass_buffer, sizeof(pass_buffer), 0);
                printf("%s \n", pass_buffer);

                if (strcmp(pass_buffer, LOGIN_SUCCESS) == 0)
                {
                    isloggedin = 0;
                }
            }
            else
            {
                printf("%s\n", LOGIN_FAILED);
            }
        }
    }
    // return 0;
}

int list_client_files(char *current_directory)
{

    // open directory
    DIR *directory_path;
    struct dirent *file_pointer;
    directory_path = opendir(current_directory);

    if (directory_path != NULL)
    {
        while ((file_pointer = readdir(directory_path)) != NULL)
        {
            printf("%s\n", file_pointer->d_name);
        }
        closedir(directory_path);
        return 0;
    }
    else
    {
        printf("%s\n", INVALID_DIRECTORY);
    }
    return 0;
}

int change_client_directory(char *current_directory, char *new_directory)
{
    char new_path[2000];
    if (new_directory[0] == '/')
    {
        strcpy(new_path, new_directory);
    }
    else
    {
        strcat(strcat(strcpy(new_path, current_directory), "/"), new_directory);
    }

    DIR *dir = opendir(new_path);
    if (dir)
    {
        realpath(new_path, current_directory);
        printf("Local directory successfully changed to %s.\n", new_directory);
        closedir(dir);
        return 0;
    }
    else
    {
        // Directory does not exist.
        printf("%s\n", INVALID_DIRECTORY);
        return 1;
    }
}

int list_client_directory(char *current_directory)
{
    // open directory
    DIR *directory_path;
    struct dirent *file_pointer;
    directory_path = opendir(current_directory);

    if (directory_path != NULL)
    {
        printf("%s\n", current_directory);
    }
    else
    {
        printf("%s\n", INVALID_DIRECTORY);
    }
    return 0;
}

void send_to_server(char *command, char *params, int sock_fd, char *buffer)
{
    int bytes_read = buffer_size;

    // send full command to server
    if (strcmp(params, "") != 0)
    {
        char full_command[buffer_size];
        bzero(full_command, sizeof(full_command));

        strcat(strcat(full_command, command), " ");
        strcat(full_command, params);
        send(sock_fd, full_command, sizeof(full_command), 0);
    }
    else
    {
        send(sock_fd, command, sizeof(command), 0);
    }

    bzero(buffer, buffer_size);

    // recv the response from server
    bytes_read = recv(sock_fd, buffer, buffer_size, 0);
    if (bytes_read < 0)
    {
        close(sock_fd);
    }
    printf("%s\n", buffer);
}