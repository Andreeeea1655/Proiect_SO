#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>

typedef struct{
    int ID;
    char username[50];
    float linie; //longitude
    float col; //latitude
    char clue[100];
    int value;
}Treasure;

void logAction(const char* huntID, const char* message)
{
    char logPath[256];
    snprintf(logPath, sizeof(logPath), "log_%s.txt", huntID);
    int f=open(logPath, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (f<0) exit(-1);
    dprintf(f, "%s\n", message);
    close(f);

    //creeaza legatursa simbolica
    char err[256];
    snprintf(err, sizeof(err), "logged_hunt-%s", huntID);
    symlink(logPath, err);
}

void HuntDir(const char* huntID)
{
    char dirPath[256];
    mkdir("hunt", 0755);
    snprintf(dirPath, sizeof(dirPath), "hunt/%s", huntID);
    mkdir(dirPath,0755);
}

char* treasurePath(const char* huntID)
{
    static char path[256];
    snprintf(path, sizeof(path), "treasure_%s.txt", huntID);
    return path;
}

void addTreasure(const char* huntID)
{
    HuntDir(huntID);
    int f=open(treasurePath(huntID), O_WRONLY | O_CREAT | O_APPEND, 0644);
    if(f<0)
    {
        perror("Error opening file");
        exit(-1);
    }
    Treasure t;
    printf("Enter treasure ID: "); scanf("%d", &t.ID);
    printf("Enter username: "); scanf("%s", t.username);
    printf("Enter latitude: "); scanf("%f", &t.col);
    printf("Enter longitude: "); scanf("%f", &t.linie);
    printf("Enter clue: "); getchar(); fgets(t.clue, 100, stdin);
    t.clue[strcspn(t.clue, "\n")] = 0;
    printf("Enter value: "); scanf("%d", &t.value);

    write(f,&t,sizeof(Treasure));
    close(f);
    logAction(huntID,"Added treasure\n"); 
}

void listTreasures(const char* huntID)
{
    int f=open(treasurePath(huntID), O_RDONLY);
    if(f<0)
    {
        perror("Error opening file");
        exit(-1);

    }
    struct stat st;
    stat(treasurePath(huntID), &st);
    printf("Hunt: %s\n", huntID);
    printf("File size: %ld\n", st.st_size);
    printf("Last modified: %s\n", ctime(&st.st_mtime));
    printf("Treasures:\n");

    Treasure t;
    while(read(f,&t,sizeof(Treasure))>0)
    {
        printf("ID: %d\n", t.ID);
        printf("Username: %s\n", t.username);
        printf("GPS: %.2f, %.2f\n", t.linie, t.col);
        printf("Clue: %s\n", t.clue);
        printf("Value: %d\n", t.value);
        printf("\n");
    }
    close(f);
    logAction(huntID,"Listed treasures\n");
}

void viewTreasure(const char *huntID, int ID)
{
    int f=open(treasurePath(huntID), O_RDONLY);
    if(f<0)
    {
        perror("Error opening file");
        exit(-1);

    }
    Treasure t;
    int ok=0;
    while(read(f,&t,sizeof(Treasure))>0)
    {
        if(t.ID==ID)
        {
            printf("ID: %d\n", t.ID);
            printf("Username: %s\n", t.username);
            printf("GPS: %.2f, %.2f\n", t.linie, t.col);
            printf("Clue: %s\n", t.clue);
            printf("Value: %d\n", t.value);
            printf("\n");
            ok=1; break;
        }
    }
    close(f);
    if(!ok)
        printf("Treasure with ID %d not found\n", ID);
    else
        logAction(huntID, "Viewed treasure\n");
}

void removeTreasure(const char* huntID, int ID)
{
    char path[256];
    char auxpath[256];
    snprintf(path, sizeof(path), "treasure_%s.txt", huntID);
    snprintf(auxpath, sizeof(auxpath), "aux_%s.txt", huntID);
    int f=open(path, O_RDONLY);
    int auxf=open(auxpath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if(f<0 || auxf<0)
    {
        perror("Error opening file");
        exit(-1);
    }
    Treasure t;
    int ok=0;
    while(read(f,&t,sizeof(Treasure))>0)
    {
        if(t.ID==ID)
        {
            ok=1; continue;
        }
        write(auxf,&t,sizeof(Treasure));
    }
    close(f);
    close(auxf);
    if(ok)
    {
       remove(path);
       rename(auxpath,path);
       logAction(huntID, "Removed treasure\n");
    }
    else
    {
        printf("Treasure ID %d not found\n", ID);
        remove(auxpath);
    }
}

void removeHunt(const char* huntID)
{
    char path[256];
    snprintf(path, sizeof(path), "hunt/%s", huntID);
    DIR *dir=opendir(path);
    if(!dir)
    {
        perror("Error opening directory");
        exit(-1);
    }
    struct dirent *entry;
    char filePath[512];
    while((entry=readdir(dir))!=NULL)
    {
        if(strcmp(entry->d_name, ".")==0 || strcmp(entry->d_name, "..")==0)
            continue;
        snprintf(filePath, sizeof(filePath), "%s/%s", path, entry->d_name);
        unlink(filePath); //sterge fisierul
    }
    closedir(dir);
    if(rmdir(path)<0)
    {
        perror("Error removing directory");
        exit(-1);
    }
    else
    {
        printf("Hunt %s removed succesfully\n", huntID);
    }
    char treasure[256];
    char logFile[256];
    char symlink[256];
    snprintf(treasure, sizeof(treasure), "treasure_%s.txt", huntID);
    snprintf(logFile, sizeof(logFile), "log_%s.txt", huntID);
    snprintf(symlink, sizeof(symlink), "logged_hunt-%s", huntID);
    unlink(treasure);
    unlink(logFile);
    unlink(symlink);
    logAction(huntID, "Removed hunt\n");
}
int main(int argc, char* argv[])
{
    if(argc<3)
    {
        perror("Not enough arguments\n");
        exit(-1);
    }
    const char* operation=argv[1];
    const char* hundID=argv[2];
    if(strcmp(operation, "--add")==0)
    {
        addTreasure(hundID);
    }
    else if(strcmp(operation, "--list")==0)
    {
        listTreasures(hundID);
    }
    else if(strcmp(operation, "--view")==0 && argc==4)
    {
        viewTreasure(hundID, atoi(argv[3]));
    }
    else if(strcmp(operation, "--remove_treasure")==0 && argc==4)
    {
        removeTreasure(hundID, atoi(argv[3]));
    }
    else if(strcmp(operation, "--remove_hunt")==0)
    {
        removeHunt(hundID);
    }
    else 
    {
        perror("Invalid command\n");
        exit(-1);
    }
    return 0;
}
