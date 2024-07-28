#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>


// struct Plane{
//     int airport_arrival;
//     int total_plane_weight;
//     int plane_id;
//     int airport_depart;
//     int plane_type;
//     int occupied_seats;
//     int dep_signal;
// };

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
int main(){

    struct my_msg plane;

    //plane id between 1-10 inclusive
    printf("\nEnter the Plane ID: ");
    scanf("%d", &plane.plane_id);

    //plane type 1 for passanger and 0 for cargo plane
    printf("\nEnter the plane type: ");
    scanf("%d", &plane.plane_type);

    //Cargo Plane
    if (plane.plane_type==0){
        int cargo_items; //1-100 inclusive
        printf("\nEnter Number of Cargo Items: ");
        scanf("%d", &cargo_items);

        int avg_cargo_weight; //1-100 inclusive
        printf("\nEnter Average Weight of Cargo Items: ");
        scanf("%d", &avg_cargo_weight);

        plane.total_plane_weight = cargo_items * avg_cargo_weight + 75 * 2; //2 crew members with avg weight 75 kgs

        //Departing airport 1-10 inclusive
        printf("\nEnter Airport Number for Departure: ");
        scanf("%d", &plane.airport_depart);

        //Arrival Airport 1-10 inclusive
        printf("\nEnter Airport Number for Arrival: ");
        scanf("%d", &plane.airport_arrival);
    }
    // will need to enter the number of occupied seats in the passanger plane
    else if(plane.plane_type==1){

        //Occupied Seats Max number of occupied seats is 10
        printf("\nEnter Number of Occupied of Seats: ");
        scanf("%d", &plane.occupied_seats);

        pid_t pids[plane.occupied_seats];
        int pipes[plane.occupied_seats][2];
        for(int i=0;i<plane.occupied_seats;i++){
            if (pipe(pipes[i]) == -1){
                perror("\nPipes failed\n");
                return 1;
            }
        }

    //Forking to create a child Process
       
        pid_t childpid;

        for(int i=0;i<plane.occupied_seats;i++){
            childpid = fork();

            if(childpid <0){
                perror("\nFork Failed\n");
                return 2;
            }
            else if(childpid==0){
                int lugage_weight;  //Max Value is 25 Kgs is passanger without luggage then weight = 0, Only 1 value for 1 passanger
                printf("Enter Weigh of Your Luggage:");
                fflush(stdout);
                scanf("%d", &lugage_weight);

                int body_weight; //Max value = 100kg. Guranteed between 10 and 100 inclusive
                printf("Enter Your Body Weight: ");
                fflush(stdout);
                scanf("%d", &body_weight);

                close(pipes[i][0]); //Close read end

                write(pipes[i][1], &lugage_weight, sizeof(int));
                write(pipes[i][1], &body_weight, sizeof(int));

                close(pipes[i][1]); //Close write end

                exit(i+1);
            }
            else{
                pids[i] = childpid;
                wait(NULL);
            }
        }
        for(int i=0;i<plane.occupied_seats;i++){
            wait(NULL);
        }
        int total_luggage=0;
        int total_passenger_weight=0;
        for(int i=0;i<plane.occupied_seats;i++){
            int lugage_weight, body_weight;
            
            close(pipes[i][1]); //Close write end

            read(pipes[i][0], &lugage_weight, sizeof(int));
            read(pipes[i][0], &body_weight, sizeof(int));

            close(pipes[i][0]); //Close read end

            total_luggage += lugage_weight;
            total_passenger_weight += body_weight;
        }
        plane.total_plane_weight = total_luggage + total_passenger_weight + 75 * 7;       
        
        }
    //Departing airport 1-10 inclusive
    printf("\nEnter Airport Number for Departure: ");
    scanf("%d", &plane.airport_depart);

    //Arrival Airport 1-10 inclusive
    printf("\nEnter Airport Number for Arrival: ");
    scanf("%d", &plane.airport_arrival);
    printf("%d\n", plane.total_plane_weight);
    
    key_t key;
    int msgid;
    // struct my_msg plane_info;
    system("touch msgq.txt");
    key = ftok("msgq.txt", 'B');
    if (key == -1){
    printf("error in creating unique key\n");
    exit(1);
    }
    msgid = msgget(key, 0644|IPC_CREAT);
    
    if(msgid == -1){
        printf("error in creating queue\n");
        return 1;
    }
    // printf("msg q created with id %d",msgid);
    plane.msg_type=plane.plane_id;
    
    

    if (msgsnd(msgid, (void *)&plane, sizeof(struct my_msg), 0) == -1) {
        perror("msgsnd");
        return 1;
    }
    // else{
    //     printf("message sent\n");
    // }


    while(1){
        sleep(1);
    }

    return 0;
}

