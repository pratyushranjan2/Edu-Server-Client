#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <signal.h>
#include "configs.h"
#include "utils.h"
#include "sigtypes.h"

// configuration definitions are in configs.h
// utility definitions are in utils.h

static client_msg_t client_msg;

char courses[nCOURSES][COURSE_LEN] = {"\0"};
char teachers[nTEACHERS][TEACHER_LEN] = {"\0"};
int i_courses = 0, i_teachers = 0, n_courses = 0, n_teachers = 0;
sem_t bin_sem;

void* thread_fn(void *arg);
void sigint_handler(int sig);

SIGHANDLER pSigintHandler;
SIGHANDLER pDefaultHandler;

// function to check for duplicate courses
int duplicate_course(char *name) {
	for (int i=0;i<i_courses;i++) {
		if (strcmp(courses[i], name) == 0) {
			return 1;
		}
	}
	return 0;
}

// function to check for duplicate teachers
int duplicate_teacher(char *name) {
	for (int i=0;i<i_teachers;i++) {
		if (strcmp(teachers[i], name) == 0) {
			return 1;
		}
	}
	return 0;
}

int add_course(char args[MAX_ARGS_LEN]) {
	int duplicate = 0;
	char *token = strtok(args, ",");
   	while( token != NULL ) {
		  if (duplicate_course(token)) {
		  	duplicate = 1;
		  	token = strtok(NULL, ",");
		  	continue;
		  }
		  strcpy(courses[i_courses++], token);
		  n_courses++;
	  	  token = strtok(NULL, ",");
   }
   return duplicate ? WARNING_DUPLICATE : SUCCESS;
}

int add_teacher(char args[MAX_ARGS_LEN]) {
	int duplicate = 0;
	char *token = strtok(args, ",");
   	while( token != NULL ) {
		  if (duplicate_teacher(token)) {
		  	duplicate = 1;
		  	token = strtok(NULL, ",");
		  	continue;
		  }
		  strcpy(teachers[i_teachers++], token);
		  n_teachers++;
	  	  token = strtok(NULL, ",");
   }
   return duplicate ? WARNING_DUPLICATE : SUCCESS;
}

void delete_course(char args[MAX_ARGS_LEN]) {
	char *token = strtok(args, ",");
   	while( token != NULL ) {
		  for (int i=0;i<i_courses;i++) {
		  		if (strcmp(token, courses[i]) == 0) {
		  			strcpy(courses[i], "\0");
		  			n_courses--;
		  		}
		  }
	  	  token = strtok(NULL, ",");
   }
}

void delete_teacher(char args[MAX_ARGS_LEN]) {
	char *token = strtok(args, ",");
   	while( token != NULL ) {
		  for (int i=0;i<i_teachers;i++) {
		  		if (strcmp(token, teachers[i]) == 0) {
		  			strcpy(teachers[i], "\0");
		  			n_teachers--;
		  		}
		  }
	  	  token = strtok(NULL, ",");
   }
}

void print_courses() {
	printf("(");
	for (int i=0;i<i_courses-1;i++) {
		printf("%s, ", courses[i]);
	}
	printf("%s)\n", courses[i_courses-1]);
}

void print_teachers() {
	printf("(");
	for (int i=0;i<i_teachers-1;i++) {
		printf("%s, ", teachers[i]);
	}
	printf("%s)\n", teachers[i_teachers-1]);
}

void assign() {
	printf("---------\n");
	int i=0, j=0;
	while (i<i_courses && j<i_teachers) {
		if (strcmp(courses[i], "\0") != 0 && strcmp(teachers[j], "\0") != 0) {
			printf("%s --> %s\n", courses[i++], teachers[j++]);
		}
		else if (strcmp(courses[i], "\0") != 0 && strcmp(teachers[j], "\0") == 0) {
			j++;
		}
		else if (strcmp(courses[i], "\0") == 0 && strcmp(teachers[j], "\0") != 0) {
			i++;
		}
		else {
			i++;j++;
		}
	}
	printf("---------\n\n");
}

// function for handling instructions from client
char* handler(int itr, char args[MAX_ARGS_LEN]) {
	int status;
	if (itr == CODE_ADD_TEACHER) {
		if (n_teachers == MAX_TEACHERS) {
			return MAX_LIMIT_MSG;
		}
		status = add_teacher(args);
		return status == SUCCESS ? ADD_SUCCESS_MSG : ADD_DUPLICATE_MSG;
	}
	else if (itr == CODE_ADD_COURSE) {
		if (n_courses == MAX_COURSES) {
			return MAX_LIMIT_MSG;
		}
		status = add_course(args);
		return status == SUCCESS ? ADD_SUCCESS_MSG : ADD_DUPLICATE_MSG;
	}
	else if (itr == CODE_DEL_TEACHER) {
		delete_teacher(args);
		return DEL_MSG;
	}
	else if (itr == CODE_DEL_COURSE) {
		delete_course(args);
		return DEL_MSG;
	}
	return INVALID_ITR_MSG;
}

// sigint handler function
void sigint_handler(int sig) {
	FILE* course_fptr = fopen(COURSE_FILE_NAME, "w");
	FILE* teacher_fptr = fopen(TEACHER_FILE_NAME, "w");
	if (course_fptr == NULL || teacher_fptr == NULL) {
		perror("\nFile error\n");
		exit(0);
	}
	
	for (int i=0;i<i_courses;i++) {
		if (strcmp(courses[i], "\0") != 0) {
			fprintf(course_fptr, "%s\n", courses[i]);
		}
	}
	
	for (int i=0;i<i_teachers;i++) {
		if (strcmp(teachers[i], "\0") != 0) {
			fprintf(teacher_fptr, "%s\n", teachers[i]);
		}
	}
	
	fclose(course_fptr);
	fclose(teacher_fptr);
	printf("\nData saved to %s and %s\n", COURSE_FILE_NAME, TEACHER_FILE_NAME);
	exit(0);
}

int main (int argc, char **argv)
{
    mqd_t qd_srv, qd_client;   // Server and Client Msg queue descriptors
    int num = 1, stat;
    char msg_thread[] = "Thread created from main thread";
    pthread_t thread_id;
    void *thread_result;
    
    pSigintHandler = sigint_handler;
    pDefaultHandler = signal(SIGINT, pSigintHandler);
    
    stat = pthread_create(&thread_id, NULL, thread_fn, (void *)msg_thread);
    if(stat != 0){ //thread creation failure
		perror("Server: Main: Error: Thread creation failed\n");
		exit(EXIT_FAILURE);
	}
	
	stat = sem_init(&bin_sem, 0, 1);  
  
    if(stat != 0) {
	  printf("Semaphore creation failure: %d\n", stat);
 	  exit(1);
    }  

    printf ("EDU SERVER: Welcome!!!\n");

    struct mq_attr attr;

    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;

    if ((qd_srv = mq_open(SERVER_QUEUE_NAME, O_RDONLY | O_CREAT, QUEUE_PERMISSIONS,
                           &attr)) == -1) {
        perror ("Server MsgQ: mq_open (qd_srv)");
        exit (1);
    }

    client_msg_t in_msg;
	//int val_client;
	//char itr[MAX_ITR_LEN];
	int itr;
	char args[MAX_ARGS_LEN];
    while (1) {
        // ssize_t mq_receive(mqd_t mqdes, char *msg_ptr, size_t msg_len, unsigned int *msg_prio);
        if (mq_receive(qd_srv,(char *) &in_msg, MAX_MSG_SIZE, NULL) == -1) {
            perror ("Server msgq: mq_receive");
            exit (1);
        }

		//val_client = atoi(in_msg.msg_val);
		//strcpy(itr, in_msg.itr);
		itr = atoi(in_msg.itr);
		strcpy(args, in_msg.args);
		
		sem_wait(&bin_sem); // semaphore acquired while handling request
		char *msg = handler(itr, args);
		sem_post(&bin_sem); // semaphore released after handling request
		
		//print_courses();
		//print_teachers();
		//assign();
		
		
        printf ("%d: Server MsgQ: message received.\n", num);
        printf("Client msg q name = %s\n", in_msg.client_q);
        printf("Client instruction_code = %d\n", itr);
        num++;

		server_msg_t out_msg; 
		strcpy(out_msg.msg_type, "Server msg");   // strcpy(destPtr, srcPtr)
		sprintf (out_msg.msg_val, "%s", msg);    
		             		       
		// Open the client queue using the client queue name received
        if ((qd_client = mq_open(in_msg.client_q, O_WRONLY)) == 1) {
            perror ("Server MsgQ: Not able to open the client queue");
            continue;
        }     
        
        // Send back the value received + 10 to the client's queue           
        // int mq_send(mqd_t mqdes, const char *msg_ptr, size_t msg_len, unsigned int msg_prio);
        if (mq_send(qd_client, (char *) &out_msg, sizeof(out_msg), 0) == -1) {
            perror ("Server MsgQ: Not able to send message to the client queue");
            continue;
        }
        printf("\n");  
            
    } // end of while(1)
    
    stat = pthread_join(thread_id, &thread_result);
    if(stat != 0){ // thread join has failed
		perror("Thread join failed\n");
		exit(EXIT_FAILURE);    
	} 
    
}  // end of main()

void* thread_fn(void *arg) {
	while (1) {
		sleep(10);
		sem_wait(&bin_sem); // semaphore acquired before reading data
		assign();
		sem_post(&bin_sem); // semaphore released after finishing reading data
	}
	pthread_exit("Thread: Thread exiting");
}
