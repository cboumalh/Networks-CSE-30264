#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>

#include <iostream>
#include <sstream>

int send_file(const char *filename, int sockfd) {
	/* Open file for reading */
	FILE *file = fopen(filename, "rb"); // rb (read binary) since we are tranfering contents over the internet
	if (!file) {
		perror("fopen failed");
		return 6;
	}

	/* Send the file contents to server */
	char buf[BUFSIZ];
	size_t bytes_read;
	while ((bytes_read = fread(buf, 1, sizeof(buf), file)) > 0) {
		if (send(sockfd, buf, bytes_read, 0) == -1) {
			perror("ERROR");
			fclose(file);
			return 7;	
		}
	}

	/* Close file and return success */
	fclose(file);
	return 0;
}

int main(int argc, char *argv[]) {
	/* Handle command line arguments */
	if (argc != 4) {
		std::cout << "Usage: " << argv[0] << " <HOSTNAME-OR-IP> <PORT> <FILENAME>" << std::endl;
		return 1;
	}

	int port;
	try {
    	port = std::stoi(argv[2]);
	} catch (const std::exception &e) {
		perror("ERROR: Invalid port number.");
		return 1;
	}
	char *filename = argv[3];

	if (port < 1 || port > 65535) { // valid port #s range from 1-65535
		perror("ERROR: Invalid port number.");
		return 1;
	}

	/* Set up addrinfo struct */
	struct addrinfo *server_info, *p;
	struct addrinfo hints = {0};
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	if (getaddrinfo(argv[1], argv[2], &hints, &server_info) != 0) {
		perror("ERROR: Could not obtain address info.");
		return 1;
	}

	/* Set socket timeout val to 10 secs 0 ms*/
	struct timeval timeout;
	timeout.tv_sec = 10;
	timeout.tv_usec = 0;

	/* For each server entry, allocate socket and try to connect */
	int sockfd = -1;
	for (p = server_info; p != NULL && sockfd < 0; p = p->ai_next) {
		/* Allocate socket */
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) {
			continue;
		}

		/* setsockopt on socket to implement timeout */
		if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) == -1) {
			perror("ERROR: Error setting timeout.");
			close(sockfd);
			sockfd = -1;
			continue;
		}
		/* Connect to host */
		if (connect(sockfd, p->ai_addr, p->ai_addrlen) < 0) {
			close(sockfd);
			sockfd = -1;
			continue;
		}
	}
	if (p == NULL) {
		perror("ERROR: Could not connect to the server.");
		return 2;
	}

	freeaddrinfo(server_info);

	/* Get clientAddr information from OS after connection is made using getsockname */
	struct sockaddr_in clientAddr;
	socklen_t clientAddrLen = sizeof(clientAddr);
	if (getsockname(sockfd, (struct sockaddr *)&clientAddr, &clientAddrLen) == -1) {
		perror("getsockname");
		return 3;
	}

	char ipstr[INET_ADDRSTRLEN] = {'\0'};
	inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, ipstr, sizeof(ipstr));
	std::cout << "Set up a connection from: " << ipstr << ":" <<
	ntohs(clientAddr.sin_port) << std::endl;

	/* Send File */
	int err = send_file(filename, sockfd);

	printf("%d", err);

	close(sockfd);

	return 0;
}