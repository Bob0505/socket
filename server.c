#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>	//<asus-bob1110+>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#define SERV_TCP_PORT 8100 //<asus-bob+>8000	/* server's port number */
#define MAX_SIZE 80
#define	exit(X)	return(X)

#define	TRUE	1
#define	FALSE	0

int TCPsocket_bind_listen(int *TCP_socketfd)
{
	int		ret;
	struct sockaddr_in	serv_addr;	

	/* open a TCP socket (an Internet stream socket) */
	*TCP_socketfd = socket(AF_INET, SOCK_STREAM, 0);
	if(*TCP_socketfd < 0) {
		perror("can't open stream socket");
		exit(-1);
	}else
		printf("Internet stream socket FD#%d\n", *TCP_socketfd);

	/* bind the local address, so that the cliend can send to server */
	memset((char*)&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family		= AF_INET;
	serv_addr.sin_addr.s_addr	= htonl(INADDR_ANY);
	serv_addr.sin_port			= htons(SERV_TCP_PORT);

	ret = bind(*TCP_socketfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
	if(ret < 0) {
		perror("can't bind local address");
		exit(-1);
	}

	/* listen to the socket */
	ret = listen(*TCP_socketfd, 5);
	if(ret < 0) {
		perror("listen()");
		exit(-1);
	}

	exit(0);
}

int AcceptNewSockfd(int TCP_socketfd, int *newsockfd)
{

	struct sockaddr_in	client_addr;
	socklen_t			clilen = sizeof(struct sockaddr_in);

	*newsockfd = accept(TCP_socketfd, (struct sockaddr *) &client_addr, &clilen);
	if (*newsockfd == -1) {
		perror("accept()");
		exit(-1);
	} else
		printf("Accpet client come from [%s:%u] by FD#%d\n",
			inet_ntoa(client_addr.sin_addr),
			ntohs(client_addr.sin_port),
			*newsockfd);

	exit(0);
}

int ReceiveData(int FD_index, int* clean_FD, char* pRevdata, int Recv_len)
{
	int	Recved_len;

	memset(pRevdata, 0, Recv_len);
	/* read a message from the client */
#if 1
	Recved_len = read(FD_index, pRevdata, Recv_len);
#else
	Recved_len = recv(FD_index, pRevdata, MAX_SIZE, 0);
#endif
	/* make sure it's a proper string */
	switch(Recved_len)
	{
		case -1:
//			perror("recv()");
			*clean_FD = TRUE;
			break;
		case 0:
//			perror("Client disconnect");
			*clean_FD = TRUE;
			break;
		default:
			printf("Receive: len=[%d] pRevdata=[%s] from FD#%d\n", Recved_len, pRevdata, FD_index);
			*clean_FD = FALSE;
			if(Recved_len != Recv_len)
				printf("Recv_len: %d, but the Recved_len: %d", Recv_len, Recved_len);
			break;
		}

	return Recved_len;
}

int SendData(int FD_index, char* pSendData, int Send_len)
{
	int sent_len;

	/* write a message to the server */
	printf("send [%s] by FD#%d\n", pSendData, FD_index); //dbg
#if 1
	sent_len = write(FD_index, pSendData, Send_len);
#else
	sent_len = send(FD_index, pSendData, Send_len, 0);
#endif
	if(sent_len != Send_len)
		printf("Send_len: %d, but the sent_len: %d", Send_len, sent_len);

	return sent_len;
}

int main(int argc, char *argv[])
{
	int		TCP_socketfd, max_fd, newsockfd;
	int		ret, FD_index, clean_FD, Recved_len, sent_len;
	fd_set	active_fd_set, read_fds;
	struct timeval	tv;
	char	pRevdata[MAX_SIZE], pSendData[MAX_SIZE];

	ret = TCPsocket_bind_listen(&TCP_socketfd);
	if(ret<0)	exit(ret);

	//Initializes the file descriptor set fdset to have zero bits for all file descriptors.
	FD_ZERO(&active_fd_set);
	//Sets the bit for the file descriptor fd in the file descriptor set fdset.
	FD_SET(TCP_socketfd, &active_fd_set);
	max_fd = TCP_socketfd;

	for(;;) {
		/* Set timeout */
		tv.tv_sec	= 2;
		tv.tv_usec	= 0;

		read_fds = active_fd_set;

		ret = select(max_fd + 1, &read_fds, NULL, NULL, &tv);
		switch(ret)
		{
			case -1:
//				perror("select()");
				exit(-1);
				break;
			case 0:
//				printf("select timeout\n");
				continue;
			default:
				/* Service all sockets */
				for (FD_index = 0; FD_index < max_fd+1; FD_index++) {	//FD_SETSIZE:1024
					//Returns a non-zero value if the bit for the file descriptor fd is set in the file descriptor set by fdset, and 0 otherwise.
					if (FD_ISSET(FD_index, &read_fds)) {
						if (FD_index == TCP_socketfd) {
							/* Connection request on original socket. */
							AcceptNewSockfd(TCP_socketfd, &newsockfd);
							/* Add to fd set */
							FD_SET(newsockfd, &active_fd_set);
							/* update max_fd */
							if (newsockfd > max_fd)
								max_fd = newsockfd;

						} else { // FD_index!=TCP_socketfd
							/* Data arriving on an already-connected socket */
							Recved_len = ReceiveData(FD_index, &clean_FD, pRevdata, MAX_SIZE);

							if(0 == clean_FD){
								sprintf(pSendData, "%s from Server", pRevdata);
								sent_len = SendData(FD_index, pSendData, MAX_SIZE);
							}else{
								printf("FD#%d disconnect\n", FD_index);
								/* Clean up */
								close(FD_index);
								//Clears the bit for the file descriptor fd in the file descriptor set fdset.
								FD_CLR(FD_index, &active_fd_set);
							}
						} //if(FD_index == TCP_socketfd)
					} //if (FD_ISSET(FD_index, &read_fds))
				} //for (FD_index = 0; FD_index < FD_SETSIZE; FD_index++)
				break;
		} //switch(ret)
	} //for(;;)
	exit(0);
}
