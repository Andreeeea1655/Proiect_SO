#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
//#define _POSIX_C_SOURCE 200809L

typedef struct{
    int ID;
    char username[50];
    float linie; //longitude
    float col; //latitude
    char clue[100];
    int value;
}Treasure;

typedef struct{
    char username[50];
    int score;
    char huntID[50];
}Person;

int main()
{
    DIR *dir=opendir("hunt/");
    if (!dir)
    {
        perror("Error opening directory");
        exit(-1);
    }
    struct dirent *entry;
    Person user[50];
    int users=0;
    while((entry=readdir(dir))!=NULL)
    {
            if(strcmp(entry->d_name, ".")==0 || strcmp(entry->d_name, "..")==0)
                continue;
            char fullPath[256];
            snprintf(fullPath, sizeof(fullPath), "hunt/%s", entry->d_name);
            struct stat st;
            if(stat(fullPath, &st)!=0 || !S_ISDIR(st.st_mode))
            {
                continue; // Not a directory
            }
            //char huntID[50];
            //strncpy(huntID, entry->d_name, sizeof(huntID));
            char path[256];
            snprintf(path, sizeof(path), "hunt/%s/treasure_%s.bin", entry->d_name, entry->d_name);
            int f=open(path, O_RDONLY);
            if(f<0)
            {
                perror("Error opening file");
                exit(-1);
            }
    Treasure t;
    while(read(f,&t,sizeof(Treasure))>0)
    {
        int ok=0;
        for(int i=0;i<users;i++)
        {
            if(strcmp(t.username, user[i].username)==0)
            {
                user[i].score+=t.value;
                ok=1;
                break;
            }
        }
        if(!ok)
        {
            strcpy(user[users].username, t.username);
            strcpy(user[users].huntID, entry->d_name);
            user[users].score=t.value;
            users++;
        }
    }
    close(f);
}
closedir(dir);
for(int i=0;i<users;i++)
{
    printf("%s: ", user[i].huntID);
    printf("%s: %d\n", user[i].username, user[i].score);
}
    return 0;
}