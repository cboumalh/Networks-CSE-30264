#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>

#include <iostream>
#include <sstream>
#include <fstream>

void signal_handler(int signum) {
	exit(0);
}

int main(int argc, char *argv[]) {
	/* Handle command line arguments */
	if (argc != 3) {
		std::cout << "Usage: " << argv[0] << " <PORT> <FILE-DIR>" << std::endl;
		return 1;
	}

	int port;
	try {
    	port = std::stoi(argv[1]);
	} catch (const std::exception &e) {
		perror("ERROR: Invalid port number");
		return 1;
	}
	if (port < 1 || port > 65535) {
		perror("ERROR: Invalid port number");
		return 1;
	}

	std::string filedir = std::string(argv[2]);

	/* Create a socket using TCP IP */
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);

	/* Set up signal handling */
	signal(SIGTERM, signal_handler);
	signal(SIGQUIT, signal_handler);

	/* set socket option so that socket can be reused even if the underlying TCP connection is in the TIME_WAIT state */
	int yes = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		perror("setsockopt");
		return 1;
	}

	/* Bind address and port to socket */
	struct sockaddr_in addr;
	addr.sin_family = AF_INET; // address family of socket
	addr.sin_port = htons(port); // server will listen on port 4000
	addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Address server should be listening on is its own.
	memset(addr.sin_zero, '\0', sizeof(addr.sin_zero));

	/* Bind sockfd to specific address and port number */
	if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		perror("bind");
		return 2;
	}

	/* Set socket to listen status */
	if (listen(sockfd, 1) == -1) {
		perror("listen");
		return 3;
	}

	/* set timout val */
	struct timeval timeout;
	timeout.tv_sec = 10;
	timeout.tv_usec = 0;

	int connection_id = 0;
	/* Keep accepting connections sequentially */
	while (true) {
		/* Accept a new connection from a client */
		struct sockaddr_in clientAddr;
		socklen_t clientAddrSize = sizeof(clientAddr);
		int clientSockfd = accept(sockfd, (struct sockaddr*)&clientAddr, &clientAddrSize);

		if (clientSockfd == -1) {
			perror("ERROR: Accept");
			return 4;
		}
		++connection_id;

		if (setsockopt(clientSockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == -1) {
			perror("ERROR: Error setting recv timeout.");
			close(clientSockfd);
			continue;
		}

		char ipstr[INET_ADDRSTRLEN] = {'\0'};
		inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, ipstr, sizeof(ipstr));
		std::cout << "Accept a connection from: " << ipstr << ":" << ntohs(clientAddr.sin_port) << std::endl;

		/* Open file in filedir called <connection_id>.file*/
		std::string filename = filedir + "/" + std::to_string(connection_id) + ".file";
		std::ofstream saveFile;
		saveFile.open(filename);

		/* Receive file contents from client and write to saveFile */
		char buf[BUFSIZ];
		ssize_t bytes_received;
		while ((bytes_received = recv(clientSockfd, buf, sizeof(buf), 0)) > 0) {
			saveFile.write(buf, bytes_received);
		}

		/* Determine if any errors */
		if (bytes_received < 0) {
			saveFile.close();
			close(clientSockfd);

			std::ofstream errorFile(filename, std::ios::out | std::ios::trunc);

			if (errorFile.is_open()) {
				errorFile << "ERROR";
				errorFile.close();
			} else {
				std::cerr << "ERROR: Failed to open file for writing." << std::endl;
			}

			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				perror("ERROR: recv() timed out.");
			} else {
				perror("ERROR: recv() failed.");
			}
			
			continue;
		}

		/* Close file and connection */
		saveFile.close();
		close(clientSockfd);
	}

	return 0;
}