#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#define STACK_SIZE (1024 * 1024)    /* Stack size for cloned child */

static int child_fn(void *arg) {
    char input;
    printf("pid child namespace: %ld\n", (long)getpid());
    printf("Type q to exit or nsenter to enter the namespace\n"); 
    while (1) {
        scanf("%c", &input);
        printf("You typed: %c\n", input);
        if (0 == strcmp("q", &input)) {
           return 0;
        }
    }
}

int main() {
    char *stack = malloc(STACK_SIZE);
    if (stack == NULL) {
       printf("malloc error\n");
    }

    pid_t child_pid = clone(child_fn, stack+STACK_SIZE, CLONE_NEWPID | SIGCHLD, NULL);
    if (-1 == child_pid) {  
       printf("clone failed: (%s)\n", strerror(errno));
       if (1 == errno) {
          printf("Did you use sudo?\n");
       }
       return -1;
    }
    printf("pid in parent namespace: %ld\n", (long)child_pid);

    sleep(1);
    waitpid(child_pid, NULL, 0);
    return 0;
} 
