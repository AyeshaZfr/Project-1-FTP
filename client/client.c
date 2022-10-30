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

#include "../helper.definations.h"
#include "../helper.userAuth.c"

#define DEFINE

// A global variable for the number of file transfers so far

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

    client_user_login(server_sd);

    char buffer[buffer_size];
    char *command;
    char *params;

    // get cwd
    char current_directory[1000];
    bzero(current_directory, sizeof(current_directory));
    getcwd(current_directory, sizeof(current_directory));

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
            send_to_server(command, params, server_sd, buffer);
            close(server_sd);

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
        else if ((strcmp(command, "STOR") == 0) || (strcmp(command, "LIST") == 0) || (strcmp(command, "RETR") == 0))
        {

            send(server_sd, "PORT", 4, 0);

            int count;
            recv(server_sd, &count, sizeof(count), 0);

            char temp_request[directory_size];
            char temp_response[directory_size];
            bzero(temp_request, directory_size);
            bzero(temp_response, directory_size);

            unsigned int h_1, h_2, h_3, h_4, p_1, p_2;
            int personal_port = ntohs(server_addr.sin_port) + (count);
            char *personal_ip = inet_ntoa(server_addr.sin_addr);
            sscanf(personal_ip, "%u.%u.%u.%u", &h_1, &h_2, &h_3, &h_4);
            p_1 = personal_port / 256;
            p_2 = personal_port % 256;

            snprintf(temp_request, directory_size, PORT_REQUEST_FORMAT, h_1, h_2, h_3, h_4, p_1, p_2);

            // send port to server
            int bytes_sent = send(server_sd, temp_request, sizeof(temp_request), 0);

            int client_receiver_sd = socket(AF_INET, SOCK_STREAM, 0);
            if (client_receiver_sd < 0)
            {
                perror("Client Receiver Socket creation:");
                exit(-1);
            }
            struct sockaddr_in client_receiver_addr;                        // structure to save IP address and port
            memset(&client_receiver_addr, 0, sizeof(client_receiver_addr)); // Initialize/fill the server_address to 0
            client_receiver_addr.sin_family = AF_INET;                      // address family
            client_receiver_addr.sin_port = htons(personal_port);           // port
            client_receiver_addr.sin_addr.s_addr = inet_addr("127.0.0.1");  // htonl(INADDR_LOOPBACK); //inet_addr("127.0.0.1");
            socklen_t client_receiver_addr_size = sizeof(client_receiver_addr);
            if (setsockopt(client_receiver_sd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
            {
                perror("setsockopt(SO_REUSEADDR) failed");
            }

            // bind
            if (bind(client_receiver_sd, (struct sockaddr *)&client_receiver_addr, sizeof(client_receiver_addr)) < 0)
            {
                perror("Client Receiver Socket: bind failed");
                exit(-1);
            }
            // listen
            if (listen(client_receiver_sd, 5) < 0)
            {
                perror("Client Receiver Socket: listen failed");
                close(client_receiver_sd);
                exit(-1);
            }

            // recv the port command successsful
            bzero(temp_response, sizeof(temp_response));
            int recv_succ = recv(server_sd, temp_response, sizeof(temp_response), 0);
            printf("%s\n", temp_response);

            int server_data_sd = accept(client_receiver_sd, 0, 0);

            // new sock send LIST STOR RETR command
            send(server_data_sd, command, sizeof(command), 0);

            // recieve file status ok
            char recvv[buffer_size];
            bzero(recvv, buffer_size);
            recv(server_data_sd, recvv, sizeof(recvv), 0);
            printf("%s\n", recvv);

            if (server_data_sd < 1)
            {
                perror("Client Data Accept Error.");
                exit(-1);
            }

            if (strcmp(command, "LIST") == 0)
            {
                char FILE_BUFFER[FILE_CHUNK_SIZE];
                bzero(FILE_BUFFER, sizeof(FILE_BUFFER));
                int recieved_bytes = recv(server_data_sd, FILE_BUFFER, sizeof(FILE_BUFFER), 0);
                if (recieved_bytes == 0)
                {
                    printf("LIST OUTPUT RECIEVING FAILED.\n");
                }
                printf("%.*s", (int)strlen(FILE_BUFFER), FILE_BUFFER);
                close(server_data_sd);
            }
            // for retr command
            else if (strcmp(command, "RETR") == 0)
            {
                // open directory
                DIR *directory_path;
                struct dirent *file_pointer;
                directory_path = opendir(current_directory);
                chdir(current_directory);

                char send_file_name[buffer_size];
                strcpy(send_file_name, params);
                send(server_data_sd, send_file_name, sizeof(send_file_name), 0);
                // new sock send LIST STOR RETR command
                if (strcmp(params, "") != 0)
                {

                    // create tmp file
                    char tmp_file[buffer_size];
                    bzero(tmp_file, sizeof(tmp_file));
                    strcpy(tmp_file, "tmp_");
                    strcat(tmp_file, params);

                    char FILE_BUFFER[FILE_CHUNK_SIZE];
                    bzero(FILE_BUFFER, sizeof(FILE_BUFFER));

                    int valread = recv(server_data_sd, FILE_BUFFER, FILE_CHUNK_SIZE, 0);

                    if (strcmp(FILE_BUFFER, INVALID_DIRECTORY) == 0)
                    {
                        printf("%s\n", INVALID_DIRECTORY);
                        close(server_data_sd);
                        continue;
                    }
                    FILE *fp = fopen(tmp_file, "w");

                    if (fp == NULL)
                    {
                        perror("Failed to write file");
                        close(server_data_sd);
                        continue;
                    }
                    if (valread == FILE_CHUNK_SIZE)
                    {
                        while (valread == FILE_CHUNK_SIZE)
                        { // Big files may require multiple reads
                            fwrite(FILE_BUFFER, 1, valread, fp);
                            bzero(FILE_BUFFER, sizeof(FILE_BUFFER));
                            valread = recv(server_data_sd, FILE_BUFFER, FILE_CHUNK_SIZE, MSG_WAITALL);
                        }
                        // printf("VAL read %d\n", valread);
                        fwrite(FILE_BUFFER, 1, valread, fp);
                    }
                    else
                    {
                        fwrite(FILE_BUFFER, 1, valread, fp);
                        bzero(FILE_BUFFER, sizeof(FILE_BUFFER));
                    }

                    bzero(buffer, sizeof(buffer));
                    recv(server_sd, buffer, sizeof(buffer), 0);
                    printf("%s \n", buffer);

                    fclose(fp);
                    close(server_data_sd);
                }
                else
                {
                    printf("%s\n", INVALID_COMMAND);
                }
                close(server_data_sd);
            }
            else if (strcmp(command, "STOR") == 0)
            {
                // open directory
                DIR *directory_path;
                struct dirent *file_pointer;
                directory_path = opendir(current_directory);
                chdir(current_directory);
                char send_file_name[buffer_size];
                strcpy(send_file_name, params);
                send(server_data_sd, send_file_name, sizeof(send_file_name), 0);

                FILE *fp = fopen(params, "r");
                if (!fp)
                {
                    printf("%s\n", INVALID_DIRECTORY);
                    send(server_data_sd, INVALID_DIRECTORY, sizeof(INVALID_DIRECTORY), 0);
                    close(server_data_sd);

                    continue;
                }
                int pid = fork();
                if (pid == 0)
                {
                    char FILE_BUFFER[FILE_CHUNK_SIZE];
                    bzero(FILE_BUFFER, sizeof(FILE_BUFFER));

                    int valread = fread(FILE_BUFFER, sizeof(char), FILE_CHUNK_SIZE, fp);

                    if (valread == 0)
                    {
                        printf("%s\n", INVALID_DIRECTORY);
                        continue;
                    }

                    if (valread < FILE_CHUNK_SIZE)
                    {
                        send(server_data_sd, FILE_BUFFER, valread, 0);
                    }
                    else
                    {
                        send(server_data_sd, FILE_BUFFER, valread, 0);
                        while (valread == FILE_CHUNK_SIZE)
                        {
                            if (valread > 0)
                            {
                                valread = fread(FILE_BUFFER, sizeof(char), FILE_CHUNK_SIZE, fp);
                                send(server_data_sd, FILE_BUFFER, valread, 0);
                                bzero(FILE_BUFFER, sizeof(FILE_BUFFER));
                            }
                            else
                            {
                                printf("%s\n", INVALID_SEQUENCE);
                            }
                        }
                    }

                    fclose(fp);
                    exit(0);
                }
                bzero(buffer, sizeof(buffer));
                recv(server_sd, buffer, sizeof(buffer), 0);
                printf("%s \n", buffer);
            }
            close(server_data_sd);
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

        if (pass_buffer[5] == '\n')
        {
            printf("%s\n", LOGIN_FAILED);
            continue;
        }

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

            if (pass_buffer[5] == '\n')
            {
                printf("%s\n", LOGIN_FAILED);
                continue;
            }

            if (strcmp(token1, "PASS") == 0)
            {
                int user_index;
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
                continue;
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