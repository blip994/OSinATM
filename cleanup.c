#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <string.h>

struct my_msg
{
    long msg_type;
    int airport_arrival;
    int total_plane_weight;
    int plane_id;
    int airport_depart;
    int plane_type;
    int occupied_seats;
    int dep_signal;
    int arr_signal;
    int ter_signal;
};
int main(){
    struct my_msg cleanup;
    key_t key;
    int msgid;
    system("touch msgq.txt");
    key = ftok("msgq.txt", 'B');
    if (key == -1)
    {
        printf("error in creating unique key\n");
        exit(1);
    }
    msgid = msgget(key, 0644 | IPC_CREAT);
    if (msgid == -1)
    {
        printf("error in creating queue\n");
        return 1;
    }
    char c;
    while(1){
    printf("Do you want the Air Traffic Control System to terminate?(Y for Yes andN for No)\n");
    fflush(stdout);
    scanf("%c",&c);
    fflush(stdin);
    if(c=='Y'){
    cleanup.ter_signal=1;
    cleanup.msg_type=99;
    if (msgsnd(msgid, (void *)&cleanup, sizeof(struct my_msg), 0) == -1) {
        perror("msgsnd");
        return 1;
    }
    return 0;
    }
    }
    return 0;
}