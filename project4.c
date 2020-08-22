
/***
 * Name: 	Wenda Zhao
 * BlazerId: 	wenda
 * Project #: 	4
 * Comment: 	
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <signal.h>

#define RUN_SH 1
#define QUIT_SH 0

int ChildFinish_flag = 0;
int ChildFinish_count = 0;
int gp;

typedef struct job_struct{
	int jobid;
	char *command;
	char *status;
	//char *start_time；
	//char *end_time;
} JOB; 

void createarray_2(char *buf, char **array) {
	int i, count;
	int blank_count;
	buf[strlen(buf)] = '\0';
	
	for (i = 0, array[0] = &buf[0], count = 1; i < strlen(buf); i++) {
		if (buf[i] == ' ') {
		   buf[i] = '\0';
		   array[count++] = &buf[i+1];
		}
	}
	array[count] = (char *)NULL;
}

void createarray_n(char *buf, char **array) {
	int i, count;
	buf[strlen(buf)] = '\0';
	char *p = strtok(buf, " ");
	i = 0;
	while(p != NULL){
		array[i++] = p;
		p = strtok(NULL, " ");
	}
	count = i;
	array[count] = (char *)NULL;
}

void handler(int sig){
	pid_t id;
	while((id = waitpid(-1,NULL,WNOHANG))>0){
		printf("wait child success:%d! \n",id);
		printf("Enter command> ");
	}
	ChildFinish_flag = 1;
	ChildFinish_count++;
	printf("child is quit!%d \n",getpid());
	printf("Enter command> ");
}

int main(int argc, char **argv) {
	int status;
	int P,i;
	int fdin, fdout;
	char firstline[] = "jobid\tcommand\tstatus\tstart_time\tend_time\n";
	char firstline0[] = "jobid\tcommand\tstatus\n";
	char input_command[BUFSIZ], buf[BUFSIZ], *args1[BUFSIZ];
	char *args2[BUFSIZ];
	
	int run_flag = RUN_SH;
	const char *quit_str = "quit";
	const char *submit_str = "submit";
	const char *show_str = "showjobs";
	const char *waiting_str = "Waiting";
	const char *history_str = "submithistory";
	const char *enter = "\n";
	const char *table = "\t";
	const char *scss = "Seccuss";
	char log;
	
	int running_number=0;
	int waiting_number=0;
	int current_number=0;
	int firstwait;
	int finish_number;
	int waitingid;
	
	//time_t t;
	//struct tm *timeinfo;
	//char *start_time, *end_time;
	
	
	JOB *pjob;
	pjob = (JOB *)malloc(sizeof(JOB)*20);
	
	FILE *fp;
	
	if (argc != 2) {
       printf("Usage: %s <MAX # of running process>\n",argv[0]);
       exit(-1);
    }
	P = atoi(argv[1]);
	gp = P;
	
	pid_t pid_array[20];
	
	//printf("writing file: history.log \n");
	fp = fopen("history.log","w");
	if (fp == NULL) {
		printf("Error opening file history.log for writing\n");
		exit(-1);
	}
	fwrite(firstline, strlen(firstline), 1, fp);
	//fwrite(enter, strlen(enter), 1, fp);
	fclose(fp);
	//printf("finished writing file: history.log \n");
	
	while(run_flag){
		printf("Enter command> ");
		gets(input_command);
		createarray_2(input_command, args1);
		
		if (signal(SIGCHLD, handler) == SIG_ERR) {
			printf("can't catch SIGCHLD\n");
			exit(-1);
		}
		if(ChildFinish_flag==1){
			running_number--;
			
			
			
			if(waiting_number>0){
				printf("waiting task is detected\n");
				for(i=0; pjob[i].jobid!= NULL;i++){
					if(strcmp(pjob[i].status, waiting_str)==0){
						waitingid = i;
						printf("Now running a waiting task\n");
						pid_array[i] = fork();
						if (pid_array[i]==0){
							printf("in waited child\n");
							createarray_n(pjob[i].command, args2);
							
							fp = fopen("history.log","a");
							if (fp == NULL) {
								printf("Error opening file history.log for writing\n");
								exit(-1);
							}
							//fwrite(pjob[i].jobid, sizeof(int), 1, fp);
							fwrite(table, strlen(table), 1, fp);
							fwrite(pjob[i].command, strlen(pjob[i].command), 1, fp);
							fwrite(table, strlen(table), 1, fp);
							fwrite(scss, strlen(scss), 1, fp);
							fwrite(enter, strlen(enter), 1, fp);
							fclose(fp);
							
							execvp(args2[0], args2);
							//sleep(20);
							perror("exec");
							exit(-1);
						}
						else if (pid_array[i] > 0) {
							pjob[i].status = "Running";
							waiting_number--;
							running_number++;
						}
						else{
							perror("fork");
							exit(EXIT_FAILURE);
						}
					}
				}
			}
			ChildFinish_flag = 0;
		}
		
		
		
		if(strcmp(args1[0], history_str)==0){
			fp = fopen("history.log","r");
			if (fp == NULL) {
				printf("Error opening file history.log for reading\n");
				exit(-1);
			}
			while((log=fgetc(fp))!=EOF){
				putchar(log);
			}
			putchar('\n');
			fclose(fp);
		}
		else if(strcmp(args1[0], submit_str)==0){
			printf("job %d added to the queue\n", current_number);
			pjob[current_number].jobid = current_number;
			pjob[current_number].command = args1[1];
			
			if (running_number<P && waiting_number==0){
				pjob[current_number].status = "Running";
				
				pid_array[current_number] = fork();
				if(pid_array[current_number]==0){
					printf("in running child\n");
					printf("%s\n", pjob[current_number].command);
					createarray_n(pjob[current_number].command, args2);
					
					fp = fopen("history.log","a");
					if (fp == NULL) {
						printf("Error opening file history.log for writing\n");
						exit(-1);
					}
					//fwrite(pjob[current_number].jobid, sizeof(int), 1, fp);
					fwrite(table, strlen(table), 1, fp);
					fwrite(pjob[current_number].command, strlen(pjob[current_number].command), 1, fp);
					fwrite(table, strlen(table), 1, fp);
					fwrite(scss, strlen(scss), 1, fp);
					fwrite(enter, strlen(enter), 1, fp);
					fclose(fp);
					
					//printf("args2[0]= %d\n", args2[0]);
					running_number++;
					current_number++;
					execvp(args2[0], args2);
					
					perror("exec");
					exit(-1);
				}
				else if (pid_array[i] > 0) {
					pjob[current_number].status = "Running";
					current_number++;
					
				}
				else{
					perror("fork");
					exit(EXIT_FAILURE);
				}
			}
			
			
			// add to waiting queue
			else if (running_number==(P)){
				pjob[current_number].status = "Waiting";
				waiting_number++;
				current_number++;
			}
		}
		else if(strcmp(args1[0], show_str)==0){
			for(i=ChildFinish_count; pjob[i].jobid!= NULL;i++){
				printf("%s", firstline0);
				printf("%d\t", pjob[i].jobid);
				printf("%s\t", pjob[i].command);
				printf("%s\t", pjob[i].status);
				printf("\n");
			}
		}
		else if (strcmp(args1[0], quit_str)==0){
			run_flag = QUIT_SH;
			printf("Now quit blazersh...\n");
			exit(0);
		}
		else {
				printf("Please input correct command \n");
				exit(-1);
		}
		
		
	}

    printf("[%ld]: Exiting main program .....\n", (long)getpid());

    return 0;	
}
