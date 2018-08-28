/*
 * Demonstrate the client code running on the Boomgate     
 * Controller which is one of the important  module of the  
 * Traffic System communicating with the controller room's 
 * System controller,  I1(Traffic light No. 1 controller) and I2 
 * (Traffic light No. 2 controller) and broadcasting the message      
 *  all over the system at the same time with real time update.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/netmgr.h>
#include <sys/neutrino.h>
#include <pthread.h>
#include <sys/dispatch.h>
#include <fcntl.h>
#include <share.h>
#define Data_SIZE 3
#define BUF_SIZE 100
#include <errno.h>


char *progname = "timer_per1.c";
enum states {
State0,
State1,
State2,
State3,
State4,
};


char buf[Data_SIZE];

typedef struct
{


	 enum states currentstate;
	 pthread_mutex_t mutex;  // needs to be set to PTHREAD_MUTEX_INITIALIZER;

}app_data;


#define MY_PULSE_CODE   _PULSE_CODE_MINAVAIL

typedef union
{
	struct _pulse   pulse;
    // your other message structures would go here too
} my_message_t;


typedef struct               //Native Client
{
	struct _pulse hdr; // Our real data comes after this header
	int ClientID; // our data (unique id from client)
    char data[Data_SIZE];

   // our data
} my_data;


typedef struct               //Native Client
{
	struct _pulse hdr1; // Our real data comes after this header
	int ClientID1; // our data (unique id from client)
    char data1[Data_SIZE];

   // our data
} my_data1;
typedef struct              //Native Client
{
	struct _pulse hdr; // Our real data comes after this header
    char buf1[BUF_SIZE];// Message we send back to clients to tell them the messages was processed correctly.
} my_reply;

typedef struct              //Native Client
{
	struct _pulse hdr1; // Our real data comes after this header
    char Buf1[BUF_SIZE];// Message we send back to clients to tell them the messages was processed correctly.
} my_reply1;


#define QNET_ATTACH_POINT  "/net/RMIT_Beagle03_v4p2.net.intra/dev/name/local/Central Controller"
#define QNET_ATTACH_POINT1  "/net/RMIT_Beagle02_v4p2.net.intra/dev/name/local/I1"
//#define QNET_ATTACH_POINT  "/net/RMIT_Beagle03_v4p2.net.intra/dev/name/local/I2"

void *keyboardX1X2(void *Data);
void *SingleStep_TrafficLight_SM(void*Data);
void *timer(void*Data);
int  client(char*sname,void*Data);
int  *client1(char*sname1,void *Data);
app_data data = {0,PTHREAD_MUTEX_INITIALIZER};

int main(int argc, char *argv[],void *Data)
{


	    app_data *tdm = (app_data*) Data;
		int ret = 0;
        pthread_t  th1, th2,th3,th4;
		void *retval;
        char sname1;


		pthread_create(&th1, NULL, keyboardX1X2, &data);
		pthread_create(&th2, NULL, SingleStep_TrafficLight_SM, &data);
		pthread_create(&th3, NULL, timer, &data);
		pthread_create(&th4, NULL, client1,(&sname1,&data));


		printf("This is A Client running\n");
		ret = client(QNET_ATTACH_POINT1,&data);


	 	    pthread_join (th1, &retval);
	    	pthread_join (th2, &retval);
	    	pthread_join (th3, &retval);




	    	printf("Main (client) Terminated....\n");
	    	return ret;

		   // ret = client(LOCAL_ATTACH_POINT);

			printf("Main (client) Terminated....\n");



		printf("All good. Main Terminated...\n\n");

}

// Here we implement a simple state machine

void *SingleStep_TrafficLight_SM(void*Data)
{
	 app_data *tds = (app_data*) Data;



		switch (tds->currentstate)
		{

		case State0:
		printf("Boom-Gate Open(%d)\n",tds->currentstate);
		pthread_mutex_lock(&tds->mutex);
		if(buf[0]=='x'&&buf[1]=='c')
		{
			tds->currentstate = State1;

		}
		else if(buf[1]=='f')
		{
		printf("Fault !!\n");
		tds->currentstate = State3;
		}
		else
		tds->currentstate = State0;
		pthread_mutex_unlock(&tds->mutex);
		break;
		case State1:
			printf("Boom-Gate Open(Train approaching)(%d)\n",tds->currentstate);
			tds->currentstate = State2;
		break;
		case State2:
			printf("Boom-Gate Closing(%d)\n",tds->currentstate);
			tds->currentstate = State3;
		break ;
		case State3:
			printf("Boom-Gate Closed(%d)\n",tds->currentstate);
			pthread_mutex_lock(&tds->mutex);
			if(buf[0]=='y'&&buf[1]=='c')
			{
			tds->currentstate = State4;
			}
			else if(buf[1]=='f')
		    {
			printf("Fault !!\n");
			tds->currentstate = State3;
			}
			else
			tds->currentstate = State3;
			pthread_mutex_unlock(&tds->mutex);
		break;
		case State4:
			printf("Boom-Gate Opening(%d)\n",tds->currentstate);
			tds->currentstate = State0;
			break;

}


  return 0;

}
/*** Client code ***/


int *client1(char *sname1,void *Data)
{
    my_data1 msg1;
    my_reply1 reply1;
    app_data *tdg = (app_data*) Data;

    msg1.ClientID1 = 600; // unique number for this client (optional)

    int server1_coid;


    printf("  ---> Trying to connect to server named: %s\n",sname1 );
    if ((server1_coid = name_open(sname1, 0)) == -1)
    {
        printf("\n    ERROR, could not connect to server!\n\n");
        pthread_exit( EXIT_FAILURE);

    }

    printf("Connection established to: %s\n", sname1);


    // We would have pre-defined data to stuff here
    msg1.hdr1.type = 0x00;
    msg1.hdr1.subtype = 0x00;

    // Do whatever work you wanted with server connection
    while (1) // send data packets
    {
    	// set up data packet
    	pthread_mutex_lock(&tdg->mutex);
    	msg1.data1[0]= buf[0];
    	msg1.data1[1]= buf[1];
    	pthread_mutex_unlock(&tdg->mutex);

    	// the data we are sending is in msg.data
        printf("Client (ID:%d), sending data packet with the character value: %c\n", msg1.ClientID1, msg1.data1[0]);
        fflush(stdout);

        if (MsgSend(server1_coid, &msg1, sizeof(msg1), &reply1, sizeof(reply1)) == -1)
        {
            printf(" Error data '%s' NOT sent to server\n", msg1.data1);
            	// maybe we did not get a reply from the server
            break;
        }
        else
        { // now process the reply
            printf("   -->Reply is: '%s'\n", reply1.Buf1);
        }

        //sleep(5);	// wait a few seconds before sending the next data packet
    }

    // Close the connection
    printf("\n Sending message to server to tell it to close the connection\n");
    name_close(server1_coid);

    pthread_exit( EXIT_SUCCESS);

}

int client(char*sname,void *Data)
{
    my_data msg;
    my_reply reply;
    app_data *tdc = (app_data*) Data;

    msg.ClientID = 600; // unique number for this client (optional)

    int server_coid;


    printf("  ---> Trying to connect to server named: %s\n",sname );
    if ((server_coid = name_open(sname, 0)) == -1)
    {
        printf("\n    ERROR, could not connect to server!\n\n");
        pthread_exit( EXIT_FAILURE);

    }

    printf("Connection established to: %s\n", sname);

    // We would have pre-defined data to stuff here
    msg.hdr.type = 0x00;
    msg.hdr.subtype = 0x00;

    // Do whatever work you wanted with server connection
    while (1) // send data packets
    {
    	// set up data packet
    	pthread_mutex_lock(&tdc->mutex);
    	msg.data[0]= buf[0];
    	msg.data[1]= buf[1];
    	pthread_mutex_unlock(&tdc->mutex);

    	// the data we are sending is in msg.data
        printf("Client (ID:%d), sending data packet with the character value: %c\n", msg.ClientID, msg.data[0]);
        fflush(stdout);

        if (MsgSend(server_coid, &msg, sizeof(msg), &reply, sizeof(reply)) == -1)
        {
            printf(" Error data '%s' NOT sent to server\n", msg.data);
            	// maybe we did not get a reply from the server
            break;
        }
        else
        { // now process the reply
            printf("   -->Reply is: '%s'\n", reply.buf1);
        }

        //sleep(5);	// wait a few seconds before sending the next data packet
    }

    // Close the connection
    printf("\n Sending message to server to tell it to close the connection\n");
    name_close(server_coid);

    pthread_exit( EXIT_SUCCESS);

}



void *keyboardX1X2(void *Data)// Sensor Thread
{
    printf("->keyboardX1 thread started...\n");


    // initialize the fake data:
    app_data *tdp = (app_data*) Data;




    while(1)
	{


        // get data from hardware
        // we'll simulate this with a just incrementing the data and use sleep(1) to simulate time lag
        printf("Is the train approaching\n");
        printf("Type integer x for approaching and y  for departing\n");
        pthread_mutex_lock(&tdp->mutex);
        gets(buf);
        pthread_mutex_unlock(&tdp->mutex);

        //sleep(1);
		// test the condition and wait until it is true

		// now change the condition and signal that it has changed
	}

   return 0;
}





void *timer(void*Data)//Timer Thread
{

	printf("##########################################################\n");
		printf("#                                                        #\n");
		printf("#              EXAMPLE Code                              #\n");
		printf("#                                                        #\n");
		printf("#         QNX implementation of POSIX timers             #\n");
		printf("#                                                        #\n");
		printf("# Process runs a simple X1 state machine {states: 0,1,2,3,4} #\n");
		printf("#                                                        #\n");
		printf("##########################################################\n");

		//enum states CurrentState = State0;

		struct sigevent         event;
			struct itimerspec       itime,itime1;
			timer_t                 timer_id;
			int                     chid;
			int                     rcvid;
			my_message_t            msg;



			chid = ChannelCreate(0); // Create a communications channel

	        app_data *tdt = (app_data*) Data;


	 event.sigev_notify = SIGEV_PULSE;
	 // create a connection back to ourselves for the timer to send the pulse on
	 		event.sigev_coid = ConnectAttach(ND_LOCAL_NODE, 0, chid, _NTO_SIDE_CHANNEL, 0);
	 		if (event.sigev_coid == -1)
	 		{
	 		   printf(stderr, "%s:  couldn't ConnectAttach to self!\n", progname);
	 		   perror(NULL);
	 		   exit(EXIT_FAILURE);
	 		}
	 		event.sigev_priority = getprio(0);
	 		event.sigev_code = MY_PULSE_CODE;

	 		// create the timer, binding it to the event
	 		if (timer_create(CLOCK_REALTIME, &event, &timer_id) == -1)
	 		{
	 		   printf (stderr, "%s:  couldn't create a timer, errno %d\n", progname, errno);
	 		   perror (NULL);
	 		   exit (EXIT_FAILURE);
	 		}

	 		// setup the timer (1s initial delay value, 1s reload interval)
	 		itime.it_value.tv_sec = 4;			  // 1 second
	 		itime.it_value.tv_nsec = 0;    // 0 nsecs = 0 secs
	 		itime.it_interval.tv_sec =4;          // 1 second
	 		itime.it_interval.tv_nsec =0; // 0 nsecs = 0 secs

	 		// and start the timer!
	 		timer_settime(timer_id, 0, &itime, NULL);

	 		// setup the timer1 (2s initial delay value, 2s reload interval)
	 		itime1.it_value.tv_sec = 3;			  // 2 second
	 		itime1.it_value.tv_nsec = 0;    // 0 nsecs = 0secs
	 		itime1.it_interval.tv_sec = 3;          // 2 second
	 		itime1.it_interval.tv_nsec = 0; // 0 nsecs = 0 secs

	 		// and start the timer1!
	 		timer_settime(timer_id, 0, &itime1, NULL);

	 		/*
	 		* As of the timer_settime(), we will receive our pulse
	 		* in 1.5 seconds (the itime.it_value) and every 1.5
	 		* seconds thereafter (the itime.it_interval)
	 		*/



	 				while (1)
	 				{
	 					// wait for message/pulse
	 				   rcvid = MsgReceive(chid, &msg, sizeof(msg), NULL);

	 				   // determine who the message came from
	 				   if (rcvid == 0) // this process
	 				   {
	 					   // received a pulse, now check "code" field...
	 					   if (msg.pulse.code == MY_PULSE_CODE) // we got a pulse
	 						{


	 						  if(tdt->currentstate==0||tdt->currentstate==2||tdt->currentstate==3||tdt->currentstate==4)
	 						  timer_settime(timer_id, 0, &itime, NULL);
	 						  else
	 						  timer_settime(timer_id, 0, &itime1, NULL);
	 						 // SingleStep_TrafficLight_SM(&data);

	 						  SingleStep_TrafficLight_SM(&data);
	 						  fflush(stdout); // make sure we print to the screen
	 						}
	 						// else other pulses ...
	 				   }
	 				   // else other messages ...
	 				}

    return 0;
}



