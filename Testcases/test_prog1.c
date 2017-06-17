#include <lib_pext.h>

//==============================================================================
// Select menu return number 
//==============================================================================

int selectMenu(void)
{
	int n;
	printf("\n");
	puts("[  1 ] TopicCreate");
	puts("[  2 ] TopicLookup");
	puts("[  3 ] TopicPublisher : Declare as a Publisher");
	puts("[  4 ] TopicSubscriber: Declare as a Subscriber");
	puts("[  5 ] Publish");
	puts("[  6 ] Retrieve");
	puts("[  7 ] Available Publisher(s)");
	puts("[  8 ] Available Subscriber(s)");	
	puts("[  9 ] Flush Message Region");
	puts("[ 10 ] Exit");
	printf("\n");
    printf("Please choose the function that you want to use[Exit/1-10] ? ");
    scanf("%d",&n);
	return n;
}

//
// ==================================== START =====================================
//

int main(void)
{
	int selection;
	int judge;
	char choice,NewTopic[100],SeleteTopic[100];
	char SelectTopicS[100],Msg[100];
    

    while (kTRUE)
    {
    	//
    	// header information 
    	//
    	printHeader();
    
		selection = selectMenu();

		switch (selection)
		{
			case kTopicCreate:
				printf("Please type in topic name: ");
				scanf("%s",NewTopic);
				if(Topic_create_SC(NewTopic)==kRETSUCCESS)
					printf("\n%s Topic is added......\n",NewTopic);
				else
					printf("\nThis Topic Is Already Exist ......\n");
				break;

			case kTopicLookup:
				Topic_Display_Topic();
				break;

			case kTopicPublisher:
				Topic_Display_Topic();
				printf("Enter Topic You Want To Publish: ");
				scanf("%s",SeleteTopic);

				judge = Topic_publisher_SC(SeleteTopic);

				if(judge==kRETSUCCESS)
				{
					printf("\nRegistered As New Publisher......\n");
				}
				else
				{
					printf("\nTopic named ' %s ' Does not exist.......\n",SeleteTopic);
				}
				break;

			case kTopicSubscriber:
				Topic_Display_Topic();
				printf("\nEnter Topic You Want To Subscribe: ");
				scanf("%s",SelectTopicS);
				judge=Topic_subsriber_SC(SelectTopicS);
				if(judge==kRETSUCCESS)
					printf("\nRegistered As New Subscriber......\n");
				else
					printf("\nTopic named ' %s ' Does not exist......\n",SelectTopicS);
				break;

			case kPublish:
				printf("\nplz Enter message: ");
				scanf(" %s",Msg);
				Topic_publish_msg_SC(Msg);
				break;

			case kRetrieve:
				if(Topic_Receive_msg_SC()!=kRETSUCCESS)
				{
					puts("\nNo Message Available.");
				}
				break;

			case kAvailablePublisher:
				Topic_Display_publisher();
				break;

			case kAvailableSubscriber:
				Topic_Display_subscriber();
				break;

			case kFlushRegion:
				Topic_init();
				puts("Flush Buffer Region");
				break;

			case kExit:
				printf("Number is %d\n", selection);
				exit(0);

			default:
				puts("\nInvalid input, please type in number [1-10].");
		}
    }

    return 0;
}
