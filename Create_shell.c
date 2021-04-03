// Creating a Shell
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#define MAX_FILE_NAME_CHARS 255
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include <dirent.h>
#include <stddef.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#define BUFFER_SIZE 50
/*functins*/

int string_in(char string [], const char** strings, size_t strings_num) {
    for (size_t i = 0; i < strings_num; i++) {
        if (!strcmp(string, strings[i])) {
            return i;
        }
    }
    return -1;
}

void catCommand(char **command){

        if (strcmp(command[1],">")==0){
            char *fileName=command[2];
            if(fileName != "\0"){
                char *args[]={command[0],NULL};
                int oldFile = creat(fileName,0644);
                int newFile = dup2(oldFile,STDOUT_FILENO);
                close(oldFile);
                execv("/bin/cat",args);
                close(newFile);
            }
            else
                printf("file does not exist\n");
        }
}
int execvp(const char *file, char *const argv[]);

char **get_input(char *input) {
    char **command = (char**)malloc(sizeof(char *)*BUFFER_SIZE);
    if(!command){
	printf("Bad allocation");
	exit(1);
    }
    char *separator = " ";
    char *parsed;
    int index = 0;
    parsed = strtok(input, separator);
    while (parsed != NULL) {
        command[index] = parsed;
        index++;
        parsed = strtok(NULL, separator);
    }
    command[index] = NULL;
    return command;
}

void pipeCommand(char** command1 , char** command2){
	int fds[2];
	pid_t pid;
	pipe(fds);
	pid=fork();
	if (pid==0){
		close(fds[0]);
		dup2(fds[1],1);
		execvp(command1[0], command1);

	}
	else{
		close(fds[1]);
		wait(NULL);
		dup2(fds[0],0);
		execvp(command2[0], command2);
		
	}
	if(pid!=0)
		waitpid(pid,NULL, WUNTRACED);


}
void cheakCommendsPipeInCmds(char* st,const char** cmds, size_t lenCmds){
	char commend1[500000];
	char commend2[5000000];
	int index=0;
	for (int i=0;i<strlen(st);i++)
	{
		if(st[i]!='|')
		{
			commend1[i]=st[i];
		}
		if(st[i]=='|')
			index=i;
	}
	for(int i=index+1, j=0; i<strlen(st);i++,j++)
		commend2[j]=st[i];
 	char **com1 = get_input(commend1);
	int indexCmds1 = string_in(com1[0], cmds, lenCmds);
	char **com2 = get_input(commend2);
	int indexCmds2 = string_in(com2[0], cmds, lenCmds);
	if( indexCmds1!=-1 && indexCmds2!=-1)
	{
		if((strcmp(com2[0],"sort")==0 && com2[1]==NULL))
			pipeCommand(com1 ,com2);
		else if(strcmp(com2[0],"grep")==0 && (strcmp(com2[1],"-n")==0 ||strcmp(com2[1],"-r")==0)){
			printf("no commend\n");
			return;
		}
		else if(strcmp(com2[0],"sort")==0 && (strcmp(com2[1],"-c")==0 || strcmp(com2[1],"-n")==0))
		{
			printf("no commend\n");
			return;
		}	
		else if((strcmp(com2[0],"grep")==0 && strcmp(com2[1],"-c")==0))
			pipeCommand(com1 ,com2);
		else if(strcmp(com2[0],"sort")!=0 ||strcmp(com2[0],"grep")!=0)
			pipeCommand(com1 ,com2);
		else
			pipeCommand(com1 ,com2);
	}
	else
		printf("no commend\n");

}
void currWorDir(){
	char cwd[1024];	
	getcwd(cwd,sizeof(cwd));
	printf("%s\n",cwd);
}
void changeDirectory(){
	char s[100];
	chdir("..");
}


int main(int argc, char*argv[]){
	char **command;
	char input[100];
	char *st ;
	const char* cmds[] = {"pwd","cd","wc","cp","nano","man","sort","sort -r","grep","grep -c","cat","cat >","exit"};
	int lenCmds = sizeof(cmds)/sizeof(cmds[0]);
	int indexCmds;
	int flag=0;
	pid_t child_pid;
	int stat_loc;
	FILE *fp;
 	char file_name[MAX_FILE_NAME_CHARS], ch;
 	int i;
	while(1){
		printf("Enter a command: ");
		fgets(input,100,stdin);
		st=input;
		int len=strlen(st);
		char* st2;
		if(st[len-1]=='\n'&&len>0)
			st[len-1]='\0';


		for (int i=0;i<len;i++)
			st2[i]=st[i];
		int len2=strlen(st2);
		command = get_input(st);
		indexCmds = string_in(command[0], cmds, lenCmds);

		//Menu for executing the written command
		if (!command[0]) {      
            		free(st);
            		free(command);
            		continue;
       		 }
			
		child_pid = fork();	
		if (child_pid < 0) {
		    perror("Fork failed");
		    exit(1);
		}
		if (child_pid == 0) {

			//commamd is pipe
			int isPipe = 0;
			int countPipe=0;
			for(i=0;i<len2;i++)
			{
				if(st2[i]=='|')
				{
					isPipe=1;
					countPipe+=1;
				}
				
			}
			if(isPipe==1 && countPipe==1)
			{
				cheakCommendsPipeInCmds(st2, cmds, lenCmds);	
			}			
			else if(isPipe==1 && countPipe!=1)
				printf("pipe command is not supported\n");
			if(indexCmds==0 && isPipe==0){
				currWorDir();
			}
			else if(indexCmds==1 && isPipe==0){
				if (strcmp(command[1],"..")==0)
					changeDirectory();
			}
			//include wc( wc -l, wc -w, wc -c) man ,nano, cp
			else if((indexCmds==2 || indexCmds==3 || indexCmds==4 || indexCmds==5 ) && isPipe==0){
			execvp(command[0], command);
			}
			else if((indexCmds==6 || indexCmds==7) && isPipe==0){ // sort and sort -r
				if(strlen(command[1])>2) // is sort
				{
                                      execlp("sort",command[0],command[1],NULL);
				}				
				else if(strcmp(command[1],"-r")==0 && strlen(command[1])==2) //is sort -r
				{	 				execlp("sort",command[0],command[1],command[2],NULL);
				}
				else
					printf("'%s %s' command is not supported\n",command[0],command[1]); 
 				
			}

			else if((indexCmds==8 || indexCmds==9) && isPipe ==0){ //grep and grep -c

				if(strlen(command[1])>2) //is grep
				{

					execlp("grep", command[0],command[1], command[2], NULL);
				}
				else if(strcmp(command[1],"-c")==0 && strlen(command[1])==2){ //is grep -c						
					execlp("grep", command[0],command[1], command[2],command[3], NULL);

				}
				else
					printf("'%s %s' command is not supported\n",command[0],command[1]); 				
			}
			else if((indexCmds==10 || indexCmds==11) && isPipe ==0){ //cat and cat>
				if(strlen(command[1])>2)//cat
				{
					
				execlp("cat",command[0],command[1],NULL);
				}
				else if(strcmp(command[1],">")==0 && strlen(command[1])==1) //cat>
				{

					catCommand(command);
  
				}
			}
			else if (indexCmds==12){
				if(child_pid == 0)
				kill(0,	SIGINT);

			}
			else if(indexCmds!=1 || indexCmds!=2 ||indexCmds!=3||indexCmds!=4||indexCmds!=5 || 
				indexCmds!=6||indexCmds!=7||indexCmds!=8||indexCmds!=9||indexCmds!=10 ||
				indexCmds!=11||indexCmds!=12)
				printf("'%s' command is not supported\n",st);
		}
		
		else {
			
		    waitpid(child_pid, &stat_loc, WUNTRACED);
		}

        free(command);
	
	}

return 0;

}







