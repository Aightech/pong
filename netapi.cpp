#include "netapi.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#include <net/if.h>
#include <err.h>
#include <errno.h>

#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

#include <netdb.h>
#include <arpa/inet.h>

#include <sys/ioctl.h>


#include <net/if.h>
#include <sys/wait.h>

#include <fcntl.h>


int waitSec(float sec,bool verbose)
{
	clock_t start=clock();
	char buff[] = "[  Waiting ... ]";
	if(verbose)
	{
		printf("Info : %.3lfs %s\xd",((double)(clock()-start))/CLOCKS_PER_SEC,buff);
		fflush(stdout);
	}
	while(clock()-start < sec*CLOCKS_PER_SEC)
	{
		if(verbose)
		{
			buff[1+(int)(14*(clock()-start)/(sec*CLOCKS_PER_SEC))] = '#';
			printf("Info : %.3lfs %s\xd",((double)(clock()-start))/CLOCKS_PER_SEC,buff);
			fflush(stdout);
		}
	}
}



NetAPI::NetAPI()
{

	for(int i =0; i<NB_BUFFERS; i++)
		m_RxBufferFilled[i]=false;
	//by default the mode verbose in unset
	m_verbose = false;    
};

int NetAPI::scan(int port,char servers[5][16],int IPmin ,int IPmax)
{
	//char servers[5][64];
	// --- Find the default IP interface --- //
       FILE *f;
       char line[100] , *iface , *c;
       f = fopen("/proc/net/route" , "r");
       while(fgets(line , 100 , f))
       {
              iface = strtok(line , " \t");
              c = strtok(NULL , " \t");
              if(iface!=NULL && c!=NULL && strcmp(c , "00000000") == 0)
                     break;
       } 
       fclose(f);
       // ------------------------------------- //
       
       
       // --- Find the IP address of the default interface --- //
       int fd;
       struct ifreq ifr;

       fd = socket(AF_INET, SOCK_DGRAM, 0);
       ifr.ifr_addr.sa_family = AF_INET;           //Type of address to retrieve - IPv4 IP address
       strncpy(ifr.ifr_name , iface , IFNAMSIZ-1); //Copy the interface name in the ifreq structure
       ioctl(fd, SIOCGIFADDR, &ifr);               //Get the ip address
       close(fd);
       char addressIP[16];
       strcpy(addressIP,inet_ntoa(( (struct sockaddr_in *)&ifr.ifr_addr)->sin_addr) );
       // --------------------------------------------------- //
       
       //display result
       printf("Default interface is : %s \n" , iface);
       printf("IP address : %s\n" , addressIP );
       
       // --- Get The pc ID --- //
        //look for the last point of the ip address
       int i=0,pt=0;
       while(i<16&&pt<3)
              if(addressIP[i++]=='.')
                     pt++;
                     
       int pc = atoi(addressIP+i); //get the host id of the computer  
       addressIP[i]='\0';
       // ------------------------ //
       
       
       char addr[16];
       int sfd;
       int sockfd, portno;
       struct sockaddr_in serv_addr;
       int res, valopt; 
       long arg; 
       fd_set myset; 
       struct timeval tv; 
       socklen_t lon;
       
       char serv_IPaddr[255][16];
       int h=0,s=0;
       int testingPort=port;
       
       
       printf("scanning...\n[");
       fflush(stdout);
       for(int i=IPmin;i<IPmax;i++)
       {
              
              if(i%5==0)
              {
                     printf("#");
                     fflush(stdout);
              }
              sprintf(addr,"%s%d" ,addressIP,i);
              //printf("%s\n",addr);
              
              sfd = socket(AF_INET, SOCK_STREAM, 0);
              
              if (sfd < 0) printf("ERROR opening socket");
              
              // Set non-blocking 
              arg = fcntl(sfd, F_GETFL, NULL); 
              arg |= O_NONBLOCK; 
              fcntl(sfd, F_SETFL, arg);
              
              serv_addr.sin_family = AF_INET;
              serv_addr.sin_addr.s_addr=inet_addr(addr);
              serv_addr.sin_port = htons(testingPort);
              if (connect(sfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
              { 
                     if (errno == EINPROGRESS) 
                     {
		              //printf("connect EINPROGRESS OK (expected)   ");
		              tv.tv_sec = 0; 
                            tv.tv_usec = 10000; 
                            FD_ZERO(&myset); 
                            FD_SET(sfd, &myset); 
                            if (select(sfd+1, NULL, &myset, NULL, &tv) > 0) 
                            { 
                                   lon = sizeof(int); 
                                   getsockopt(sfd, SOL_SOCKET, SO_ERROR, (void*)(&valopt), &lon); 
                                   if(valopt==0)
                                   { 
                                          if(h<255)
                                                 strcpy(serv_IPaddr[h++],addr);
                                   }
                                   
                            }  
                     }
			
              }
              else
                     printf("somebody on %s\n\n" , addr);
                     
              arg = fcntl(sfd, F_GETFL, NULL); 
              arg &= (~O_NONBLOCK); 
              fcntl(sfd, F_SETFL, arg);       
                     
              close(sfd);
       }
       printf("]\n");
       char request[30];
	sprintf(request,"C%dP%s",m_Rxport,"test");
	char reply[50];
       
       //Buffer B;
       //strcpy(B.Tx,"G0");
       
       for(i=0;i<h;i++)
       {
              printf("- Somebody on: %s : ",serv_IPaddr[i]);
              
		
		
		struct sockaddr_in addr = getAddr(port,serv_IPaddr[i]);
		int n = this->send(&addr,(char *)request,(char*)"tcp",reply);
		//printf("reply : %s\n",reply);
		if(strcmp(reply,"server")==0)//if the server accepted the connection request by replying "accepted"
		{
			printf("server\n");
			strcpy(servers[s],serv_IPaddr[i]);
			s++;
		}
//              B.T_flag=1;
//              sendTCP(serv_IPaddr[i],testingPort,&B);
//              if(atoi(strchr(B.Rx,'G')+1)==0)
//              {
//                     printf("\tsh13 server\n");
//                     strcpy(servers[s].IPaddress,serv_IPaddr[i]);
//                     strcpy(servers[s].name, strtok((strchr(B.Rx,'N')+1),";"));
//                     servers[s++].portNo=testingPort;
//              }
//              else
                     
              
       }
return s;
}

struct sockaddr_in NetAPI::getAddr(int port, char * hostname)
{
	/* gethostbyname: get the server's DNS entry */
	struct sockaddr_in addr;
	struct hostent * dest = gethostbyname(hostname);
	if (dest == NULL && m_verbose)
	{
		m_verboseMtx.lock();
		printf("Tx:ERROR, no such host as %s\n", hostname);
		m_verboseMtx.unlock();
	}
	/* build the server's Internet address */
	bzero((char *) &addr, sizeof(addr));
	addr.sin_family = AF_INET;
	bcopy((char *)dest->h_addr, (char *)&addr.sin_addr.s_addr, dest->h_length);
	addr.sin_port = htons(port);
	//use the basic send methode
	return addr;
}

void NetAPI::sendToClient(int index, char * buf, char *protocol)
{
	if(index == ALL_CLIENT)
		for (int i = 0; i < m_claddr.size(); i ++)
			send(m_claddr[i],buf,protocol);
	else if(index < m_claddr.size())
		send(m_claddr[index],buf,protocol);
}

void NetAPI::clearSendingThread()
{
	for (int i = 0; i < m_TxThread.size(); i ++)
	{
		m_TxThread.back()->join();
		m_TxThread.pop_back();
	}
}

int NetAPI::send(struct sockaddr_in * addr, char * buf, char *protocol, char * recvBuff)
{	
	return _send(addr, buf, protocol, recvBuff);
	/*m_TxThread.push_back( new std::thread(&NetAPI::_send, this, addr,  buf, protocol, recvBuff) );
	if(recvBuff != NULL)
	{
		m_TxThread.back()->join();
		m_TxThread.pop_back();
	}*/
	//printf("------- %d ------\n", m_TxThread.size());
}

int NetAPI::_send(struct sockaddr_in * addr, char * buf, char *protocol, char * recvBuff)
{
	//----CHECKING THE PROTOCOL---- Default is udp
	bool udpP = false, tcpP = false;
	
	if(protocol==NULL || strcmp(protocol,"udp")==0 || strcmp(protocol,"UDP")==0 )
		udpP = true;
	else if ( strcmp(protocol,"tcp")==0 || strcmp(protocol,"TCP")==0)
		tcpP = true;
	else
	{	m_verboseMtx.lock();	printf("ERROR protocol unknown\n");	m_verboseMtx.unlock();	return -1;}
	//----GET A SOCKET----
	int m_Txfd = 0;
	if(udpP)
		m_Txfd = socket(AF_INET, SOCK_DGRAM, 0);
	else if(tcpP)
		m_Txfd = socket(AF_INET, SOCK_STREAM, 0);
	if (m_Txfd < 0 && m_verbose) {	m_verboseMtx.lock();	printf("ERROR opening Tx socket\n");	m_verboseMtx.unlock();	}
	
	
	//----SEND THE MESSAGE ----
	int n=0;
	if(udpP)
		n = sendto(m_Txfd, buf, strlen(buf), MSG_DONTWAIT, (struct sockaddr *)addr, sizeof(*addr));
	else if (tcpP)
	{
		if (connect(m_Txfd,(struct sockaddr *) addr,sizeof(*addr)) < 0 && m_verbose) 
		{m_verboseMtx.lock();	printf("Tx:ERROR couldn't connect\n");	m_verboseMtx.unlock();	return -1;}
		
		n = write(m_Txfd, buf, strlen(buf));
	}
	
	//----DISPLAY WHAT HAS BEEN SENT
	if (m_verbose) 
	{
		m_verboseMtx.lock();
		printf("Tx:Sent [%s], to %s:%d\n", buf, inet_ntoa(addr->sin_addr), ntohs(addr->sin_port));
		if(n < 0)
			printf("Tx:ERROR in sendto\n");
		m_verboseMtx.unlock();
	}
	
	// FINISH HERE IF THERE NO RECEIVING BUFFER ----
	if(recvBuff==NULL){	close(m_Txfd);	return n;	}
	
	waitSec(0.01);
	
	// ----GET THE REPLY ----//
	int m;
	if(udpP)
		m = recvfrom(m_Txfd, recvBuff, BUFSIZE, 0, (struct sockaddr*)addr, &m_Txlen);
	else if(tcpP)
		m = read(m_Txfd, recvBuff, BUFSIZE);
	
	//---DISPLAY WHAT HAS BEEN RECEIVED
	if (m_verbose) 
	{
		m_verboseMtx.lock();
		printf("Tx:data from server: [%s]\n", recvBuff);
		if(n < 0)
			printf("Tx:ERROR in recvfrom\n");
		m_verboseMtx.unlock();
	}
	close(m_Txfd);
	return m;
	
};

int NetAPI::connectToServer(int port, char * IP)
{
	char request[30];
	sprintf(request,"C%dP%s",m_Rxport,m_connectionPhrase);
	char reply[50];
	
	
	struct sockaddr_in addr = getAddr(port,IP);
	int n = this->send(&addr,(char *)request,(char*)"tcp",reply);

	if(strcmp(reply,"accepted")==0)//if the server accepted the connection request by replying "accepted"
	{
		/* build the server's Internet address */
		m_Txdest = gethostbyname(IP);
		bzero((char *) &m_Serveraddr, sizeof(m_Serveraddr));
		m_Serveraddr.sin_family = AF_INET;
		bcopy((char *)m_Txdest->h_addr, (char *)&m_Serveraddr.sin_addr.s_addr, m_Txdest->h_length);
		m_Serveraddr.sin_port = htons(port);
		if(m_verbose)
		{
			m_verboseMtx.lock();
			printf("General:Connection success to %s:%d\n", inet_ntoa(m_Serveraddr.sin_addr), ntohs(m_Serveraddr.sin_port));
			m_verboseMtx.unlock();
		}
		return 1;
	}
	else
	{
		if(m_verbose)
		{
			m_verboseMtx.lock();
			printf("General:Connection fail to %s:%d\n", IP, port);
			m_verboseMtx.unlock();
		}
		return 0;
	}

}

int NetAPI::startReceiver(int port, char * protocol)
{
	m_Rxport = port;
	if (strcmp(protocol,"UDP")==0 || strcmp(protocol,"udp")==0)
	{
		m_Rxfd = socket(AF_INET, SOCK_DGRAM, 0);//socket: create the parent socket 
		if (m_Rxfd < 0 && m_verbose) 
		{
			m_verboseMtx.lock();
			printf("ERROR opening Rx socket\n");
			m_verboseMtx.unlock();
		}
		//start the receiver thread
		m_ReceiverThread = new std::thread(&NetAPI::receiverUDP, this);
	}
	else if (strcmp(protocol,"TCP")==0 || strcmp(protocol,"tcp")==0)
	{
		m_Rxfd = socket(AF_INET, SOCK_STREAM, 0);//socket: create the parent socket 
		if (m_Rxfd < 0 && m_verbose) 
		{
			m_verboseMtx.lock();
			printf("ERROR opening Rx socket\n");
			m_verboseMtx.unlock();
		}
		//start the receiver thread
		m_ReceiverThread = new std::thread(&NetAPI::receiverTCP, this);
	}
	else
	{
		if (m_verbose) 
		{
			m_verboseMtx.lock();
			printf("ERROR Protocol unknown\n");
			m_verboseMtx.unlock();
		}
	}

	waitSec(0.1);
	waitSec(0.5,true);


}

int NetAPI::receiverUDP()
{
	/* setsockopt: Handy debugging trick that lets 
	* us rerun the server immediately after we kill it; 
	* otherwise we have to wait about 20 secs. 
	* Eliminates "ERROR on binding: Address already in use" error. 
	*/
	int optval = 1;/* flag value for setsockopt */
	setsockopt(m_Rxfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));

	//build the server's Internet address
	bzero((char *) &m_Rxaddr, sizeof(m_Rxaddr));
	m_Rxaddr.sin_family = AF_INET;
	m_Rxaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	m_Rxaddr.sin_port = htons((unsigned short)m_Rxport);

	//bind: associate the parent socket with a port 
	if(bind(m_Rxfd, (struct sockaddr *) &m_Rxaddr, sizeof(m_Rxaddr)) < 0 && m_verbose) 
	{
		m_verboseMtx.lock();
		printf("Rx:ERROR on binding\n");
		m_verboseMtx.unlock();
	}

	//main loop: wait for a datagram, then echo it
	m_cllen = sizeof(&m_clientaddr);

	m_ReceiverActive = true;
	if(m_verbose)
	{
		m_verboseMtx.lock();
		printf("Rx: Receiver UDP started listening on port %d:\n", m_Rxport);
		m_verboseMtx.unlock();
	}

	m_clientIndex =0;
	char tempBuf[BUFSIZE];
	int currentBuffer;
	while (m_ReceiverActive) 
	{
		currentBuffer=0;
		while(m_RxBufferFilled[currentBuffer] && ++currentBuffer < NB_BUFFERS){}
		if(currentBuffer < NB_BUFFERS)
		{
			m_bufferMtx[currentBuffer].lock();//lock the mutex of the new buffer
			/*
			* recvfrom: receive a UDP datagram from a client
			*/
			bzero(tempBuf, BUFSIZE);
			int n = recvfrom(m_Rxfd, tempBuf, BUFSIZE,MSG_WAITALL,(struct sockaddr *)& m_clientaddr, &m_cllen);

			m_verboseMtx.lock();
			if (n < 0 && m_verbose)
				printf("Rx:ERROR in recvfrom\n");
			if(m_verbose)
				printf("Rx:Received packet from %s:%d\nRx:Buffer temp: [%s]\n\n", inet_ntoa(m_clientaddr.sin_addr), ntohs(m_clientaddr.sin_port),tempBuf);
			m_verboseMtx.unlock();

			char reply[50]="nope";

			int port;
			switch(tempBuf[0])
			{
				case 'M'://message
				{
					strcpy(m_RxBuffer[currentBuffer],tempBuf);
					
					processReceiverMessage(tempBuf,reply);
					//printf("Rx:Buffer[%d]: [%s]\n\n",currentBuffer, m_RxBuffer[currentBuffer]);
					m_RxBufferFilled[currentBuffer]=true;
				}break;
				case 'C'://connection attempt :  C[portno]P[connectionPhrase]
				{
					if(m_connectable)//if the connections are allowed
					{
						port = atoi(tempBuf+1);
						if(m_connectionPhrase != NULL && strcmp(strchr(tempBuf,'P')+1,m_connectionPhrase)==0)
						{
							if(m_clientIndex<NB_MAX_CLIENT)
							{
								strcpy(reply,"accepted");

								m_claddr.push_back(new struct sockaddr_in);
								bzero((char *) m_claddr[m_clientIndex], sizeof(&m_claddr[m_clientIndex]));
								m_claddr[m_clientIndex]->sin_family = AF_INET;
								m_claddr[m_clientIndex]->sin_addr.s_addr = m_clientaddr.sin_addr.s_addr;
								m_claddr[m_clientIndex]->sin_port = htons((unsigned short)port);



								if(m_verbose) 
								{
									m_verboseMtx.lock();
									printf("Rx:Client %s:%d accepted.\n", inet_ntoa(m_claddr[m_clientIndex]->sin_addr),port);
									m_verboseMtx.unlock();
								}
								m_clientIndex++;


							}
							else
							{
								strcpy(reply,"full");
							}

						}
						else
						{
							m_verboseMtx.lock();
							if(m_connectionPhrase == NULL)
								printf("Rx:Connection phrase need to be set.\n");
							else
								printf("Rx:Connection phrase incorect\n"); 
							m_verboseMtx.unlock();
						}
					}
					else if(m_verbose)
					{
						m_verboseMtx.lock();
						printf("Rx:Api is not connectable. \n");
						m_verboseMtx.unlock(); 
					}

				}break;
				default:
				{

				}



			}

			m_bufferMtx[currentBuffer].unlock();

		}
	}
	
	if(m_verbose)
	{
		m_verboseMtx.lock();
		printf("Rx:Receiver stoped listening port %d\n", m_Rxport);
		m_verboseMtx.unlock();
	}

	return 0;

}

int NetAPI::receiverTCP()
{
	/* setsockopt: Handy debugging trick that lets 
	* us rerun the server immediately after we kill it; 
	* otherwise we have to wait about 20 secs. 
	* Eliminates "ERROR on binding: Address already in use" error. 
	*/
	int optval = 1;/* flag value for setsockopt */
	setsockopt(m_Rxfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));

	/*
	* build the server's Internet address
	*/
	bzero((char *) &m_Rxaddr, sizeof(m_Rxaddr));
	m_Rxaddr.sin_family = AF_INET;
	m_Rxaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	m_Rxaddr.sin_port = htons((unsigned short)m_Rxport);

	/* 
	* bind: associate the parent socket with a port 
	*/
	if(bind(m_Rxfd, (struct sockaddr *) &m_Rxaddr, sizeof(m_Rxaddr)) < 0 && m_verbose) 
	{
		m_verboseMtx.lock();
		printf("Rx:ERROR on binding\n");
		m_verboseMtx.unlock();
	}

	/* 
	* main loop: wait for a datagram, then echo it
	*/
	m_cllen = sizeof(m_clientaddr);
	listen(m_Rxfd,5);


	m_ReceiverActive = true;
	if(m_verbose)
	{
		m_verboseMtx.lock();
		printf("Rx: Receiver TCP started listening on port %d:\n", m_Rxport);
		m_verboseMtx.unlock();
	}

	
	m_clientIndex =0;
	char tempBuf[BUFSIZE];

	int newsocket;
	int currentBuffer;
	while (m_ReceiverActive) 
	{

		currentBuffer=0;
		while(m_RxBufferFilled[currentBuffer] && ++currentBuffer < NB_BUFFERS){}
		if(currentBuffer < NB_BUFFERS)
		{
			m_bufferMtx[currentBuffer].lock();//lock the mutex of the new buffer
			newsocket = accept(m_Rxfd,(struct sockaddr *)& m_clientaddr, &m_cllen);
			/*
			* recvfrom: receive a UDP datagram from a client
			*/
			bzero(tempBuf, BUFSIZE);
			int n = read(newsocket, tempBuf, BUFSIZE);

			m_verboseMtx.lock();
			if (n < 0 && m_verbose)
				printf("Rx:ERROR in recvfrom\n");
			if(m_verbose)
				printf("Rx:Received packet from %s:%d\nRx:Buffer temp: [%s]\n\n", inet_ntoa(m_clientaddr.sin_addr), ntohs(m_clientaddr.sin_port),tempBuf);
			m_verboseMtx.unlock();

			char reply[50]="nope";

			int port;
			switch(tempBuf[0])
			{
				case 'M'://message
				{
					
					strcpy(m_RxBuffer[currentBuffer],tempBuf);
					processReceiverMessage(tempBuf,reply);
					printf("Rx:Buffer[%d]: [%s]\n\n",currentBuffer, m_RxBuffer[currentBuffer]);
					m_RxBufferFilled[currentBuffer]=true;
					
				}break;
				case 'C'://connection attempt :  C[portno]P[connectionPhrase]
				{
					if(m_connectable)//if the connections are allowed
					{
						port = atoi(tempBuf+1);
						if(m_connectionPhrase != NULL && strcmp(strchr(tempBuf,'P')+1,m_connectionPhrase)==0)
						{
							if(m_clientIndex<NB_MAX_CLIENT)
							{
								strcpy(reply,"accepted");

								m_claddr.push_back(new struct sockaddr_in);
								bzero((char *) m_claddr[m_clientIndex], sizeof(&m_claddr[m_clientIndex]));
								m_claddr[m_clientIndex]->sin_family = AF_INET;
								m_claddr[m_clientIndex]->sin_addr.s_addr = m_clientaddr.sin_addr.s_addr;
								m_claddr[m_clientIndex]->sin_port = htons((unsigned short)port);



								if(m_verbose) 
								{
									m_verboseMtx.lock();
									printf("Rx:Client %s:%d accepted.\n", inet_ntoa(m_claddr[m_clientIndex]->sin_addr),port);
									m_verboseMtx.unlock();
								}
								m_clientIndex++;


							}
							else
							{
							strcpy(reply,"full");
							}

						}
						else
						{
							m_verboseMtx.lock();
							if(m_connectionPhrase == NULL)
								printf("Rx:Connection phrase need to be set.\n");
							else
								printf("Rx:Connection phrase incorect\n"); 
							m_verboseMtx.unlock();
							strcpy(reply,"server");
						}
					}
					else if(m_verbose)
					{
						m_verboseMtx.lock();
						printf("Rx:Api is not connectable. \n");
						m_verboseMtx.unlock(); 
					}

				}break;
				default:
				{

				}



			}
			

			/*    
			* sendto: echo the input back to the client 
			*/
			n = write(newsocket, reply, sizeof(reply));
			m_verboseMtx.lock();
			if (n < 0 && m_verbose) 
				printf("Rx:ERROR in sendto\n");
			else if(m_verbose)
				printf("Rx:Replied to %s: [%s]\n", inet_ntoa(m_clientaddr.sin_addr),reply);
			m_verboseMtx.unlock();

			close(newsocket);
			m_bufferMtx[currentBuffer].unlock();//lock the mutex of the new buffer
		}
		
	}
	close(m_Rxfd);
	if(m_verbose)
	{
		m_verboseMtx.lock();
		printf("Rx:Receiver stoped listening port %d\n", m_Rxport);
		m_verboseMtx.unlock();
	}

	return 0;

}

int NetAPI::endReceiver()
{
	m_ReceiverActive = false;//end the loop of the receiver
	struct sockaddr_in addr = getAddr(m_Rxport,(char *)"127.0.0.1");
	this->send(&addr,(char *)"halt",(char*)"tcp");//send a stopping message to it 
	if(m_ReceiverThread->joinable())//wait for it
		m_ReceiverThread->join();
	return 1;
}

int NetAPI::getReceiverBuffer(char *buffer)
{
	int currentBuffer = 0;
	while(!m_RxBufferFilled[currentBuffer] && ++currentBuffer < NB_BUFFERS){}
	if(currentBuffer < NB_BUFFERS)//if some buffer is unread
	{
		m_bufferMtx[currentBuffer].lock();//mutex to ensure the well being of the data
		//printf("Buffer [%d]: %s  \n",currentBuffer,m_RxBuffer[currentBuffer]);
		strcpy(buffer,m_RxBuffer[currentBuffer]);//copy the buffer
		m_RxBufferFilled[currentBuffer]=false;
		m_bufferMtx[currentBuffer].unlock();
		return currentBuffer;
	}
	return -1;
}

void NetAPI::setConnectionPhrase(char * conPhr)
{
	if(m_connectionPhrase != NULL)	delete m_connectionPhrase;
	m_connectionPhrase = new char[strlen(conPhr)+1];
	strcpy(m_connectionPhrase,conPhr);
}

const vector<struct sockaddr_in *>& NetAPI::getClientAddr() const
{
	if(m_verbose)
	{
		(const_cast<NetAPI*> (this))->m_verboseMtx.lock();
		printf("General:Client Listing (%d clients)\n",(int)m_claddr.size());
		for(int i=0; i<m_claddr.size();i++)
		     printf("General:Client n°%d : %s:%d \n",i,  inet_ntoa(m_claddr[i]->sin_addr), ntohs(m_claddr[i]->sin_port));
		(const_cast<NetAPI*> (this))->m_verboseMtx.unlock();
	} 
	return   m_claddr;
}

int NetAPI::say(char * str)
{
	if(m_verbose)
	{
		m_verboseMtx.lock();
		printf("%s\n", str);
		m_verboseMtx.unlock();
	}
}
