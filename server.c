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
#include <errno.h>

#define PORT 6000
#define directory_size 500
#define MAX_CLIENTS 3

#include "helper.definations.h"
#include "helper.userAuth.c"
#define DEFINE

void server_user_login(int server_sd);
void serve_client(int client_fd, int user_index);
int main()
{
    // create  and bind socket
    read_file();

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

        // sd2stat[i] = new_stat(NULL, 0, default_path);
    }

    fd_set readfds;

    // server_user_login(server_sd);

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
        if (activity < 0 && (errno != EINTR))
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
                    // server_user_login(sd);
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

                if (users[user_index].authenticated == 0)
                {
                    server_user_login(sd);
                }
                else
                {
                    serve_client(sd, user_index);
                }
            }
        }
    }
}

void server_user_login(int client_sd)
{
    printf("Hello I come into server loop\n");

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
                break;
            }

            if (strcmp(pass_buffer, users[i].username) == 0)
            {
                send(client_sd, LOGIN_NEED_PASS, sizeof(LOGIN_NEED_PASS), 0);
            }
            else
            {
                if (i == num_users - 1)
                {
                    send(client_sd, LOGIN_FAILED, sizeof(LOGIN_FAILED), 0);
                    break;
                }
                else
                {
                    continue;
                }
            }
            char pass_buffer[buffer_size];
            recv(client_sd, pass_buffer, sizeof(pass_buffer), 0);

            if (strcmp(pass_buffer, users[i].password) == 0)
            {

                send(client_sd, LOGIN_SUCCESS, sizeof(LOGIN_SUCCESS), 0);
                char current_directory[buffer_size];
                getcwd(current_directory, sizeof(current_directory));
                // update_authentication(users[i].username, 0);
                update_directory(users[i].sd, current_directory);
                users[i].sd = client_sd;
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
void serve_client(int client_sd, int user_index)
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

    // get users CWD
    char current_directory[buffer_size];
    strcpy(current_directory, users[user_index].directory);

    if (strcmp(command, "CWD") == 0)
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
            strcpy(users[user_index].directory,new_path);
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
    else
    {
        char buffer[directory_size];
        bzero(buffer, buffer_size);
        int recv_bytes = recv(client_sd, buffer, sizeof(buffer), 0);
    }
}