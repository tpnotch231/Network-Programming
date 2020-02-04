#include<stdio.h>
#include<stdlib.h>
void procCreate(int n)
{
	
	if(n>0&&!fork())
	{
		procCreate((n-2)/2);
	}
	else
	{
		if(n>0&&!fork())
		{
			procCreate((n-2)/2);
		}
		else
		{
		}
	}
	return;
}
int main(int argc, char *argv[])
{
	procCreate(atoi(argv[1]));
	sleep(100);
}
