#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <string.h>



struct my_msg{
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


//This process is used to direct planes to airports and from one airport to another
int main(){
    
    key_t key;
    int num_of_airports_managed; //2-10 inclusive
    printf("Enter the number of airports to be handled/managed:\n");
    fflush(stdout);
    scanf("%d",&num_of_airports_managed);

    //Set up a message queue
    
    int msgid;

    system("touch msgq.txt");
    key = ftok("msgq.txt", 'B');
    if (key == -1){
        printf("error in creating unique key\n");
        exit(1);
    }

    msgid = msgget(key, 0644 | IPC_CREAT);
    if(msgid == -1){
        printf("error in creating queue\n");
        return 1;
    }

    FILE *fp;
    // system("touch AirTrafficController.txt");
    fp = fopen("AirTrafficController.txt", "a");
    if (fp == NULL) {
        printf("Error opening file!\n");
        exit(1);
    }

    
    while (1){           
            struct my_msg rec_msg;
            if(msgrcv(msgid,(void *)&rec_msg,sizeof(struct my_msg),0,0)==-1){
                perror("message receive error");
                return 1;
            }
            if(rec_msg.ter_signal==1) break;
            printf("%d", rec_msg.plane_id);
            if(rec_msg.msg_type>0 && rec_msg.msg_type<11){
                if(rec_msg.dep_signal==1){
                    printf("plane %d has departed from Airport %d and will land at Airport %d",rec_msg.plane_id,rec_msg.airport_depart,rec_msg.airport_arrival);
                    fflush(stdout);
                    fprintf(fp, "Plane %d has departed from Airport %d and will land at Airport %d.\n",
                        rec_msg.plane_id, rec_msg.airport_depart, rec_msg.airport_arrival);
                    fflush(fp);
                    struct my_msg *modified_msg = (struct my_msg *)malloc(sizeof(struct my_msg));
                    if (modified_msg == NULL) {
                        perror("malloc failed");
                        exit(1);
                    }
                    memcpy(modified_msg, &rec_msg, sizeof(struct my_msg));
                    modified_msg->msg_type=rec_msg.airport_arrival + 10;
                    if (msgsnd(msgid, (void *)modified_msg, sizeof(struct my_msg), 0) == -1) {
                        perror("msgsnd");
                        return 1;
                    }
                    free(modified_msg);
                    
                    
                    continue;
                }

            fflush(stdout);

            fflush(stdout);

            struct my_msg *modified_msg = (struct my_msg *)malloc(sizeof(struct my_msg));
            if (modified_msg == NULL) {
                perror("malloc failed");
                exit(1);
            }
            memcpy(modified_msg, &rec_msg, sizeof(struct my_msg));
            modified_msg->msg_type=rec_msg.airport_depart + 10;
            
            if (msgsnd(msgid, (void *)modified_msg, sizeof(struct my_msg), 0) == -1) {
                perror("msgsnd");
                return 1;
            }
            fflush(stdout);
            // Free the allocated memory after sending (if successful)
            // rec_msg.data.signal=1;
            free(modified_msg);   
            
            }
            
    }
    fclose(fp);
    return 0;
}