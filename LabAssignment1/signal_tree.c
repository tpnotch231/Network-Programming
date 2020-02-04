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
int points, A, S, N, child1, child2;
typedef struct node{
	int left, right, parent;
	int pid;
} Node;
Node *tree;
struct sigaction Action;
int search(int pid)
{
	for(int i=0;i<N;i++)
	{
		if(tree[i].pid==pid) return i;
		if(tree[i].pid==-1) return i+N;
	}
	return -1;
}
bool isSibling(int origin, int target)
{
	origin=search(origin);
	if(origin==-1||origin>=N)
	{
		printf("Critical error in isSibling.\n");
	}
	return (tree[tree[tree[origin].parent].left].pid==target)&&(tree[tree[tree[origin].parent].right].pid==target);
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
	else if(isSibling(getpid(),siginfo->si_pid))
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
bool isDescendantCore(int origin, int target)
{
	if(origin==-1) return false;
	return (tree[origin].pid==target)||(isDescendantCore(tree[origin].left,target)||isDescendantCore(tree[origin].right,target));
}
bool isDescendant(int origin, int target)
{
	origin=search(origin);
	if(origin==-1||origin>=N)
	{
		printf("Critical error in isDescendant.");
	}
	return isDescendantCore(origin,target);
}
bool isAncestor(int origin, int target)
{
	origin=search(origin);
	if(origin==-1||origin>=N)
	{
		printf("Critical error in isAncestor.");
	}
	while(origin!=-1)
	{
		if(tree[origin].pid==target)
		{
			return true;
		}
		origin=tree[origin].parent;
	}
	return false;
}
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
	int n1, n2, id, parid;	//n1 is the number of children in the left subtree. 
	parid=search(getpid());
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
		id=search(getpid());
		if(id>N)
		{
			id-=N;
			tree[id].pid=getpid();
			tree[id].parent=parid;
			tree[parid].left=id;
		}
		procCreate(n1);
	}
	else
	{
		if(n2>0&&!(--n2,(child2=fork())))
		{
			child1=0;		
			id=search(getpid());
			if(id>N)
			{
				id-=N;
				tree[id].pid=getpid();
				tree[id].parent=parid;
				tree[parid].right=id;
			}
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
	key_t SHMKey=ftok("./signal_tree.c",'A');
	if(SHMKey==-1){
		printf("Unable to access source file to create IPC key.\n");
		return -1;
	}
	int SHMID;
	if(-1==(SHMID=shmget(SHMKey,numOfProcesses*sizeof(Node),IPC_CREAT|IPC_EXCL|0660)))
	{
		perror("Error at shmget");
		return -1;
	}
	if(-1==(tree= (Node*) shmat(SHMID,NULL,0)))
	{
		perror("Error at shmat");
		return -1;
	}
	for(int i=0;i<N;i++)
	{
		tree[i].pid=-1;
		tree[i].left=-1;
		tree[i].right=-1;
		tree[i].parent=-1;
	}
	tree[0].pid=getpid();
	Action.sa_sigaction=*handler;
	Action.sa_flags|=SA_SIGINFO;
	if(-1==sigaction(SIGUSR1,&Action,NULL))
	{
		perror("Error in assigning the signal handler");
	}
	printf("Topmost parent: %d\n", tree[0].pid);
	//int child1=0, child2=0;
	//procCreate(numOfProcesses-1,&child1,&child2);
	procCreate(numOfProcesses-1);
	for(int i=getppid()-numOfProcesses;i<=getpid()+numOfProcesses;i++)
	{
		//printf("%d\n",getppid());
		if((i>=tree[0].pid)&&(-1==kill(i,SIGUSR1))&&SIGNALERRORMESSAGES)
		{
			char message[50];
			sprintf(message,"%d",i);
			strcat(message,": sending signal failed");
			perror(message);
			printf("Change the value of the SIGNALERRORMESSAGES macro in the program.\n");
		}
	}
	if(-1==shmdt(tree))
	{
		perror("Error at shmdt");
		return -1;
	}
	/*struct shmid_ds shmid_ds;
	if((rootNode[0].pid==getpid())&&(-1==shmctl(SHMID,IPC_RMID,&shmid_ds)))
	{
		perror("Error while destroying the shared memory");
		return -1;
	}*/
	printf("Process %d exiting with %d points.\n", getpid(),points);
	return 0;
}
