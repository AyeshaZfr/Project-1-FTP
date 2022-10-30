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
// #include <errno.h>

#define PORT 6000
#define directory_size 500
#define MAX_CLIENTS 3

#include "helper.definations.h"
#include "helper.userAuth.c"

#define DEFINE

unsigned int transfer_count = 0;

void server_user_login(int server_sd);
void serve_client(int client_fd, int user_index, struct sockaddr_in *client_address_ptr);

int main()
{
    read_file();
    // create  and bind socket

    int sd, max_fd, activity, new_socket;
    int client_socket[MAX_CLIENTS];

    int server_sd = socket(AF_INET, SOCK_STREAM, 0);

    //  if server_sd returns -1  indiciating error
    if (server_sd < 0)
    {
        perror("socket:");
        exit(-1);
    }
    // if no error
    // setsock
    if (setsockopt(server_sd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
    {
        perror("setsock");
        return -1;
    }

    // bind
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    socklen_t server_addr_size = sizeof(server_addr);
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

    // initialise all client_socket[] to 0 so not checked
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        client_socket[i] = 0;
    }

    fd_set readfds;

    while (1)
    {
        // clear the set socket
        FD_ZERO(&readfds);

        // add master socket to set
        FD_SET(server_sd, &readfds);
        max_fd = server_sd;

        // add child sockets to set
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            // socket descriptor
            sd = client_socket[i];

            // if valid socket descriptor then add to read list
            if (sd > 0)
            {
                FD_SET(sd, &readfds);
            }

            // highest file descriptor number, need it for the select function
            if (sd > max_fd)
            {
                max_fd = sd;
            }
        }

        activity = select(max_fd + 1, &readfds, NULL, NULL, NULL);
        if (activity < 0)
        {
            perror("select");
        }

        if (FD_ISSET(server_sd, &readfds))
        {
            new_socket = accept(server_sd,
                                (struct sockaddr *)&server_addr, (socklen_t *)&server_addr_size);

            if (new_socket < 0)

            {
                perror("accept");
                exit(EXIT_FAILURE);
            }
            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                // if position is empty
                if (client_socket[i] == 0)
                {
                    client_socket[i] = new_socket;
                    break;
                }
            }
        }

        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            sd = client_socket[i];

            if (FD_ISSET(sd, &readfds))
            {

                int user_index = get_user_index(sd);

                // // for data connection
                // struct sockaddr_in *client_address_ptr;

                if (users[user_index].authenticated == 0)
                {
                    server_user_login(sd);
                }
                else
                {
                    struct sockaddr_in client_address_ptr;
                    serve_client(sd, user_index, &client_address_ptr);
                }
            }
        }
    }
}

void server_user_login(int client_sd)
{
    printf("I come heres \n");

    char *token2;
    int isloggedin = -1;
    int user_found = -1;

    while (isloggedin == -1)
    {
        char pass_buffer[buffer_size];
        bzero(pass_buffer, sizeof(pass_buffer));
        int bytes = recv(client_sd, pass_buffer, sizeof(pass_buffer), 0);
        for (int i = 0; i < num_users + 1; i++)
        {

            if (sizeof(pass_buffer) == 0)
            {
                send(client_sd, LOGIN_FAILED, sizeof(LOGIN_FAILED), 0);
                continue;
            }

            if (strcmp(pass_buffer, users[i].username) == 0)
            {
                send(client_sd, LOGIN_NEED_PASS, sizeof(LOGIN_NEED_PASS), 0);
                char pass_buffer[buffer_size];
                recv(client_sd, pass_buffer, sizeof(pass_buffer), 0);

                if (strcmp(pass_buffer, users[i].password) == 0)
                {

                    send(client_sd, LOGIN_SUCCESS, sizeof(LOGIN_SUCCESS), 0);
                    char current_directory[buffer_size];
                    bzero(current_directory, sizeof(current_directory));
                    getcwd(current_directory, sizeof(current_directory));
                    strcpy(users[i].directory, current_directory);
                    users[i].server_sd = client_sd;
                    int authenticated = 1;
                    users[i].authenticated = authenticated;
                    isloggedin = 0;
                    break;
                }
                else
                {
                    send(client_sd, LOGIN_FAILED, sizeof(LOGIN_FAILED), 0);
                    break;
                }
            }
        }
    }
}
void serve_client(int client_sd, int user_index, struct sockaddr_in *client_address_ptr)
{

    char buffer[directory_size];
    bzero(buffer, sizeof(buffer));
    char *command;
    char *params;

    recv(client_sd, buffer, sizeof(buffer), 0);

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

    printf("Command %s\n", command);

    // get users CWD
    char current_directory[buffer_size];
    strcpy(current_directory, users[user_index].directory);

    if (strcmp(command, "QUIT") == 0)
    {

        printf("Closing Client Connection.\n");

        send(client_sd, SERVER_CLOSE, sizeof(SERVER_CLOSE), 0);
    }

    else if (strcmp(command, "CWD") == 0)
    {
        char new_path[2000];
        if (params[0] == '/')
        {
            strcpy(new_path, params);
        }
        else
        {
            strcat(strcat(strcpy(new_path, current_directory), "/"), params);
        }

        DIR *dir = opendir(new_path);
        // strcpy(new_path, users[user_index].directory);
        if (dir)
        {
            realpath(new_path, current_directory);
            char directory_buffer[directory_size];
            bzero(directory_buffer, directory_size);
            strcat(strcat(directory_buffer, VALID_DIRECTORY), new_path);
            strcpy(users[user_index].directory, new_path);
            send(client_sd, directory_buffer, sizeof(directory_buffer), 0);
            closedir(dir);
        }
        else
        {
            // Directory does not exist.
            send(client_sd, INVALID_DIRECTORY, sizeof(INVALID_DIRECTORY), 0);
        }
    }
    else if (strcmp(command, "PWD") == 0)
    {
        // open directory
        DIR *directory_path;
        struct dirent *file_pointer;
        directory_path = opendir(current_directory);
        if (directory_path != NULL)
        {
            char buffer[directory_size];
            bzero(buffer, buffer_size);
            strcat(strcat(buffer, "257 "), current_directory);
            send(client_sd, buffer, sizeof(buffer), 0);
        }
        else
        {
            send(client_sd, INVALID_DIRECTORY, sizeof(INVALID_DIRECTORY), 0);
        }
    }
    else if ((strcmp(command, "PORT") == 0))
    {
        transfer_count = transfer_count + 1;
        send(client_sd, &transfer_count, sizeof(int), 0);
        printf("I sent trans count \n");

        int client_receiver_sd = socket(AF_INET, SOCK_STREAM, 0);

        unsigned int h_1, h_2, h_3, h_4, p_1, p_2;

        char full_command[buffer_size];
        bzero(full_command, directory_size);
        printf("Before recv tmp command\n");
        recv(client_sd, full_command, sizeof(full_command), 0);
        printf("After recv tmp command\n");
        // strcat(strcat(full_command, command), " ");
        // strcat(full_command, params);
        sscanf(full_command, PORT_REQUEST_FORMAT, &h_1, &h_2, &h_3, &h_4, &p_1, &p_2);
        printf("h_1=%u | h_2=%u | h_3=%u | h_4=%u | p_1=%u | p_2=%u\n", h_1, h_2, h_3, h_4, p_1, p_2);
        unsigned int client_ip_address = (h_1 << 24) + (h_2 << 16) + (h_3 << 8) + h_4;
        unsigned short client_port = (p_1 * 256) + p_2;

        if (client_receiver_sd < 0)
        {
            perror("Client Receiver Socket creation:");
            exit(-1);
        }

        // open data address for ftp
        struct sockaddr_in client_receiver_addr;                        // structure to save IP address and port
        memset(&client_receiver_addr, 0, sizeof(client_receiver_addr)); // Initialize/fill the server_address to 0
        client_receiver_addr.sin_family = AF_INET;                      // address family
        client_receiver_addr.sin_port = htons(client_port);             // port
        client_receiver_addr.sin_addr.s_addr = inet_addr("127.0.0.1");  // htonl(INADDR_LOOPBACK); //inet_addr("127.0.0.1");

        setsockopt(client_receiver_sd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)); //&(int){1},sizeof(int)
        if (setsockopt(client_receiver_sd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
        {
            perror("setsockopt(SO_REUSEADDR) failed");
        }

        // b.server connects to that port using port 20; client accepts connection
        printf("Client Port Accepting Hello: %d\n", client_receiver_addr.sin_port);

        if (connect(client_receiver_sd, (struct sockaddr *)&client_receiver_addr, sizeof(client_receiver_addr)) < 0)
        {
            perror("Connection Failed: Server Data.");
            exit(-1);
        }

        // send port created
        int send_bytes = send(client_sd, PORT_SUCCESS, sizeof(PORT_SUCCESS), 0);
        printf("SENT BYTES old port: %d\n", send_bytes);

        // recv the LIST TSPR RETR command
        bzero(buffer, sizeof(buffer));
        int snew_sd_rec_bytes_ = recv(client_receiver_sd, buffer, sizeof(buffer), 0);
        printf("This is my command buffer: %s\n", buffer);

        // send file status ok
        send(client_receiver_sd, FILE_STATUS_OK, sizeof(buffer), 0);

        if (strcmp(buffer, "LIST") == 0)
        {
            // open directory
            DIR *directory_path;
            struct dirent *file_pointer;
            directory_path = opendir(current_directory);
            printf("This is directory %s\n", current_directory);

            // int pid = fork();
            // if (fork() == 0)o
            // {
            // Will execute the ls command and store its output in a pipe
            bzero(buffer, sizeof(buffer));
            strcpy(buffer, "cd ");
            strcat(buffer, current_directory);
            strcat(buffer, " && ls");
            printf("buffer %s\n", buffer);
            FILE *ls_file = popen(buffer, "r");
            if (ls_file == NULL)
            {
                printf("LIST COMMAND COULD NOT BE EXECUTED\n");
            }
            else
            {

                char FILE_BUFFER[FILE_CHUNK_SIZE];
                bzero(FILE_BUFFER, sizeof(FILE_BUFFER));

                printf("BEGIN SENDING LIST CONTENTS\n");
                int bytes_read = fread(FILE_BUFFER, 1, FILE_CHUNK_SIZE, ls_file);
                int pid = fork();
                if (pid == 0)
                {

                    send(client_receiver_sd, FILE_BUFFER, bytes_read, 0);
                    pclose(ls_file);
                    // close(client_receiver_sd);
                    printf("Exiting fork\n");
                    exit(0);
                }

                printf("END SENDING LIST CONTENTS\n");
            }
            printf("Done exiting fork fork\n");
            close(client_receiver_sd);
        }
        else if (strcmp(buffer, "RETR") == 0)
        {

            char params[buffer_size];
            bzero(params, sizeof(params));
            recv(client_receiver_sd, params, sizeof(params), 0);

            char FILE_BUFFER[FILE_CHUNK_SIZE];
            bzero(FILE_BUFFER, sizeof(FILE_BUFFER));

            printf("Params recieved: %s\n", params);
            int pid = fork();
            if (pid == 0)
            {
                // open directory
                DIR *directory_path;
                struct dirent *file_pointer;
                directory_path = opendir(current_directory);
                chdir(current_directory);

                FILE *fp = fopen(params, "r");
                if (!fp)
                {
                    perror("File does not exist.");
                    send(client_receiver_sd, INVALID_DIRECTORY, sizeof(INVALID_DIRECTORY), 0);
                    close(client_receiver_sd);
                    exit(0);
                }

                int valread;

                valread = fread(FILE_BUFFER, sizeof(char), FILE_CHUNK_SIZE, fp);

                printf("Bytes read: %d\n", valread);

                if (valread < FILE_CHUNK_SIZE)
                {

                    send(client_receiver_sd, FILE_BUFFER, valread, 0);
                }
                else
                {
                    send(client_receiver_sd, FILE_BUFFER, valread, 0);
                    while (valread == FILE_CHUNK_SIZE)
                    {

                        if (valread > 0)
                        {
                            valread = fread(FILE_BUFFER, sizeof(char), FILE_CHUNK_SIZE, fp);
                            send(client_receiver_sd, FILE_BUFFER, valread, MSG_WAITALL);
                            bzero(FILE_BUFFER, sizeof(FILE_BUFFER));
                        }
                        else
                        {
                            send(client_receiver_sd, INVALID_SEQUENCE, sizeof(INVALID_SEQUENCE), 0);
                            printf("sent error\n");
                        }
                    }
                }

                send(client_sd, TRANSFER_COMPLETED, sizeof(TRANSFER_COMPLETED), 0);

                fclose(fp);
                close(client_receiver_sd);
                exit(0);
            }
            close(client_receiver_sd);
        }
        else if (strcmp(buffer, "STOR") == 0)
        {
            DIR *directory_path;
            struct dirent *file_pointer;
            directory_path = opendir(current_directory);
            chdir(current_directory);
            char FILE_BUFFER[FILE_CHUNK_SIZE];
            bzero(FILE_BUFFER, sizeof(FILE_BUFFER));

            char params[buffer_size];
            bzero(params, sizeof(params));
            recv(client_receiver_sd, params, sizeof(params), 0);

            // create tmp file
            char tmp_file[buffer_size];
            bzero(tmp_file, sizeof(tmp_file));
            strcpy(tmp_file, "tmp_");
            strcat(tmp_file, params);

            int valread = recv(client_receiver_sd, FILE_BUFFER, FILE_CHUNK_SIZE, 0);
            printf("Valread %d\n", valread);

            if (strcmp(FILE_BUFFER, INVALID_DIRECTORY) == 0)
            {
                printf("%s\n", INVALID_DIRECTORY);
                close(client_receiver_sd);
            }
            else
            {
                FILE *fp = fopen(tmp_file, "w");

                if (fp == NULL)
                {
                    perror("Failed to write file");
                    return;
                }
                if (valread < FILE_CHUNK_SIZE)
                {
                    printf("Buffer %s\n", buffer);
                    fwrite(FILE_BUFFER, 1, valread, fp);
                }
                else
                {

                    while (valread == FILE_CHUNK_SIZE)
                    { // Big files may require multiple reads
                        fwrite(FILE_BUFFER, 1, valread, fp);
                        bzero(FILE_BUFFER, sizeof(FILE_BUFFER));
                        valread = recv(client_receiver_sd, FILE_BUFFER, FILE_CHUNK_SIZE, 0);
                    }
                }
                send(client_sd, TRANSFER_COMPLETED, sizeof(TRANSFER_COMPLETED), 0);
                fclose(fp);
            }

            close(client_receiver_sd);
        }
        else
        {
            send(client_sd, INVALID_COMMAND, sizeof(INVALID_COMMAND), 0);
        }
        // close(client_receiver_sd);
    }
    else
    {

        send(client_sd, INVALID_COMMAND, sizeof(INVALID_COMMAND), 0);
    }
}