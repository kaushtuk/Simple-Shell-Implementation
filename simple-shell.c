/**
 * Simple shell interface program.
 *
 * Operating System Concepts - Ninth Edition
 * Copyright John Wiley & Sons - 2013
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>



#define MAX_LINE		80 /* 80 chars per line, per command */

char* history[10][MAX_LINE/2 + 1];
int buffHead = 0;
void free_history(void);
void print_history(void);
char** history_computation(char** args);
int main(void)
{
	char *args[MAX_LINE/2 + 1];	/* command line (of 80) has max of 40 arguments */
    int should_run = 1;
	
	/*int i, upper;*/
		
    while (should_run){   
        printf("osh>");
        fflush(stdout);

        pid_t pid;

        char cmd_line[MAX_LINE+1];
        char *sptr = cmd_line;
        int av=0;
        if(scanf("%[^\n]%*1[\n]",cmd_line)<1) {
            if(scanf("%1[\n]",cmd_line)<1) {
                printf("STDIN FAILED\n");
            }
            continue;
        }
        //Parse cmd_line
        while(*sptr==' ' || *sptr=='\t')
            sptr++;
        while(*sptr!='\0'){
            char *tempBuff=(char*)malloc((MAX_LINE+1)*sizeof(char));
            args[av]=(char*)malloc((MAX_LINE+1)*sizeof(char));
            int ret = sscanf(sptr,"%[^ \t]%[ \t]",args[av],tempBuff);
            if(ret<1){
                printf("INVALID COMMAND\n");
                return 1;
            }
            sptr += (strlen(args[av])+strlen(tempBuff));
            av++;
            free(tempBuff);
        }
        int need_to_wait = 1;
        if(strlen(args[av-1])==1 && args[av-1][0]=='&') {
            need_to_wait = 0;
            free(args[av-1]);
            args[av-1]=NULL;
        } else {
            args[av]=NULL;
        }
        if(strcmp(args[0],"exit")==0){
            free_history();
            return 0;
        }
        //History Computation
        if(args[1]==NULL && strcmp(args[0],"history")==0) {
            print_history();
            continue;
        }
        char **argsPtr = history_computation(args);
        //Fork child to Execute args
        pid = fork();
        if(pid<0) {
            printf("FORK FAILED\n");
            return 1;
        } else if(pid==0) {
            if(execvp(argsPtr[0],argsPtr)) {
                printf("INVALID COMMAND\n");
                return 1;
            }
        } else {
            if(need_to_wait) {
                while(wait(NULL) != pid);
            }
            else {
                printf("[1]%d\n",pid);
            }
        }
        /**
         * After reading user input, the steps are:
         * (1) fork a child process
         * (2) the child process will invoke execvp()
         * (3) if command did not include &, parent will invoke wait()
         */
    }
    
	return 0;
}
char** history_computation(char **args) {
    int i;
    for(i=0;i<(MAX_LINE/2+1);i++) {
        if(history[buffHead%10][i])
            free(history[buffHead%10][i]);
        history[buffHead][i]=args[i];
    }
    return history[(buffHead++)%10];
}
void free_history(void) {
    int i,j;
    for(i=0;i<10 && i<buffHead;i++) {
        for(j=0;history[i][j]!=NULL;j++) {
            if(history[i][j])
                free(history[i][j]);
        }
    }
}
void print_history(void) {
    int i,j;
    for(i=0;i<10 && i<buffHead;i++) {
        printf("[%d] ",buffHead-i);
        for(j=0;history[i][j]!=NULL;j++) {
            printf("%s ",history[i][j]);
        }
        printf("\n");
    }
}
