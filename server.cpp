/*
Server Program for handling multiple socket connections with select and fd_set on Linux
Adapted from Geeks for Geeks: https://www.geeksforgeeks.org/socket-programming-in-cc-handling-multiple-clients-on-server-without-multi-threading/
@author Gurbani Baweja
*/

#include <stdio.h>
#include <string.h> 
#include <stdlib.h>
#include <errno.h>
#include <unistd.h> 
#include <arpa/inet.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h> 
#include <fstream>
#include<iostream>
#include "tands.h"

using namespace std; 

#define TRUE 1
#define FALSE 0

	
int main(int argc , char *argv[])
{
	int opt = TRUE;
	int master_socket , addrlen , new_socket, read_size, client_socket[20] ,
		max_clients = 20 , activity, i , valread , sd;
	int max_sd;
	struct sockaddr_in address;

	char client_message[2000], message[2000], send_message[5];
	std::string sendMessage;
	int transNum;   // For storing number of transactions per client

	int port_num = atoi(argv[1]);

	// A boolean variable to determine if server should exit or not 
	bool serverExit = false; 
	
	// Writing to the output file 
	string fname = "Server.log"; // File name
	ofstream outputFile;
	outputFile.open(fname); 
	outputFile<<"            Server Output            \n";
    outputFile<<"Using port "<<port_num<<"\n";
	outputFile.close();

	// Variables to find time elapsed while waiting for client message
	// Reference: https://stackoverflow.com/questions/26112839/c-find-elapsed-time-in-c-linux
	struct timeval connectionStart_timeval, receivedMessage_timeval;
	unsigned long elapsed_seconds = 0;
				
	//set of socket descriptors
	fd_set readfds;
		
	
	//initialise all client_socket[] to 0 so not checked
	for (i = 0; i < max_clients; i++)
	{
		client_socket[i] = 0;
	}
		
	//create a master socket
	if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}
	
	//set master socket to allow multiple connections 
	if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
		sizeof(opt)) < 0 )
	{
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	
	//type of socket created
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons( port_num );
		
	//bind the socket to port number provided 
	if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
		
	// Specifying maximum of 3 pending connections for the master socket
	if (listen(master_socket, 3) < 0)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}
		
	//accept the incoming connection
	addrlen = sizeof(address);
	puts("Waiting for connections ...");
		
	while(serverExit == false)
	{
		//clear the socket set
		FD_ZERO(&readfds);
	
		//add master socket to set
		FD_SET(master_socket, &readfds);
		max_sd = master_socket;
			
		//add child sockets to set
		for ( i = 0 ; i < max_clients ; i++)
		{
			//socket descriptor
			sd = client_socket[i];
				
			//if valid socket descriptor then add to read list
			if(sd > 0)
				FD_SET( sd , &readfds);
				
			//highest file descriptor number, need it for the select function
			if(sd > max_sd)
				max_sd = sd;
		}
	
		gettimeofday(&connectionStart_timeval, NULL);

		//wait for an activity on one of the sockets 
		activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);    
	    
		gettimeofday(&receivedMessage_timeval, NULL);
		elapsed_seconds = receivedMessage_timeval.tv_sec - connectionStart_timeval.tv_sec;
		
		// if server is idle more than 30 seconds, it exits  
		if(elapsed_seconds > 30){
			serverExit = true;
		}

		if ((activity < 0) && (errno!=EINTR))
		{
			printf("select error");
		}
			
		//If something happened on the master socket ,
		//then its an incoming connection
		if (FD_ISSET(master_socket, &readfds))
		{
			if ((new_socket = accept(master_socket,
					(struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
			{
				perror("accept");
				exit(EXIT_FAILURE);
			}
		    // CONNECTION ACCEPTED	

		    // Receiving the client name from the client, here message contains the client name
			if ((valread = read( new_socket , message, 1024)) == 0)
			{
				perror("recv failed");

			}
			
				
			//add new socket to array of sockets
			for (i = 0; i < max_clients; i++)
			{
				//if position is empty
				if( client_socket[i] == 0 )
				{
					client_socket[i] = new_socket;
					printf("Adding to list of sockets as %d\n" , i);
						
					break;
				}
			}
			
		}
			
		//else its some message from some other socket
		for (i = 0; i < max_clients; i++)
		{   
			sd = client_socket[i];
				
			if (FD_ISSET( sd , &readfds))
			{
                //Check if it was for closing , and also read the
				//incoming message
				if ((valread = read( sd , client_message, 1024)) == 0)
				{
					//Somebody disconnected , get their details and print
					getpeername(sd , (struct sockaddr*)&address , \
						(socklen_t*)&addrlen);
					printf("Host disconnected , ip %s , port %d \n" ,
						inet_ntoa(address.sin_addr) , ntohs(address.sin_port));
						
					//Close the socket and mark as 0 in list for reuse
					close( sd );
					client_socket[i] = 0;
				}
					
				//Receive message from the client and send back a receipt as acknowledgement of the message
				else
				{   
					//Current time in decimal
                    float fsec1 = time(NULL);

					//Converting the number sent by client for Trans(int num) into an int from char
					int num = atoi(client_message);
					//Calling Trans()
                    Trans(num);  
                    transNum++;

					//Writing to the output file
					outputFile.open(fname, ios_base::app);
					outputFile<<fsec1<<": # "<<transNum<<"   (T "<<num<<" ) from "<<message<<"\n";
					outputFile.close();
                    
					//Message to be sent back to the server as a receipt of acknowledgement 
                    sendMessage = "D " + std::to_string(transNum);
                    strcpy(send_message,sendMessage.c_str());   //converting string message to char array
                    puts(send_message);

					//Current time in decimal
                    float fsec2 = time(NULL);

					send(sd,send_message,strlen(send_message),0);
					
					//Writing to the output file
					outputFile.open(fname, ios_base::app);
					outputFile<<fsec2<<": # "<<transNum<<"   ( Done )"<<" from "<<message<<"\n";
					outputFile.close();
				}
			}
			
		}
		//Writing to the output file
		outputFile.open(fname, ios_base::app);
		outputFile<<"  Summary"<<"\n";
		outputFile<<"    "<<transNum<<"  from  "<<message<<"\n";
		//outputFile<<"  Summary"<<"\n";
		outputFile.close();
	}

	return 0;
}
