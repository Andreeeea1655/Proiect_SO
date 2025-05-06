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

void SIGTERM_handler()
{
    write(1, "SIGTERM signal received\n", strlen("SIGTERM signal received\n"));
    exit(0);
}

void SIGUSR1_handler()
{
    write(1, "SIGUSR1 signal received\n", strlen("SIGUSR1 signal received\n"));
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

void start_monitor()
{
    if(monitor_running)
    {
        write(1,"Monitor already running\n", strlen("Monitor already running\n")); //1-stdout , 0-stdin
        return;
    }
    pid_monitor=fork();
    if(pid_monitor<0)
    {
        perror("Fork fail.\n");
        exit(-1);
    }
    if(pid_monitor==0)
    {
        run_monitor();
        while(1);
    }
    else
    {
        monitor_running=1;
        write(1,"Start monitor\n", strlen("Start monitor\n"));
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
            sleep(1);
            kill(pid_monitor, SIGUSR1);
            sleep(1);
            kill(pid_monitor, SIGUSR2);
            sleep(1);
            kill(pid_monitor, SIGTERM);
            sleep(1);
        }
        else if(strcmp(command, "stop monitor")==0)
        {
            stop_monitor();
        }
        else if(strcmp(command, "exit monitor")==0)
        {
            exit_monitor();
        }
        else
        {
            write(1,"Invalid command\n", strlen("Invalid command\n"));
        }
        //:))
    }
    return 0;
}