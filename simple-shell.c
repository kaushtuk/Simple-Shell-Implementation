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
int history_wait[10];
int buffHead = 0;
void init_history(void);
void free_history(void);
void print_history(void);
char** history_computation(char** args,int *needWait);
int main(void)
{
	char *args[MAX_LINE/2 + 1];	/* command line (of 80) has max of 40 arguments */
    int should_run = 1;
	
	/*int i, upper;*/
		
    init_history();
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
                return 1;
            }
            continue;
        }
        //Parse cmd_line
        while(*sptr==' ' || *sptr=='\t')
            sptr++;
        while(*sptr!='\0'){
            char *tempBuff=(char*)malloc((MAX_LINE+1)*sizeof(char));
            args[av]=(char*)malloc((MAX_LINE+1)*sizeof(char));
            int ret = sscanf(sptr,"%[^ \t]",args[av]);
            sptr += strlen(args[av]);
            if(ret<1){
                printf("INVALID COMMAND\n");
                return 1;
            }
            ret = sscanf(sptr,"%[ \t]",tempBuff);
            if(ret>0)
                sptr += strlen(tempBuff);
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
        char **argsPtr = history_computation(args, &need_to_wait);
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
char** history_computation(char **args, int *needWait) {
    int i;
    if(args[1]==NULL && strcmp(args[0],"!!")==0) {
        if(buffHead>0){
            strcpy(args[0],history[(buffHead-1)%10][0]);
            for(i=1;history[(buffHead-1)%10][i]!=NULL;i++) {
                args[i]=(char*)malloc((MAX_LINE+1)*sizeof(char));
                strcpy(args[i],history[(buffHead-1)%10][i]);
            }
            args[i]=NULL;
            *needWait=history_wait[(buffHead-1)%10];
        } else {
            printf("NO COMMANDS IN HISTORY\n");
            return args;
        }
    } else if(args[1]==NULL && args[0][0]=='!') {
        int idx;
        char *sptr=&(args[0][1]);
        if(sscanf(sptr,"%d",&idx)==1) {
            if(idx>0 && buffHead>idx-1 && idx>buffHead-9) {
                strcpy(args[0],history[(idx-1)%10][0]);
                for(i=1;history[(idx-1)%10][i]!=NULL;i++) {
                    args[i]=(char*)malloc((MAX_LINE+1)*sizeof(char));
                    strcpy(args[i],history[(idx-1)%10][i]);
                }
                args[i]=NULL;
                *needWait=history_wait[(idx-1)%10];
            } else {
                printf("NO SUCH COMMAND IN HISTORY(index out of range)\n");
                return args;
            }
        } else {
            printf("NO SUCH COMMAND IN HISTORY(invalid index)\n");
            return args;
        }
    }

    for(i=0;i<(MAX_LINE/2+1) && history[buffHead%10][i]!=NULL;i++)
        free(history[buffHead%10][i]);
    for(i=0;args[i]!=NULL;i++) {
        history[buffHead%10][i]=args[i];
    }
    history[buffHead%10][i]=args[i];
    history_wait[buffHead%10]=*needWait;
    return history[(buffHead++)%10];
}
void init_history(void) {
    int i,j;
    for(i=0;i<10;i++) {
        for(j=0;j<(MAX_LINE/2+1);j++) {
            history[i][j]=NULL;
        }
        history_wait[i]=0;
    }
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
        int index;
        if(buffHead>10)
            index=buffHead-9+i;
        else
            index=i+1;
        printf("[%d] ",index);
        for(j=0;history[(index-1)%10][j]!=NULL;j++) {
            printf("%s ",history[(index-1)%10][j]);
        }
        if(history_wait[(index-1)%10]==0) {
            printf("&");
        }
        printf("\n");
    }
}
