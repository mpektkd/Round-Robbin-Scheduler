#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/signal.h>
#include <string.h>
#include <assert.h>

#include <sys/wait.h>
#include <sys/types.h>

#include "proc-common.h"
#include "request.h"

#include "queue2.h"

/* Compile-time parameters. */
#define SCHED_TQ_SEC 2                /* time quantum */
#define TASK_NAME_SZ 60               /* maximum size for a task's name */
#define SHELL_EXECUTABLE_NAME "shell" /* executable for shell */


/*global queue*/
struct Queue *queue_low;
struct Queue *queue_high;
/* Print a list of all tasks currently being scheduled.  */
static void
sched_print_tasks(void)
{   
    struct QNode *q; 
    if(queue_high->front != NULL){
        q = queue_high -> front;
        printf("The high priority list of tasks\n");
	if(queue_high->size == 0){
        printf("Empty\n");
    }
    else{
    while(q != NULL){
		printf("Proccess:%s, with id:%d and PID:%d\n",q -> name,q -> id, q -> key);
		q = q -> next;
	    }
        printf("%d\n",queue_high->size);
    }
    }
	q = queue_low -> front;
    printf("The low priority list of tasks\n");
    if(queue_low->size == 0){
        printf("Empty\n");
    }
        else{
        while(q != NULL){
		    printf("Proccess:%s, with id:%d and PID:%d\n",q -> name,q -> id, q -> key);
		    q = q -> next;
	        }
            printf("%d\n",queue_low->size);
        }
    
}

/* Send SIGKILL to a task determined by the value of its
 * scheduler-specific id.
 */
static int
sched_kill_task_by_id(int id)
{	
    printf("i'm here\n");
    struct QNode *q;
    if(queue_high->size != 0){
        q = queue_high -> front;
        printf("i'm here\n");
	    while(q != NULL){
            printf("i'm here\n");
		    if(q->id == id)
				break;
			q = q -> next;	
            }
            printf("i'm here\n");
        if(q != NULL){
	        printf("Proccess %s, with id:%d and PID:%d has been killed\n",q -> name, q -> id, q -> key);
	        if(kill(q -> key, SIGKILL) < 0){
	    	    perror("error at SIGKILL\n");
		        exit(1);
	            }
            return id;    
        }
    }
    if(queue_low->size != 0){
	    q = queue_low -> front;
	    while(q != NULL){
		    if(q->id == id)
                break;
			q = q -> next;
	        }
        if(q != NULL){
	        printf("Proccess %s, with id:%d and PID:%d has been killed\n",q -> name, q -> id, q -> key);
	        if(kill(q -> key, SIGKILL) < 0){
		        perror("error at SIGKILL\n");
		        exit(1);
	        }

	
	    return id;
        }
    }
    printf("Bad id\n");
    return -1;

}

/* Create a new task.  */
static void
sched_create_task(char *executable)
{
	pid_t pid = fork();

	if(pid < 0){
		perror("error at fork\n");
		exit(1);
	}
	if(pid == 0){
		char *newargv[] = {executable,NULL};
		char *newenv[] = {NULL};
		change_pname(executable);

		raise(SIGSTOP);

		execve(executable,newargv,newenv);
	}

	else{
        char * name = malloc(strlen(executable)+1);
		strcpy(name, executable);
		++queue_low->size;
		int id = find_id_2(queue_low,queue_high);
		printf("Proccess %s, with id:%d and PID:%d has born\n", executable, id, pid);
		enQueue(queue_low,pid,id,name);
	}
}

static int 
sched_set_high(int id){
    if((queue_high->size == 0) && (id == 0)){
        printf("You have to add a task first\n");
        return -1;
    }
    struct QNode *q = queue_low -> front;
    while(q != NULL){
        if(q->id == id)
            break;
        printf("%d\n",q->id);
        q = q->next;
        }
    if(q != NULL){
        ++(queue_high->size);
        printf("i'm here\n");
        enQueue(queue_high,q->key,q->id,q->name);
        printf("i'm here\n");
        delete(queue_low,q->key);
        printf("i'm here\n");
        --(queue_low->size); 
        printf("The task with id:%d has been promoted\n",id);
        if(kill(queue_high->front->key,SIGCONT)){
            perror("SIGCONT error\n");
            exit(1);
        }
        alarm(SCHED_TQ_SEC);
        return id;
        }
    else{
        printf("Bad id\n");
        return -1;
    }
}

static int 
sched_set_low(int id){
    if((queue_low->size)+(queue_high->size)-1 != queue_low->size && (id == 0)){
        printf("You have to add all tasks in the low priority list first\n");
        return -1;
    }
    
    printf("the size is %d\n",queue_low->size);
    struct QNode *q = queue_high -> front;
    while(q != NULL){
        if(q->id == id)
            break;
        printf("%d\n",q->id);
        q = q->next;
        }
    if(q != NULL){
        ++(queue_low->size);
        enQueue(queue_low,q->key,q->id,q->name);
        delete(queue_high,q->key);
        --(queue_high->size); 
        printf("The task with id:%d has been demoted\n",id);
        if(kill(queue_low->front->key,SIGCONT)){
            perror("SIGCONT error\n");
            exit(1);
        }
        alarm(SCHED_TQ_SEC);
        return id;  
        }
    else{
        printf("Bad id\n");
        return -1;
    }
}

/* Process requests by the shell.  */
static int
process_request(struct request_struct *rq)
{
	switch (rq->request_no) {
        case REQ_HIGH_TASK:
            return sched_set_high(rq->task_arg);
            
        case REQ_LOW_TASK:
            return sched_set_low(rq->task_arg);

		case REQ_PRINT_TASKS:
			sched_print_tasks();
			return 0;

		case REQ_KILL_TASK:
			return sched_kill_task_by_id(rq->task_arg);

		case REQ_EXEC_TASK:
			sched_create_task(rq->exec_task_arg);
			return 0;

		default:
			return -ENOSYS;
	}
}

/* 
 * SIGALRM handler
 */
static void
sigalrm_handler(int signum)
{   
    pid_t head;
    if(queue_high->front != NULL){
	    head = queue_high -> front -> key;
    
	if(kill(head,SIGSTOP) < 0){
		perror("error at SIGSTOP");
		exit(1);
	}
    }
    else{
        head = queue_low -> front -> key;
    
	if(kill(head,SIGSTOP) < 0){
		perror("error at SIGSTOP");
		exit(1);
	}
    }
}

/* 
 * SIGCHLD handler
 */
static void
sigchld_handler(int signum)
{   
	pid_t pid;
	int status;

	for(;;){
		pid = waitpid(-1,&status,WUNTRACED|WNOHANG);
	
		if(pid == 0)
			break;
		explain_wait_status(pid,status);
		if(WIFEXITED(status)){
			if(queue_high->size != 0){
                printf("1 %d\n",queue_high->size);
                deQueue(queue_high);
                --(queue_high->size);
                if(queue_high->size == 0){
                    printf("2 %d\n",queue_high->size);
                    printf("The high priority list is empty\n");
                    if(queue_low->size == 0){
                        printf("3 %d\n",queue_low->size);
                        printf("The high and low priority lists are empty\n");
                        printf("Done!\n");
                        exit(10);
                        }
                    else{
                        pid = queue_low -> front -> key;

			            if(kill(pid,SIGCONT) < 0){
				            perror("error at SIGCONT\n");
				            exit(1);
			                }
			            alarm(SCHED_TQ_SEC);            
                        }
                    }
            
                else {
                    pid = queue_high -> front -> key;
            
                    if(kill(pid,SIGCONT) < 0){
				        perror("error at SIGCONT\n");
				        exit(1);
			            }
			        alarm(SCHED_TQ_SEC);
            
                
                }
            }
            else{
                printf("4 %d\n",queue_low->size);
                deQueue(queue_low);
                --(queue_low->size);
                printf("5 %d\n",queue_low->size);
			    if(queue_low->size == 0){
                    printf("6 %d\n",queue_low->size);
                    printf("The low priority list is empty\n");
        	        printf("Done!\n");
    	            exit(10);
	    	        }
    
                else{
                    printf("62 %d\n",queue_low->size);
			        if(queue_low->front ==NULL && queue_low->rear == NULL){
						printf("i'm NULL\n");
					}
					pid = queue_low -> front -> key;
					printf("i'm here\n");
			        if(kill(pid,SIGCONT) < 0){
				        perror("error at SIGCONT\n");
				        exit(1);
			            }
						printf("i'm here\n");
			        alarm(SCHED_TQ_SEC);
                    }
		        }
        }
        if(WIFSIGNALED(status)){
            if(queue_high->size != 0){
                printf("7 %d\n",queue_high->size);
                delete(queue_high,pid);
                --(queue_high->size);
                if(queue_high->size == 0){
                    printf("8 %d\n",queue_high->size);
                    printf("The high priority list is empty\n");
                    if(queue_low->size == 0){
                        printf("9 %d\n",queue_low->size);
                        printf("The high and low priority lists are empty\n");
                        printf("Done!\n");
                        exit(10);
                        }
                    else{
                        pid = queue_low -> front -> key;

			            if(kill(pid,SIGCONT) < 0){
				            perror("error at SIGCONT\n");
				            exit(1);
			                }
			            alarm(SCHED_TQ_SEC);            
                        }
                    }
            
                else {
                    pid = queue_high -> front -> key;
            
                    if(kill(pid,SIGCONT) < 0){
				        perror("error at SIGCONT\n");
				        exit(1);
			            }
			        alarm(SCHED_TQ_SEC);
            
                
                }
            }
            else{
                printf("10 %d\n",queue_low->size);
                delete(queue_low,pid);
                --(queue_low->size);
				sched_print_tasks();
                printf("11 %d\n",queue_low->size);
			    if(queue_low->size == 0){
                    printf("12 %d\n",queue_low->size);
                    printf("The low and high priority lists are empty\n");
        	        printf("Done!\n");
    	            exit(10);
	    	        }
    
                else{
			        pid = queue_low -> front -> key;

			        if(kill(pid,SIGCONT) < 0){
				        perror("error at SIGCONT\n");
				        exit(1);
			            }
			        alarm(SCHED_TQ_SEC);
                    }
		        }
        }

		if(WIFSTOPPED(status)){
			int d;
			char *n;
			if(queue_high->size != 0){
                printf("13 %d\n",queue_high->size);
                pid = queue_high -> front -> key;
			    d = queue_high -> front -> id;
			    n = queue_high -> front -> name;
			    deQueue(queue_high);

			    enQueue(queue_high,pid,d,n);
			    pid = queue_high -> front -> key;

			    if(kill(pid,SIGCONT) < 0){
				    perror("error at SIGCONT\n");
				    exit(1);
			    }
			    alarm(SCHED_TQ_SEC);
		        }
            else{
                printf("15 %d\n",queue_low->size);
			    pid = queue_low -> front -> key;
			    d = queue_low -> front -> id;
			    n = queue_low -> front -> name;
			    deQueue(queue_low);

			    enQueue(queue_low,pid,d,n);
			    pid = queue_low -> front -> key;

			    if(kill(pid,SIGCONT) < 0){
				    perror("error at SIGCONT\n");
				    exit(1);
			        }
			    alarm(SCHED_TQ_SEC);
		        }
	        }
        }
}

/* Disable delivery of SIGALRM and SIGCHLD. */
static void
signals_disable(void)
{
	sigset_t sigset;

	sigemptyset(&sigset);
	sigaddset(&sigset, SIGALRM);
	sigaddset(&sigset, SIGCHLD);
	if (sigprocmask(SIG_BLOCK, &sigset, NULL) < 0) {
		perror("signals_disable: sigprocmask");
		exit(1);
	}
}

/* Enable delivery of SIGALRM and SIGCHLD.  */
static void
signals_enable(void)
{
	sigset_t sigset;

	sigemptyset(&sigset);
	sigaddset(&sigset, SIGALRM);
	sigaddset(&sigset, SIGCHLD);
	if (sigprocmask(SIG_UNBLOCK, &sigset, NULL) < 0) {
		perror("signals_enable: sigprocmask");
		exit(1);
	}
}


/* Install two signal handlers.
 * One for SIGCHLD, one for SIGALRM.
 * Make sure both signals are masked when one of them is running.
 */
static void
install_signal_handlers(void)
{
	sigset_t sigset;
	struct sigaction sa;

	sa.sa_handler = sigchld_handler;
	sa.sa_flags = SA_RESTART;
	sigemptyset(&sigset);
	sigaddset(&sigset, SIGCHLD);
	sigaddset(&sigset, SIGALRM);
	sa.sa_mask = sigset;
	if (sigaction(SIGCHLD, &sa, NULL) < 0) {
		perror("sigaction: sigchld");
		exit(1);
	}

	sa.sa_handler = sigalrm_handler;
	if (sigaction(SIGALRM, &sa, NULL) < 0) {
		perror("sigaction: sigalrm");
		exit(1);
	}

	/*
	 * Ignore SIGPIPE, so that write()s to pipes
	 * with no reader do not result in us being killed,
	 * and write() returns EPIPE instead.
	 */
	if (signal(SIGPIPE, SIG_IGN) < 0) {
		perror("signal: sigpipe");
		exit(1);
	}
}

static void
do_shell(char *executable, int wfd, int rfd)
{
	char arg1[10], arg2[10];
	char *newargv[] = { executable, NULL, NULL, NULL };
	char *newenviron[] = { NULL };

	sprintf(arg1, "%05d", wfd);
	sprintf(arg2, "%05d", rfd);
	newargv[1] = arg1;
	newargv[2] = arg2;

	raise(SIGSTOP);
	execve(executable, newargv, newenviron);

	/* execve() only returns on error */
	perror("scheduler: child: execve");
	exit(1);
}

/* Create a new shell task.
 *
 * The shell gets special treatment:
 * two pipes are created for communication and passed
 * as command-line arguments to the executable.
 */
static void
sched_create_shell(char *executable, int *request_fd, int *return_fd)
{
	pid_t p;
	int pfds_rq[2], pfds_ret[2];

	if (pipe(pfds_rq) < 0 || pipe(pfds_ret) < 0) {
		perror("pipe");
		exit(1);
	}

	p = fork();
	if (p < 0) {
		perror("scheduler: fork");
		exit(1);
	}

	if (p == 0) {
		/* Child */
		change_pname(executable);
		close(pfds_rq[0]);
		close(pfds_ret[1]);
		do_shell(executable, pfds_rq[1], pfds_ret[0]);
		assert(0);
	}
    ++(queue_low->size);
	enQueue(queue_low,p,0,executable);
	/* Parent */
	close(pfds_rq[1]);
	close(pfds_ret[0]);
	*request_fd = pfds_rq[0];
	*return_fd = pfds_ret[1];
}

static void
shell_request_loop(int request_fd, int return_fd)
{
	int ret;
	struct request_struct rq;

	/*
	 * Keep receiving requests from the shell.
	 */
	for (;;) {
		if (read(request_fd, &rq, sizeof(rq)) != sizeof(rq)) {
			perror("scheduler: read from shell");
			fprintf(stderr, "Scheduler: giving up on shell request processing.\n");
			break;
		}

		signals_disable();
		ret = process_request(&rq);
		signals_enable();

		if (write(return_fd, &ret, sizeof(ret)) != sizeof(ret)) {
			perror("scheduler: write to shell");
			fprintf(stderr, "Scheduler: giving up on shell request processing.\n");
			break;
		}
	}
}

int main(int argc, char *argv[])
{
	int nproc = argc;
	/* Two file descriptors for communication with the shell */
	static int request_fd, return_fd;
	queue_low = createQueue();
    queue_high = createQueue();
	/* Create the shell. */
	sched_create_shell(SHELL_EXECUTABLE_NAME, &request_fd, &return_fd);
	/* TODO: add the shell to the scheduler's tasks */
	
	/*
	 * For each of argv[1] to argv[argc - 1],
	 * create a new child process, add it to the process list.
	 */
	pid_t pid;
	for(int i = 1; i < nproc; ++i){
		pid = fork();
		if(pid < 0){
			perror("error at fork\n");
			exit(1);
		}
		if(pid != 0){
            ++(queue_low->size);
			enQueue(queue_low,pid,i,argv[i]);
		
		}
		
		else{
			change_pname(argv[i]);

			char *newargv[] = {argv[i],NULL};
			char *newenv[] = {NULL};

			raise(SIGSTOP);

			execve(argv[i],newargv,newenv);
		}
	}

	/* Wait for all children to raise SIGSTOP before exec()ing. */
	wait_for_ready_children(nproc-1);
	show_pstree(getpid());
	/* Install SIGALRM and SIGCHLD handlers. */
	install_signal_handlers();

	if (nproc-1 == 0) {
		fprintf(stderr, "Scheduler: No tasks. Exiting...\n");
		exit(1);
	}
	pid_t start = queue_low -> front -> key;
	if(kill(start,SIGCONT) < 0){
		perror("error at SIGCONT\n");
		exit(1);
	}

	alarm(SCHED_TQ_SEC);

	shell_request_loop(request_fd, return_fd);

	/* Now that the shell is gone, just loop forever
	 * until we exit from inside a signal handler.
	 */
	
	while (pause())
		;

	/* Unreachable */
	fprintf(stderr, "Internal error: Reached unreachable point\n");
	return 1;
}
