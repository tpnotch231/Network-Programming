#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<math.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/types.h>
//This is a node used to create a process tree.
typedef struct node{
	int left, right, parent;
	int pid;
} Node;
//The following function searches through the array of nodes and finds the node with the given pid.
int search(Node *tree, int pid, int N)
{
	for(int i=0;i<N;i++)
	{
		if(tree[i].pid==pid) return i;
		if(tree[i].pid==-1) return i+N;
	}
	return -1;
}
bool isDescendant(Node *tree, int N,int origin, int target)
{
	origin=search(tree,origin,N);
	if(origin==-1||origin>=N)
	{
		printf("Critical error in isDescendant.");
	}
	return isDescendant(tree,N,origin,target);
}
bool isDescendantCore(Node *tree, int N, int origin, int target)
{
	if(origin==-1) return false;
	return (isDescendantCore(tree,N,tree[origin].left,target)||(tree[origin].pid==target)||isDescendantCore(tree,N,tree[origin].right,target));
}
bool isAncestor(Node *tree, int N, int origin, int target)
{
	origin=search(tree,origin,N);
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
void procCreate(int n,int *child1,int *child2, Node *tree, int N)
{
	int n1, n2, id, parid;	//n1 is the number of children in the left subtree. 
	parid=search(tree, getpid(), N);
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
	if(n1>0&&!(--n1,(*child1=fork())))
	{
		id=search(tree,getpid(),N);
		if(id>N)
		{
			id-=N;
			tree[id].pid=getpid();
			tree[id].parent=parid;
			tree[parid].left=id;
		}
		procCreate(n1,child1,child2);
	}
	else
	{
		if(n2>0&&!(--n2,(*child2=fork())))
		{
			*child1=0;		
			id=search(tree,getpid(),N);
			if(id>N)
			{
				id-=N;
				tree[id].pid=getpid();
				tree[id].parent=parid;
				tree[parid].right=id;
			}
			procCreate(n2,child1,child2);
		}
		else
		{
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
	Node *tree;
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
	printf("Topmost parent: %d\n", tree[0].pid);
	int child1=0, child2=0;
	procCreate(numOfProcesses-1,&child1,&child2);
	
	
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
	return 0;
}
