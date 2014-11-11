#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>	//<asus-bob1110+>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#define SERV_TCP_PORT 8100 //<asus-bob+>8000	/* server's port number */
#define MAX_SIZE 80
#define	exit(X)	return(X)	//<asus-bob+>

int main(int argc, char *argv[])
{
	int		sockfd, newsockfd, max_fd;
	int		port, recv_len, sel_ret, index;
	char	string[MAX_SIZE], msg[MAX_SIZE];
	fd_set	active_fd_set, read_fds;
	socklen_t	clilen = sizeof(struct sockaddr_in);
	struct sockaddr_in	client_addr, serv_addr;
	struct timeval		tv;
	int		clean_FD;
	int		ret;

  /* command line: server [port_number] */
  
	if(argc == 2) 
		sscanf(argv[1], "%d", &port); /* read the port number if provided */
	else
		port = SERV_TCP_PORT;

	/* open a TCP socket (an Internet stream socket) */
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("can't open stream socket");
		exit(1);
	}
	printf("Internet stream socket FD#%d\n", sockfd);	//dbg

	/* bind the local address, so that the cliend can send to server */
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(port);

	if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		perror("can't bind local address");
		exit(1);
	}

	/* listen to the socket */
	if(listen(sockfd, 5) < 0) {
        perror("listen()");
		exit(1);
	}

    FD_ZERO(&active_fd_set);
    FD_SET(sockfd, &active_fd_set);
    max_fd = sockfd;

	for(;;) {
		/* Set timeout */
		tv.tv_sec = 2;
		tv.tv_usec = 0;

        read_fds = active_fd_set;
		sel_ret = select(max_fd + 1, &read_fds, NULL, NULL, &tv);

		if (sel_ret == -1) {
			perror("select()");
			exit(-1);
		} else if (sel_ret == 0) {
//			printf("select timeout\n");
			continue;
		} else {

			/* Service all sockets */
			for (index = 0; index < FD_SETSIZE; index++) {
                if (FD_ISSET(index, &read_fds)) {
					if (index == sockfd) {
						/* Connection request on original socket. */
						/* Accept */
						newsockfd = accept(sockfd, (struct sockaddr *) &client_addr, &clilen);
						if (newsockfd == -1) {
							perror("accept()");
							exit(-1);
						} else {
							printf("Accpet client come from [%s:%u] by FD#%d\n",
									inet_ntoa(client_addr.sin_addr),
									ntohs(client_addr.sin_port),
									newsockfd);

							/* Add to fd set */
							FD_SET(newsockfd, &active_fd_set);
							if (newsockfd > max_fd)
								max_fd = newsockfd;
						}
					} else {	// index!=sockfd
						/* Data arriving on an already-connected socket */

						/* Receive */
						memset(string, 0, sizeof(string));
						/* read a message from the client */
#if 1
						recv_len = read(index, string, sizeof(string)); 
#else
						recv_len = recv(index, string, sizeof(string), 0);
#endif
						/* make sure it's a proper string */

						if (recv_len == -1) {
//							perror("recv()");
							clean_FD = 1;
						} else if (recv_len == 0) {
//							perror("Client disconnect\n");
							clean_FD = 1;
						} else {
							printf("Receive: len=[%d] msg=[%s] for FD#%d\n", recv_len, string, index);
							/* Send (In fact we should determine when it can be written)*/

							/* write a message to the server */
							sprintf(string, "%s from Server", string);
							printf("send [%s]\n", string); //dbg
#if 1
							ret = write(index, string, sizeof(string));
#else
							ret = send(index, string, recv_len, 0);
#endif
							if(sizeof(msg) != ret)
								printf("write ret:%d\n", ret);	//dbg

							clean_FD = 0;
						}

						if(clean_FD)
						{
							printf("FD#%d disconnect\n", index);
							/* Clean up */
							close(index);
							FD_CLR(index, &active_fd_set);
						}
					} //if(i == sockfd)
				} //if (FD_ISSET(index, &read_fds))
			} //for (index = 0; index < FD_SETSIZE; index++)
		} //if (sel_ret == -1) {
	} //for(;;)

	exit(0);
}
