/*
 * net_com.h
 *
 *  Created on: 21.04.2020
 *      Author: markus
 */

#ifndef NET_COM_H_
#define NET_COM_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <cstring>
#include <unistd.h>

class Net_com
{
	public:
		int m_port;
		int sockfd;
		char* m_server_address;
		char* m_client_address;
		struct sockaddr_in servaddr, cliaddr;

		Net_com(int port, char* server_address, char* client_address);
		bool net_com_connect(void);
		void net_com_close(void);
		ssize_t net_com_sendto(void* data, size_t length);
		ssize_t net_com_receive(void* data, size_t length);
};


#endif /* NET_COM_H_ */
