#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define buffer_size 100
struct user
{
    char username[128];
    char password[257];
};

struct user users[10];
int num_users = 0;

int read_file(){
    FILE *fptr;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    fptr = fopen("users.txt", "r");
    while ((read = getline(&line, &len, fptr)) != -1) {
        for (int i = 0; i < read; i++){
            if(line[i]==','){

                char new_username [i];
                memcpy(new_username, &line[0], i);
                strcpy( users[num_users].username, new_username);

                char new_password [read - i - 2 ];
                memcpy(new_password, &line[i+2], read);
                strtok(new_password, "\n");
                strcpy( users[num_users].password, new_password);

                num_users++;
                break;
            }            
        }
    }
    return 0;
}


