/*
Client Program for communication with a server using socket programming
Adapted from Binary Tides: https://www.binarytides.com/server-client-example-c-sockets-linux/
@author Gurbani Baweja
*/
#include <stdio.h>	
#include <string.h>	
#include <sys/socket.h>	
#include <arpa/inet.h>	
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fstream>
#include <iostream>
#include "tands.h"

using namespace std;

int main(int argc , char *argv[])
{
	int sock, client_pid;
	struct sockaddr_in server;
	char message[1000] , server_reply[2000];
	ofstream outputFile;
	
	// Port number provided
	int port_num = atoi(argv[1]);

	int sent_trans = 0;   //Tracker for total transactions sent to the server
    
	//Get Hostname
	//Reference : https://www.geeksforgeeks.org/c-program-display-hostname-ip-address/
	char hostbuffer[256];
	char *IPbuffer;
	struct hostent *host_entry;
	int hostname;
	// To retrieve hostname
	hostname = gethostname(hostbuffer, sizeof(hostbuffer));
    if (hostname == -1)
	{
		perror("gethostname");
		exit(1);
	}
	
	//To retrive client pid
	client_pid = getpid();
	

	//Client details message for server
	string clientId, hname, clientDetail;
    char cDetail_message[1000];
	hname = std::string(hostbuffer);
	clientId = std::to_string(client_pid);
	clientDetail = hname + "." + clientId;    
	strcpy(cDetail_message, clientDetail.c_str());
	string fname = clientDetail + ".log";    // Name of the output file of the client
    
	//Writing to the output file
	outputFile.open(fname); 
	outputFile<<"In           "<<clientDetail<<"  Output\n";
    outputFile<<"             "<<"Using port "<<port_num<<"\n";
    outputFile<<"             "<<"Using Server Address "<<argv[2]<<"\n";
    outputFile<<"             "<<"Host "<<clientDetail<<"\n";
    outputFile.close();


	//Create socket
	sock = socket(AF_INET , SOCK_STREAM , 0);
	if (sock == -1)
	{
		printf("Could not create socket");
	}
	
	server.sin_addr.s_addr = inet_addr(argv[2]);
	server.sin_family = AF_INET;
	server.sin_port = htons( port_num );

	//Connect to remote server
	if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
	{
		perror("connect failed. Error");
		return 1;
	}
	
	//Send client information to the server
	if( send(sock , cDetail_message , strlen(cDetail_message) , 0) < 0){
		puts("Send failed");
		return 1;
	}

	
    //keep communicating with server till the end of file / end of input from the keyboard
	while(scanf("%s",message)!=EOF)
	{
		// Determines if the command is for Sleep()		
		if(message[0] =='S'){
		  
		  //Moving the pointer to the char following 'S' to obtain the number for Sleep() function call
		  char* substr = message + 1;
		  memmove(message, substr, strlen(substr)+1);
		  //Writing to the output file
		  outputFile.open(fname, ios_base::app);
		  outputFile<<"S"<<message<<"          Sleep  "<<message<<"  units";
		  outputFile << endl << endl;
		  outputFile.close();
		  //Calling the sleep function
		  Sleep((atoi(message)));
		}
       
        //Determines if the command is for Trans()
		else if (message[0] == 'T'){		  
		  
		  //Moving the pointer to the char following 'S' to obtain the number for Sleep() function call
		  char* substr = message + 1;
		  memmove(message, substr, strlen(substr)+1);
		  
		  //Current time
		  //Refernces: https://stackoverflow.com/questions/30702759/implementing-a-timer-in-c and https://stackoverflow.com/questions/6012663/get-unix-timestamp-with-c
		  float fsec1 = time(NULL);

		  //Writing to the output file
		  outputFile.open(fname, ios_base::app);
		  outputFile<<"T"<<message<<"           "<<fsec1<<":  Send("<<"T  "<<message<<")";
		  outputFile << endl << endl;
		  outputFile.close();

		  //Sending mesage to the server
		  if( send(sock , message , strlen(message) , 0) < 0){
			  puts("Send failed");
			  return 1;
		  }
		  sent_trans++;
		  
		  //Receive a reply from the server
		  if( recv(sock , server_reply , 2000 , 0) < 0){
			  puts("recv failed");
			  break;
		  }
		  //Current time
		  //Refernces: https://stackoverflow.com/questions/30702759/implementing-a-timer-in-c and https://stackoverflow.com/questions/6012663/get-unix-timestamp-with-c
		  float fsec2 = time(NULL);

		  //Writing to the output file
		  outputFile.open(fname, ios_base::app);
		  outputFile<<"             "<<fsec2<<":  Recv("<<server_reply<<")";
		  outputFile << endl << endl;
		  outputFile.close();

		}
     
	}

	close(sock);
	
	//Writing to the output file
	outputFile.open(fname, ios_base::app);
	outputFile<<"         Sent  "<<sent_trans<<"  transactions";
	outputFile.close();
	
	return 0;
}
