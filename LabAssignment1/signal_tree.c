#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<math.h>
void swap(int *n1,int *n2)
{
	int temp=*n1;
	*n1=*n2;
	*n2=temp;
}
void procCreate(int n,int *child1,int *child2)
{
	int n1, n2;
	n2=log2(n);
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
		procCreate(n1,child1,child2);
	}
	else
	{
		if(n2>0&&!(--n2,(*child2=fork())))
		{
			*child1=0;
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
	printf("Topmost parent: %d\n", getpid());
	int child1=0, child2=0;
	procCreate(atoi(argv[1])-1,&child1,&child2);
	//sleep(5);
	if(child1||child2)
	{
		printf("%d -> %d\n",getpid(),child1);
		printf("%d -> %d\n",getpid(),child2);
	}
}
