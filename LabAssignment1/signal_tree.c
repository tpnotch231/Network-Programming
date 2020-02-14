#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<math.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/types.h>
#include<signal.h>
#include<stdbool.h>
#include<string.h>
#define SIGNALERRORMESSAGES 0
//This is a node used to create a process tree.
int points, A, S, N, child1, child2,sib;
typedef struct node{
	int left, right, parent;
	int pid;
} Node;
Node *tree;
struct sigaction Action, SiblingAct;
void siblingHandler(int sig, siginfo_t *siginfo, void *context)
{
	sib=siginfo->si_pid;
}
void handler(int sig, siginfo_t *siginfo, void *context)
{
	if(siginfo->si_pid==getppid())
	{
		points+=A;
	}
	else if((siginfo->si_pid==child1) || (siginfo->si_pid==child2))
	{
		points-=S;
	}
	else if(siginfo->si_pid==sib)
	{
		points-=S/2;
	}
	if(points<=0)
	{
		char arr[50]="Process ";
		int i=0, j=8;
		int pid=getpid();
		char num[7];
		while(pid>0)
		{
			num[i++]=pid%10;
			pid/=10;
		}
		i--;
		while(i>=0)
			arr[j++]=num[i--]+'0';
		i++;
		char *ptr=" exited with 0 or fewer points.\n";
		while(ptr[i]!='\n')
		{
			arr[j++]=ptr[i++];
		}
		arr[j]='\n';
		write(STDOUT_FILENO,arr,sizeof(arr));
		exit(0);
	}
}
//The following function searches through the array of nodes and finds the node with the given pid.
void swap(int *n1,int *n2)	//Helper function to swap integers.
{
	int temp=*n1;
	*n1=*n2;
	*n2=temp;
}
/* The following function creates n processes. The variables child1 and 
 * child2 store the PIDs of the immediate children of the calling process.
 */ 
void procCreate(int n)
{
	int n1, n2;	//n1 is the number of children in the left subtree. 
	n2=log2(n);	//n2 is the number of children in the right subtree.
	n2=pow(2,n2-1)-1;
	n1=n-n2;
	if(n1>=2*n2+1)
	{
		n1-=n2+1;
		n2+=n2+1;
		if(n1<n2)
		swap(&n1,&n2);
	}
	if(n1>0&&!(--n1,(child1=fork())))
	{
		procCreate(n1);
	}
	else
	{
		if(n2>0&&!(--n2,(child2=fork())))
		{
			sib=child1;
			kill(child1,SIGUSR2);
			child1=0;		
			procCreate(n2);
		}
		else
		{
			wait(1);
		}
	}
	return;
}
int main(int argc, char *argv[])
{
	if(argc!=4){
		printf("Not enough command line arguments.\n");
		return -1;
	}
	
	int numOfProcesses=atoi(argv[1]);
	points=numOfProcesses;
	N=numOfProcesses;
	A=atoi(argv[2]);
	S=atoi(argv[3]);
	sib=-1;
	child1=0;
       	child2=0;
	int rootNode=getpid();
	printf("Topmost node: %d\n",rootNode);
	
	Action.sa_sigaction=*handler;
	Action.sa_flags|=SA_SIGINFO;
	SiblingAct.sa_sigaction=*siblingHandler;
	SiblingAct.sa_flags|=SA_SIGINFO;
	if(-1==sigaction(SIGUSR1,&Action,NULL))
	{
		perror("Error in assigning the signal handler");
	}
	if(-1==sigaction(SIGUSR2,&SiblingAct,NULL))
	{
		perror("Error in assigning the signal handler");
	}

	procCreate(numOfProcesses-1);
	/*printf("%d -> %d\n",getpid(),child1);
	printf("%d -> %d\n",getpid(),child2);
	printf("%d -> %d\n",getpid(),sib);*/
	for(int i=getppid()-numOfProcesses;i<=getpid()+numOfProcesses;i++)
	{
		if((i>=rootNode)&&(-1==kill(i,SIGUSR1))&&SIGNALERRORMESSAGES)
		{
			char message[50];
			sprintf(message,"%d",i);
			strcat(message,": sending signal failed");
			perror(message);
			printf("Change the value of the SIGNALERRORMESSAGES macro in the program to suppress these messages.\n");
		}
	}
	printf("Process %d exiting with %d points.\n", getpid(),points);
	return 0;
}
