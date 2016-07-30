
#include <stdio.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include <string.h>

typedef struct _generic_msg {
  long mtype;
  char mtext[16];
} msg_t;

int
main()
{
  int id;
  int res;
  msg_t msg;
  
  id = msgget(100, 0);
  if (id < 0) {
    perror("msgget");
    exit(1);
  }

  msg.mtype = 1;
  strncpy(msg.mtext, "TESTING", 7);

  res = msgsnd(id, &msg, sizeof(msg_t), 0);
  if (res < 0) {
    perror("msgsnd");
    exit(1);
  }

  return 0;
}
