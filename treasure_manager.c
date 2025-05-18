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
    snprintf(logPath, sizeof(logPath), "hunt/%s/logged_hunt", huntID);
    int f=open(logPath, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (f<0)
    {
        perror("Error opening file");
        exit(-1);
    }
    dprintf(f, "%s\n", message);
    close(f);

    //creeaza legatursa simbolica
    char err[256];
    snprintf(err, sizeof(err), "logged_hunt-%s", huntID);
    if(access(err, F_OK)<0)
    {
        symlink(logPath, err);
    }
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
    snprintf(path, sizeof(path), "hunt/%s/treasure_%s.bin", huntID, huntID);
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
    write(1, "Enter treasure ID: ", strlen("Enter treasure ID: ")); scanf("%d", &t.ID);
    int f1=open(treasurePath(huntID), O_RDONLY);
    if(f1>=0)
    {
        Treasure existsID;
        while(read(f1,&existsID,sizeof(Treasure))>0)
        {
            if(existsID.ID==t.ID)
            {
                char buffer[20];
                int ID=snprintf(buffer, sizeof(buffer), "%d", t.ID);
                write(1,"Already existent treasure with ID: ", strlen("Already existent treasure with ID: "));
                write(1,buffer,ID);
                write(1,"\n", 1);
                close(f1);
                exit(-1);
            }
        }
        close(f1);
    }
    write(1,"Enter username: ", strlen("Enter username: ")); scanf("%s", t.username);
    write(1,"Enter latitude: ", strlen("Enter latitude: ")); scanf("%f", &t.col);
    write(1,"Enter longitude: ", strlen("Enter longitude: ")); scanf("%f", &t.linie);
    write(1,"Enter clue: ", strlen("Enter clue: ")); getchar(); fgets(t.clue, 100, stdin);
    t.clue[strcspn(t.clue, "\n")] = 0;
    write(1,"Enter value: ", strlen("Enter value: ")); scanf("%d", &t.value);

    write(f,&t,sizeof(Treasure));
    close(f);
    logAction(huntID,"Added treasure\n"); 
}

void listHunts()
{
    DIR *dir=opendir("hunt");
    if(!dir)
    {
        perror("Error opening directory");
        exit(-1);
    }
    struct dirent *entry;
    while((entry=readdir(dir))!=NULL)
    {
        if((strcmp(entry->d_name, "..")==0) || strcmp(entry->d_name, ".")==0)
            continue;
        char path[256];
        int len1=snprintf(path, sizeof(path), "hunt/%s/treasure_%s.bin", entry->d_name, entry->d_name);
        if(len1<0 || len1>sizeof(path))
            continue;
        int f=open(path, O_RDONLY);
        if(f<0)
        {
            perror("Error opening file");
            exit(-1);
        }
        Treasure t;
        int ct=0;
        while(read(f,&t, sizeof(Treasure))>0)
        {
            ct++;
        }
        close(f);

        char logPath[256];
        int len2=snprintf(logPath, sizeof(logPath), "%s, %d treasures\n", entry->d_name, ct);
        if(len2>0 && len2<sizeof(logPath))
        {
            write(1,logPath, strlen(logPath));
        }
    }
    closedir(dir);
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
    char bufferSize[100];
    int size=snprintf(bufferSize, sizeof(bufferSize), "%ld", st.st_size);
    write(1,"Hunt: ", strlen("Hunt: "));
    write(1,huntID, strlen(huntID));
    write(1,"\n", 1);
    write(1,"File size: ", strlen("File size: "));
    write(1,bufferSize, size);
    write(1,"\n", 1);
    write(1,"Last modified: ", strlen("Last modified: "));
    write(1,ctime(&st.st_mtime), strlen(ctime(&st.st_mtime)));
    write(1,"Treasures:\n", strlen("Treasures:\n"));

    Treasure t;
    while(read(f,&t,sizeof(Treasure))>0)
    {
        char bufferID[20];
        char bufferGPS[20];
        char bufferValue[20];
        int ID=snprintf(bufferID, sizeof(bufferID), "%d", t.ID);
        int linie=snprintf(bufferGPS, sizeof(bufferGPS), "%.2f", t.linie);
        int coloana=snprintf(bufferGPS, sizeof(bufferGPS), "%.2f", t.col);
        int value=snprintf(bufferValue, sizeof(bufferValue), "%d", t.value);
        write(1,"ID: ", strlen("ID: "));
        write(1,bufferID, ID);
        write(1,"\n", 1);
        write(1,"Username: ", strlen("Username: "));
        write(1,t.username, strlen(t.username));
        write(1,"\n", 1);
        write(1,"GPS: ", strlen("GPS: "));
        write(1,bufferGPS, linie);
        write(1,", ", 2);
        write(1,bufferGPS, coloana);
        write(1,"\n", 1);
        write(1,"Clue: ", strlen("Clue: "));
        write(1,t.clue, strlen(t.clue));
        write(1,"\n", 1);
        write(1,"Value: ", strlen("Value: "));
        write(1,bufferValue, value);
        write(1,"\n", 1);
        write(1,"\n", 1);
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
    lseek(f,0,SEEK_SET);
    Treasure t;
    int ok=0;
    while(read(f,&t,sizeof(Treasure))>0)
    {
        if(t.ID==ID)
        {
            char bufferID[20];
            char bufferGPS[20];
            char bufferValue[20];
            int ID=snprintf(bufferID, sizeof(bufferID), "%d", t.ID);
            int linie=snprintf(bufferGPS, sizeof(bufferGPS), "%.2f", t.linie);
            int coloana=snprintf(bufferGPS, sizeof(bufferGPS), "%.2f", t.col);
            int value=snprintf(bufferValue, sizeof(bufferValue), "%d", t.value);
            write(1,"ID: ", strlen("ID: "));
            write(1,bufferID, ID);
            write(1,"\n", 1);
            write(1,"Username: ", strlen("Username: "));
            write(1,t.username, strlen(t.username));
            write(1,"\n", 1);
            write(1,"GPS: ", strlen("GPS: "));
            write(1,bufferGPS, linie);
            write(1,", ", 2);
            write(1,bufferGPS, coloana);
            write(1,"\n", 1);
            write(1,"Clue: ", strlen("Clue: "));
            write(1,t.clue, strlen(t.clue));
            write(1,"\n", 1);
            write(1,"Value: ", strlen("Value: "));
            write(1,bufferValue, value);
            write(1,"\n", 1);
            write(1,"\n", 1);
            ok=1; break;
        }
    }
    char bufferID[20];
    int bID=snprintf(bufferID, sizeof(bufferID), "%d", ID);
    if(!ok)
    {
        write(1, "Didn't find reasure with ID: ", strlen("Didn't find reasure with ID: "));
        write(1,bufferID, bID);
        write(1,"\n", 1);
    }
    else
        logAction(huntID, "Viewed treasure\n");
    close(f);
}

void removeTreasure(const char* huntID, int ID)
{
    char path[256];
    char auxpath[256];
    snprintf(path, sizeof(path), "hunt/%s/treasure_%s.bin", huntID, huntID);
    snprintf(auxpath, sizeof(auxpath), "hunt/%s/aux_%s.bin", huntID, huntID);
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
    char bufferID[20];
    int bID=snprintf(bufferID, sizeof(bufferID), "%d", ID);
    if(ok)
    {
       remove(path);
       rename(auxpath,path);
       logAction(huntID, "Removed treasure\n");
       write(1,"Removed treasure with ID: ", strlen("Removed treasure with ID: "));
       write(1,bufferID, bID);
       write(1,"\n", 1);
    
    }
    else
    {
        write(1, "Didn't find reasure with ID: ", strlen("Didn't find reasure with ID: "));
        write(1,bufferID, bID);
        write(1,"\n", 1);
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
        write(1,"Removed succesfully hunt ",strlen("Removed succesfully hunt "));
        write(1,huntID, strlen(huntID));
        write(1,"\n", 1);
    }
    char treasure[256];
    char logFile[256];
    char symlink[256];
    snprintf(treasure, sizeof(treasure), "treasure_%s.txt", huntID);
    snprintf(logFile, sizeof(logFile), "hunt/%s/logged_hunt", huntID);
    snprintf(symlink, sizeof(symlink), "logged_hunt-%s", huntID);
    unlink(treasure);
    unlink(logFile);
    unlink(symlink);
    logAction(huntID, "Removed hunt\n");
}
int main(int argc, char* argv[])
{
    if(argc<2)
    {
        perror("Not enough arguments\n");
        exit(-1);
    }
    const char* operation=argv[1];
    const char* huntID=argv[2];
    if(strcmp(operation, "--add")==0)
    {
        addTreasure(huntID);
    }
    else if(strcmp(operation, "--list_treasures")==0)
    {
        listTreasures(huntID);
    }
    else if(strcmp(operation,"--list_hunts")==0 && argc==2)
    {
        listHunts();
    }
    else if(strcmp(operation, "--view_treasure")==0 && argc==4)
    {
        viewTreasure(huntID, atoi(argv[3]));
    }
    else if(strcmp(operation, "--remove_treasure")==0 && argc==4)
    {
        removeTreasure(huntID, atoi(argv[3]));
    }
    else if(strcmp(operation, "--remove_hunt")==0)
    {
        removeHunt(huntID);
    }
    else 
    {
        perror("Invalid command\n");
        exit(-1);
    }
    return 0;
}
