#ifndef _mdw_CHILD_H
#define _mdw_CHILD_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>

extern int startchild(char *cmd, FILE **readpipe, FILE **writepipe);

#endif
