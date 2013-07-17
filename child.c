/* child.c */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include "child.h"

/* Exec the named cmd as a child process, returning
 * two pipes to communicate with the process, and
 * the child's process ID */
int start_child(char *cmd, FILE **readpipe, FILE **writepipe) {
    int childpid, pipe1[2], pipe2[2];

    if ((pipe(pipe1) < 0) || (pipe(pipe2) < 0) ) {
       perror("pipe"); exit(-1);
    }

    if ((childpid = fork()) < 0) {
       perror("fork"); exit(-1);
    } else if (childpid > 0) {   /* Parent. */
       close(pipe1[0]); close(pipe2[1]);
       /* Write to child on pipe1[1], read from child on pipe2[0]. */
       *readpipe = fdopen(pipe2[0], "r");
       *writepipe = fdopen(pipe1[1], "w");
       setlinebuf(*writepipe);
       return childpid;

    } else { /* Child. */
       close(pipe1[1]); close(pipe2[0]);
       /* Read from parent on pipe1[0], write to parent on pipe2[1]. */
       dup2(pipe1[0],0); dup2(pipe2[1],1);
       close(pipe1[0]); close(pipe2[1]);

       if (execlp(cmd, cmd, NULL) < 0)
          perror("execlp");
       /* Never returns */
    }
}
