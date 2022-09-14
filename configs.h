#ifndef CONFIGS_DEFINED

#define CONFIGS_DEFINED 1

#define MSG_VAL_LEN  16
#define MSG_TYPE_LEN 16
#define CLIENT_Q_NAME_LEN 16
#define MAX_ITR_LEN 2
#define MAX_ITR_NAME_LEN 16
#define MAX_ARGS_LEN 14

#define MAX_COURSES 15
#define MAX_TEACHERS 10

#define SERVER_QUEUE_NAME   "/server_msgq"
#define QUEUE_PERMISSIONS 0660
#define MAX_MESSAGES 10
#define MAX_MSG_SIZE sizeof(client_msg_t)
#define MSG_BUFFER_SIZE (MAX_MSG_SIZE * MAX_MESSAGES)

#define COURSE_FILE_NAME "course_data.txt"
#define TEACHER_FILE_NAME "teacher_data.txt"

typedef struct{
char client_q[CLIENT_Q_NAME_LEN];
char itr[MAX_ITR_LEN];
char args[MAX_ARGS_LEN];
} client_msg_t;

typedef struct{
char msg_type[MSG_TYPE_LEN];
char msg_val[MSG_VAL_LEN];
} server_msg_t;

#endif
