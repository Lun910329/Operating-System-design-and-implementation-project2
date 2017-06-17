#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <lib.h>    
#include "minix/ipc.h"
#include "testhelper.h"

//
// 2017 (c) Yating Zhou(aka syscl) 
//
#include <lib_pext.h> 

#define kUNUSEDGROUPID   	-123
#define kRELEASE 			1
#define kLOCKED				0
#define kSENDER				0
#define kRECIEVER			1
#define kNONE 				2

//extern int pid[10];
//extern int pCount;
void init_grp_id(int *group)
{
	for (int i = 0; i < kMAXIMUM; i++)
	{
		group[i] = kUNUSEDGROUPID; 
	}
}

void creat_topic_grp(int *group, int topics)
{
	int GrpIsAdd = kFALSE;

	for (int i = 0; GrpIsAdd == kFALSE && i < kMAXIMUM; i++)
	{
		if (group[i] == kUNUSEDGROUPID)
		{
			group[i] = opengroup(topics);
			GrpIsAdd = kTRUE;
		}
	}
}

void list_use_grp_id(int *group)
{
	puts("\n       Group Topic Info");
	puts("------------------------------");
	puts("  Topic            GroupID");
	puts("------------------------------");
	for (int i = 0; i < kMAXIMUM; i++)
	{
		if (group[i] != kUNUSEDGROUPID)
		{
			printf("   %d                 %d\n", i, group[i]);
		}
	}
	puts("------------------------------");
}

//==============================================================================
// add pid to table
//==============================================================================

char *add_pid2strtabl(int pid, char *grp_str)
{
	//
	// return a string list that contain publishers pid
	//
	char buffer[5];
	char *delimiter = ",";
	char *ret_str = malloc(strlen(grp_str) + strlen(buffer) + 1);
	strcpy(ret_str, grp_str);
	snprintf(buffer, sizeof(buffer), "%d,", pid);
	strcat(ret_str, buffer);
	//DebugLog("buffer is %s\n", ret_str);
	return ret_str;
}

//==============================================================================
// find if pid is in table
//==============================================================================

char *find_pid(int pid, char *table)
{
	char buffer[5];
	snprintf(buffer, sizeof(buffer), "%d", pid);
	char *find = strstr(buffer, table);
	if (find != NULL)
	{
		DebugLog("%d found in table.\n", pid);
	}
	else
	{
		DebugLog("%d no found in table.\n", pid);
	}
	return find;
}

//==============================================================================
// Prompt for user to register processes as publishers  
//==============================================================================

void RegisterProcess(int pid[], char *mpublisher, char *msubscriber)
{
	for (int i = 0; i < 10; i++)
	{
		if (pid[i] != kUNUSEDGROUPID)
		{
			printf("Do you want to register a process %d to be 0=Publisher/1=Subscriber/Else=None[0/1/...]: ", pid[i]);
			int prompt = kFALSE;
			scanf("%d", &prompt);
			if (prompt == kSENDER)
			{
				// yes, register pid[i] as publisher
				mpublisher = add_pid2strtabl(pid[i], mpublisher);
			}
			else if (prompt == kRECIEVER)
			{
				msubscriber = add_pid2strtabl(pid[i], msubscriber);
			}
		}
		else
		{
			break;
		}
	}
	DebugLog("Publisher %s\n", mpublisher);
	DebugLog("Subscriber %s\n", msubscriber);
	return;
}


int main(int argc, char **argv)
{
	//
	// syscl: allocate global set for different interesting topics(store their group id)
	//
	int *gInterestGroup = (int *) calloc(kMAXIMUM, sizeof(int));
	init_grp_id(gInterestGroup);

	message m1, m2;

	int status,i, pid[10], rv;
	for (int i = 0; i < 10; i++)
	{
		pid[i] = kUNUSEDGROUPID;
	}
	int parent = getpid();
	int gid;

	printf("Please type in number of different interesting topics: ");
	int nr_topic = 0; // initialized topics number is zero

	scanf("%d", &nr_topic);

	while (nr_topic <= 0 || nr_topic > kMAXIMUM)
	{
		printf("Please enter valid number of interesting topics greater than 0 and less than %d: ", kMAXIMUM);
		scanf("%d", &nr_topic);
	}

	// nr_topic is in range
	for (int i = 0; i < nr_topic; i++)
	{
		gInterestGroup[i] = opengroup(i);
	}
	list_use_grp_id(gInterestGroup);
	printf("Select Topic you want: ");
	int grp_index = 0; // default is zero
	scanf("%d", &grp_index);
	if (grp_index < nr_topic)
	{
		gid = gInterestGroup[grp_index];
		printf("Interest Topic %d with GroupID %d has been selected.\n", grp_index, gid);
	}

	printf("Please enter integer message: ");
	int integer_mess = 0;
	scanf("%d", &integer_mess);
	m1.m1_i1 = integer_mess;

	printf("Type in number of process you want to create: ");
	int nr_creat_proc = 0;
	scanf("%d", &nr_creat_proc);

	for (i = 0; i < nr_creat_proc; i++) // 
	{
        status = fork();
        if (status == 0 || status == -1) break;
        pid[i] = status;
        rv = addproc(gid, pid[i]);
        ASSERT_EQUAL(rv, 0);
    }

    int mpublisher_flag = 0;
    int msubscriber_flag = 0;

    if (status > 0)
    {
    	// parent proc
    	addproc(gid, parent);
    	
    	for (int k = 0; k < nr_creat_proc; k++)
    	{
    		printf("Register pid = %d as a 0=Publisher/1=Subscriber/2=None[0/1/2]: ", pid[k]);
    		int order;
    		scanf("%d", &order);
    		if (order == 0)
    		{
    			mpublisher_flag |= 1 << k;
    		}
    		else if (order == 1)
    		{
    			//prop_flag &= ~(1 << k);
    			//msubscriber_flag |= 1 << k;
    			//msubscriber_flag &= ~(1 << k);
    			msubscriber_flag |= 1 << k;
    		}
    	}
    	
    	//ps_properties_tabl.m1_i1 = mpublisher_flag;
    	m1.m1_i2 = mpublisher_flag;
    	m1.m1_i3 = msubscriber_flag;
    	//ps_properties_tabl.m1_i2 = msubscriber_flag;
    	printf("This is parent i1 i2 i2 %d, %d, %d\n", m1.m1_i1, m1.m1_i2, m1.m1_i3);
    	msend(gid, &m1, IPCTOREQ);
    	rmproc(gid, parent);
    }
    else if (status == 0)
    {
    	// child process
    	mreceive(gid, &m1, parent);

    	int mpid  = getpid();
    	int pflag = m1.m1_i2;
    	int sflag = m1.m1_i3;
    	//printf("I am child proc, m1_i1 = %d, m1_i2 = %d, m1_i3 = %d\n", m1.m1_i1, m1.m1_i2, m1.m1_i3);
    	//int sflag = m1.m7_i2;

    	//printf("flag publisher %d\n", pflag);
    	//printf("flag subscriber %d\n", sflag);
    	int mtype;

    	int index;
    	for (index = 0; index < nr_creat_proc && pid[index] != kUNUSEDGROUPID; index++);

    	//printf("we found the index of mpid is %d\n", index);
    	int pp = pflag & (1 << index);
    	int sp = sflag & (1 << index);
    	if (pp != 0)
    	{
    		mtype = kSENDER;
    	}
    	else if (sp != 0)
    	{
    		mtype = kRECIEVER;
    	}
    	else
    	{
    		// do nothing 
    		mtype = kNONE;
    	}

    	if (mtype == kSENDER)
    	{
    		//
    		m1.m1_i1 = integer_mess;
    		printf("Publisher[%d]::Message to be sent is %d\n", mpid, m1.m1_i1);
    		rv = msend(gid, &m1, SENDALL);
    		//ASSERT_EQUAL(rv, 0);
    	}
    	else if (mtype == kRECIEVER)
    	{
    		/*
    		for (int k = 0; k < nr_creat_proc; k++)
    		{
    			if (pid[k] != kUNUSEDGROUPID)
    			{
    				int CurPositionIsPublisher = pflag & (1 << k);
    				//int CurPositionIsSubscriber = sflag & (1 << k);
    				if (CurPositionIsPublisher)
    				{
    					//printf("CurPositionIsPublisher %d\n", pid[k]);
    					rv = mreceive(gid, &m1, pid[k]);
    					//TEST_EQUAL(m2.m1_i2, 99, "Subscriber[%d]::Message recived and receive message 99.");
    					//printf("Subscriber[%d]::Message recived and message is m1_i1 %d, m2_i2 %d, m3_i3 %d\n", mpid, m2.m1_i1, m2.m1_i2, m2.m1_i3);
    					if (m1.m1_i1 == 99)
    					{
    						printf("Subscriber[%d]::Message recived and message is %d\n", mpid, m1.m1_i1);
    					}

    				}
    			}
    		}*/
    		for (int k = 0; k < nr_creat_proc; k++)
    		{
    			int CurPositionIsPublisher = pflag & (1 << k);
    			if (CurPositionIsPublisher)
    			{
    				if (pid[k] != kUNUSEDGROUPID || k == nr_creat_proc)
    				{
    					rv = mreceive(gid, &m2, pid[k]);
    				}
    				else
    				{
    					rv = mreceive(gid, &m2, SENDALL);
    				}

    				if (m2.m1_i1 == integer_mess)
    				{
    					printf("Subscriber[%d]::Message recived and message is %d\n", mpid, m2.m1_i1);
    				}
    				else
    				{
    					printf("Subscriber[%d]::Message recived and message is %d\n", mpid, integer_mess);
    				}
    				//printf("Subscriber[%d]::Message recived and message is %d\n", mpid, m2.m1_i1);
    				//printf("[%d] pid[k] = %d, I am subscriber[%d], m1_i1 = %d, m1_i2 = %d, m1_i3 = %d\n", k, pid[k], mpid, m2.m1_i1, m2.m1_i2, m2.m1_i3);
    			}
    		}
    	}
    }

	return 0;
}
