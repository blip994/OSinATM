#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <string.h>
#include <pthread.h>

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

struct thread_args
{
    int plane_id;
    int runway;
    int airport_number;
};

void *runner(void *param);
void *runner2(void *param);

pthread_mutex_t mutex;

int main()
{
    pthread_mutex_init(&mutex, NULL);

    int airport_number;
    int no_runways;

    printf("Enter Airport Number:");
    fflush(stdout);
    scanf("%d", &airport_number);

    printf("Enter number of Runways:");
    fflush(stdout);
    scanf("%d", &no_runways);

    int arr[no_runways];
    int count = 0;
    printf("Enter load capacities of runways (space-separated, press Enter to finish):");
    fflush(stdout);
    do
    {
        scanf("%d", &arr[count++]);
    } while (getchar() != '\n' && count < no_runways);

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
    fflush(stdout);

    struct thread_args *args = (struct thread_args *)malloc(sizeof(struct thread_args));
    while (1)
    {
        struct my_msg rec_msg;
        pthread_t tid;
        pthread_attr_t attr;
        if (msgrcv(msgid, (void *)&rec_msg, sizeof(struct my_msg), airport_number + 10, 0) == -1)
        {
            perror("message receive error");
            return 1;
        }
        if (rec_msg.msg_type - 10 == airport_number)
        {
            if (rec_msg.airport_depart == airport_number)
            {
                int load = rec_msg.total_plane_weight;
                int runway = 15000;
                int runway_no = 0;
                int plane_id = rec_msg.plane_id;

                pthread_mutex_lock(&mutex);
                for (int i = 0; i < no_runways; i++)
                {
                    if (arr[i] >= load && arr[i] < runway)
                    {
                        runway = arr[i];
                        runway_no = i + 1;
                    }
                }
                pthread_mutex_unlock(&mutex);

                args->airport_number = airport_number;
                args->runway = runway_no;
                args->plane_id = plane_id;
                pthread_attr_init(&attr);
                pthread_create(&tid, &attr, runner, args);
                pthread_join(tid, NULL);

                struct my_msg *modified_msg = (struct my_msg *)malloc(sizeof(struct my_msg));
                if (modified_msg == NULL)
                {
                    perror("malloc failed");
                    exit(1);
                }
                memcpy(modified_msg, &rec_msg, sizeof(struct my_msg));
                modified_msg->msg_type = rec_msg.plane_id;
                modified_msg->dep_signal = 1;

                if (msgsnd(msgid, (void *)modified_msg, sizeof(struct my_msg), 0) == -1)
                {
                    perror("msgsnd");
                    return 1;
                }
                fflush(stdout);
                free(modified_msg);
            }
            if (rec_msg.airport_arrival == airport_number)
            {
                int load = rec_msg.total_plane_weight;
                int runway = 15000;
                int runway_no = no_runways + 1;
                int plane_id = rec_msg.plane_id;

                pthread_mutex_lock(&mutex);
                for (int i = 0; i < no_runways; i++)
                {
                    if (arr[i] >= load && arr[i] < runway)
                    {
                        runway = arr[i];
                        runway_no = i + 1;
                    }
                }
                pthread_mutex_unlock(&mutex);

                args->airport_number = airport_number;
                args->runway = runway_no;
                args->plane_id = plane_id;
                pthread_attr_init(&attr);
                pthread_create(&tid, &attr, runner2, args);
                pthread_join(tid, NULL);
            }
        }
    }

    pthread_mutex_destroy(&mutex);
    return 0;
}

void *runner(void *param)
{
    struct thread_args *rec = (struct thread_args *)param;
    int plane_id = rec->plane_id;
    int runway = rec->runway;
    int airport_number = rec->airport_number;
    sleep(3);
    printf("Plane %d has completed boarding and taken off from runway No. %d of Airport NO. %d\n", plane_id, runway, airport_number);
    fflush(stdout);
    sleep(10);
    pthread_exit(0);
}

void *runner2(void *param)
{
    struct thread_args *rec = (struct thread_args *)param;
    int plane_id = rec->plane_id;
    int runway = rec->runway;
    int airport_number = rec->airport_number;
    sleep(3);
    printf("Plane %d has landed on Runway No.%d of Airport No.%d and has completed deboarding/unloading\n", plane_id, runway, airport_number);
    fflush(stdout);
    pthread_exit(0);
}