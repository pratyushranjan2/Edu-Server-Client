#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <string.h>
#include "configs.h"
#include "utils.h"

int main (int argc, char **argv)
{
    char client_queue_name [64];
    mqd_t qd_srv, qd_client;   // Server and Client Msg queue descriptors
    //int num = 1;
    //char itr[MAX_ITR_LEN];
    char itr_name[MAX_ITR_NAME_LEN];
    char args[MAX_ARGS_LEN];

    if ((qd_srv = mq_open(SERVER_QUEUE_NAME, O_WRONLY)) == -1) {
        perror ("Client MsgQ: mq_open (qd_srv)");
        exit (1);
    }

    struct mq_attr attr;

    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;

    client_msg_t out_msg;
    // create the client queue for receiving messages from the server
    sprintf (out_msg.client_q, "/clientQ-%d", getpid ());

    if ((qd_client = mq_open(out_msg.client_q, O_RDONLY | O_CREAT, QUEUE_PERMISSIONS,
                           &attr)) == -1) {
        perror ("Client msgq: mq_open (qd_client)");
        exit (1);
    }

    while (1) {
		printf("Enter Instruction: ");
		scanf("%s %s", itr_name, args);
		sprintf (out_msg.itr, "%d", get_itr_code(itr_name));
		//printf("Enter Arguements: ");
		//scanf("%s", args);
		sprintf(out_msg.args, "%s", args);
		//printf("you entered %s %s\n", itr_name, args);
		
		if (strlen(args) == 0) {
			perror("No arguements passed\n");
			continue;
		}       
        // send message to my_msgq_rx queue
        // int mq_send(mqd_t mqdes, const char *msg_ptr, size_t msg_len, unsigned int msg_prio);
        if (mq_send (qd_srv, (char *) &out_msg, sizeof(out_msg), 0) == -1) {
            perror ("Client MsgQ: Not able to send message to the queue /server_msgq");
            continue;
        }

        printf ("Client MsgQ: Message sent successfully\n");

        sleep(2);  // sleep for 5 seconds
        //num++;

		server_msg_t in_msg;
        // ssize_t mq_receive(mqd_t mqdes, char *msg_ptr, size_t msg_len, unsigned int *msg_prio);
        if (mq_receive(qd_client,(char *) &in_msg, MAX_MSG_SIZE, NULL) == -1) {
            perror ("Client MsgQ: mq_receive from server");
            exit (1);
        }
        
        //int val_server = atoi(in_msg.msg_val);
        printf("Client MsgQ: Msg received from the server: %s\n\n", in_msg.msg_val);

    }

    printf ("Client MsgQ: bye\n");

    exit (0);
}
