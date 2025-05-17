#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>

pid_t pid_monitor=0;
int monitor_running=0;
int pipe_monitor[2]; //0-citire, 1- scriere

void list_hunts_handler()
{
    pid_monitor=fork();
    if(pid_monitor<0)
    {
        perror("Fork failed\n");
        exit(-1);
    }
    if(pid_monitor==0)
    {
        //child
        execl("./treasure_manager", "./treasure_manager", "--list_hunts", NULL);
        //execl replaces procesul curent cu unul nou
        perror("exec failed\n");
        exit(-1);
    }
    else
    {
        //parent
        int status;
        waitpid(pid_monitor, &status, 0);
    }
}

void list_treasures_handler(const char* huntID)
{
    pid_monitor=fork();
    if(pid_monitor<0)
    {
        perror("Fork failed\n");
        exit(-1);
    }
    if(pid_monitor==0)
    {
        //child
        execl("./treasure_manager", "./treasure_manager", "--list_treasures", huntID, NULL);
        //execl replaces procesul curent cu unul nou
        perror("exec failed\n");
        exit(-1);
    }
    else
    {
        //parent
        int status;
        waitpid(pid_monitor, &status, 0);
    }
}

void view_treasure_handler(const char* huntID, const char* treasureID)
{
    char tID[10];
    snprintf(tID, sizeof(tID), "%s", treasureID);
    pid_monitor=fork();
    if(pid_monitor<0)
    {
        perror("Fork failed\n");
        exit(-1);
    }
    if(pid_monitor==0)
    {
        //child
        execl("./treasure_manager", "./treasure_manager", "--view_treasure",huntID, tID, NULL);
        //execl replaces procesul curent cu unul nou
        perror("exec failed\n");
        exit(-1);
    }
    else
    {
        //parent
        int status;
        waitpid(pid_monitor, &status, 0);
    }
}
void calculate_score_handler()
{
    int pipe_score[2];
    if(pipe(pipe_score)<0)
    {
        perror("Pipe failed\n");
        exit(-1);
    }
    pid_t pid=fork();
    if(pid<0)
    {
        perror("Fork failed\n");
        exit(-1);
    }
    if(pid==0)
    {
        //child
        close(pipe_score[0]);
        dup2(pipe_score[1], STDOUT_FILENO);
        close(pipe_score[1]);
        execl("./calculate_score", "./calculate_score", NULL);
        //execl replaces procesul curent cu unul nou
        perror("exec failed\n");
        exit(-1);
    }
    else
    {
        //parent
        close(pipe_score[1]);
        char buffer[256];
        int bt;
        while((bt=read(pipe[0], buffer, sizeof(buffer)-1))>0)
        {
            buffer[bt]='\0';
            write(1, buffer, bt);
        }
        close(pipe_score[0]);
        int status;
        waitpid(pid_monitor, &status, 0);
    }
}
void SIGTERM_handler()
{

    write(1, "SIGTERM signal received\n", strlen("SIGTERM signal received\n"));
    exit(0);
}

void SIGUSR1_handler()
{
    write(1, "SIGUSR1 signal received\n", strlen("SIGUSR1 signal received\n"));
    int f=open("cmd.txt", O_RDONLY);
    if(f<0)
    {
        perror("Error opening file\n");
        exit(-1);
    }
    char buffer[256];
    int i=0;
    int bt=read(f, buffer, sizeof(buffer)-1);
    buffer[bt]='\0';
    close(f);
    char* line[3]={NULL, NULL, NULL};
    char* p=strtok(buffer,"\n");
    while(p && i<3)
    {
        line[i++]=p;
        p=strtok(NULL,"\n");
    }

    pid_t child=fork();
    if(child<0)
    {
        perror("Fork failed\n");
        exit(-1);
    }
    if(child==0)
    {
        if(line[0] && strcmp(line[0], "list_hunts")==0)
        {
            list_hunts_handler();
        }
        else if(line[0] && line[1] && strcmp(line[0], "list_treasures")==0)
        {
            list_treasures_handler(line[1]);
        }
        else if(strcmp(line[0], "view_treasure")==0)
        {
            if(line[0] && line[1] && line[2])
                view_treasure_handler(line[1], line[2]);
            else
            {
                write(STDOUT_FILENO, "Missing ID\n", strlen("Missing ID\n"));
                exit(-1);
            }
        }
        else if(line[0] && strcmp(line[0], "calculate_score")==0)
        {
           calculate_score_handler();
        }
        else
        {
            write(pipe_monitor[1], "Invalid command\n", strlen("Invalid command\n"));
            exit(-1);
        }
        //perror("exec failed\n");
        exit(-1);
    }
    else
    {
        int status;
        waitpid(child, &status, 0);
    }
}

void SIGUSR2_handler()
{
    write(1, "SIGUSR2 signal received\n", strlen("SIGUSR2 signal received\n"));
}

void run_monitor()
{
    struct sigaction sa1;
    memset(&sa1,0,sizeof(struct sigaction));
    sa1.sa_handler=SIGUSR1_handler;
    sigaction(SIGUSR1,&sa1,NULL);
    
    struct sigaction sa2;
    memset(&sa2,0,sizeof(struct sigaction));
    sa2.sa_handler=SIGUSR2_handler;
    sigaction(SIGUSR2,&sa2,NULL);

    struct sigaction sa3;
    memset(&sa3,0,sizeof(struct sigaction));
    sa3.sa_handler=SIGTERM_handler;
    sigaction(SIGTERM,&sa3,NULL);
}

void command_monitor(const char* cmd, const char* argv1, const char* argv2)
{
    int f=open("cmd.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if(f<0)
    {
        perror("Error opening file\n");
        exit(-1);
    }
    char ln[100];
    snprintf(ln, sizeof(ln), "%s\n", cmd);
    write(f,ln,strlen(ln));
    //fsync(f);
    if(argv1)
    {
        snprintf(ln, sizeof(ln), "%s\n", argv1);
        write(f,ln, strlen(ln));
    }
    if(argv2)
    {
        snprintf(ln, sizeof(ln), "%s\n", argv2);
        write(f,ln, strlen(ln));
    }
    close(f);
    kill(pid_monitor, SIGUSR1);
    sleep(1);
}
void start_monitor()
{
    if(monitor_running)
    {
        write(1,"Monitor already running\n", strlen("Monitor already running\n")); //1-stdout , 0-stdin
        return;
    }
    if(pipe(pipe_monitor)<0)
    {
        perror("Pipe failed\n");
        exit(-1);
    }
    pid_monitor=fork();
    if(pid_monitor<0)
    {
        perror("Fork fail.\n");
        exit(-1);
    }
    if(pid_monitor==0)
    {
        //child
        close(pipe_monitor[0]);
        //close(pipe_monitor[1]);
        run_monitor();
        while(1)
        {
            pause();
        }
        exit(0);
    }
    else
    {
        //parent
        monitor_running=1;
        close(pipe_monitor[1]);
    }
}

void stop_monitor()
{
    if(!monitor_running)
    {
        perror("Monitor is not running\n");
        exit(-1);
    }
    write(1,"Stop monitor process\n", strlen("Stop monitor process\n"));
    //trimite un semnal terminal
    if(kill(pid_monitor, SIGTERM)==-1)
    { 
        perror("Kill failed\n");
        exit(-1);
    }
    //aici scri in pipe
    char buffer[256];
    int bt;
    while(bt=read(pipe_monitor[0], buffer, sizeof(buffer)-1))
    {
        buffer[bt]='\0';
        write(1, buffer, bt);
    }
    close(pipe_monitor[0]);
    int status;
    pid_t wait_pid=waitpid(pid_monitor, &status, 0);

    if(wait_pid==-1)
    {
        perror("Waitpid fail\n");
        exit(-1);
    }
    else
    {
        if(WIFEXITED(status))
        {
            write(1,"Monitor process stopped\n", strlen("Monitor process stopped\n"));
        }
        else
        {
            write(1,"Monitor process not stopped\n", strlen("Monitor process stopped\n"));
        }
    }
    monitor_running=0;
}

void exit_monitor()
{
    if(monitor_running)
    {
        write(1,"Stop monitor\n", strlen("Stop monitor\n"));
        stop_monitor();
        monitor_running=0;
    }
    else
    {
        write(1,"Exit monitor\n", strlen("Exit monitor\n"));
        exit(0);
    }
}

int main()
{
    char command[50];
    while(1)
    {
        int len=read(0, command, sizeof(command)-1);
        command[len-1]='\0';
        if(strcmp(command, "start monitor")==0)
        {
            start_monitor();
        }
        else if(strcmp(command, "stop monitor")==0)
        {
            stop_monitor();
        }
        else if(strcmp(command, "exit monitor")==0)
        {
            exit_monitor();
        }
        else if(strcmp(command, "list hunts")==0)
        {
            command_monitor("list_hunts", NULL, NULL);
        }
        else if(strncmp(command, "list treasures",14)==0)
        {
            char* huntID=command+15;
            command_monitor("list_treasures", huntID,NULL);
        }
        else if(strncmp(command, "view treasure",13)==0)
        {
            char *ln=command+14;
            char* huntID=strtok(ln," ");
            char* treasureID=strtok(NULL," ");
            command_monitor("view_treasure", huntID, treasureID);
        }
        else if(strcmp(command, "calculate score")==0)
        {
            command_monitor("calculate_score", NULL, NULL);
        }
        else
        {
            write(1,"Invalid command\n", strlen("Invalid command\n"));
        }
    }
    return 0;
}
