#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define buffer_size 500
struct user
{
    int sd;
    int authenticated;
    char username[buffer_size];
    char password[buffer_size];
    char directory[buffer_size];
};

struct user users[10];
int num_users = 0;

int read_file()
{
    FILE *fptr;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    fptr = fopen("users.txt", "r");
    while ((read = getline(&line, &len, fptr)) != -1)
    {
        for (int i = 0; i < read; i++)
        {
            if (line[i] == ',')
            {

                char new_username[i];
                memcpy(new_username, &line[0], i);
                strcpy(users[num_users].username, new_username);

                char new_password[read - i - 2];
                memcpy(new_password, &line[i + 2], read);
                strtok(new_password, "\n");
                strcpy(users[num_users].password, new_password);

                char directory[buffer_size];
                memcpy(directory, "", 1);
                strcpy(users[num_users].directory, directory);

                int sd = 0;
                users[num_users].sd = sd;

                int authenticated = 0;
                users[num_users].authenticated = authenticated;

                num_users++;
                break;
            }
        }
    }

    return 0;
}
int get_user_index(int sd)
{
    for (int i = 0; i < num_users; i++)
    {     
        if (users[i].sd == sd)
        {   
            return i;
        }
    }
    return -1;
}


int update_directory(int sd, char *new_value)
{
    int index = get_user_index(sd);

    strcpy(users[index].directory, new_value);

    return 0;
}
