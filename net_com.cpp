/*
 * net_com.cpp
 *
 *  Created on: 21.04.2020
 *      Author: markus
 */

#include "net_com.h"

Net_com::Net_com(int port, char* server_address, char* client_address) : m_port(port), m_server_address(server_address), m_client_address(client_address)
{

}

/*
 *
 */
bool Net_com::net_com_connect(void)
{
	bool ret = true;
	// Creating socket file descriptor
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0 ) {
		ret = false;
	}

	memset(&servaddr, 0, sizeof(servaddr));
	memset(&cliaddr, 0, sizeof(cliaddr));



	cliaddr.sin_family = AF_INET;
	cliaddr.sin_port = htons(m_port);
	cliaddr.sin_addr.s_addr = inet_addr(m_client_address);//htonl(INADDR_ANY);

	// Filling server information
	servaddr.sin_family    = AF_INET; // IPv4
	servaddr.sin_addr.s_addr = inet_addr(m_server_address);;
	servaddr.sin_port = htons(m_port);

	// Bind the socket with the server address
	if ( bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 )
	{
		ret = false;
	}
	return ret;
}

void Net_com::net_com_close(void)
{
	close(sockfd);
}

ssize_t Net_com::net_com_sendto(void* data, size_t length)
{
	int len = sizeof(cliaddr);
	return sendto(sockfd, data, length, 0, (struct sockaddr *) &cliaddr, len);
}

ssize_t Net_com::net_com_receive(void* data, size_t length)
{
	socklen_t len;
	ssize_t ret;
	ret = recvfrom(sockfd, data, length, 0, (struct sockaddr *) &cliaddr, &len);
	return ret;
}
