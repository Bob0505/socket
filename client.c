#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>

#define SERV_TCP_PORT 8100 //<asus-bob+>8000	/* server's port number */
#define MAX_SIZE 80
#define	exit(X)	return(X)	//<asus-bob+>

int TCPsocket(int *TCP_socketfd, struct sockaddr_in *serv_addr)
{
	struct hostent *host_ptr;
	char	*serv_host = "localhost";


	/* get the address of the host */
	if((host_ptr = gethostbyname(serv_host)) == NULL) {
		perror("gethostbyname error");
		exit(-1);
	}

	if(host_ptr->h_addrtype !=	AF_INET) {
		perror("unknown address type");
		exit(-1);
	}

	/* address */
	memset((char*)serv_addr, 0, sizeof(serv_addr));
	serv_addr->sin_family		= AF_INET;
	serv_addr->sin_addr.s_addr	= ((struct in_addr *)host_ptr->h_addr_list[0])->s_addr;
	serv_addr->sin_port			= htons(SERV_TCP_PORT);

	/* open a TCP socket */
	*TCP_socketfd = socket(AF_INET, SOCK_STREAM, 0);
	if(*TCP_socketfd < 0) {
		perror("can't open stream socket");
		exit(-1);
	}else
		printf("open a TCP socket FD%d\n", *TCP_socketfd); //dbg

	exit(0);
}

int SendData(int TCP_socketfd, char* pSendData, int Send_len)
{
	int sent_len;

	/* write a message to the server */
	printf("send [%s]\n", pSendData); //dbg
#if 1
	sent_len = write(TCP_socketfd, pSendData, Send_len);
#else
	sent_len = send(TCP_socketfd, pSendData, Send_len, 0);
#endif
	if(sent_len != Send_len)
		printf("Send_len: %d, but the sent_len: %d", Send_len, sent_len);

	return sent_len;
}

int ReceiveData(int TCP_socketfd, char* pRevdata, int Recv_len)
{
	int	Recved_len;

	memset(pRevdata, 0, Recv_len);
	/* read a message from the client */
#if 1
	Recved_len = read(TCP_socketfd, pRevdata, Recv_len);
#else
	Recved_len = recv(TCP_socketfd, pRevdata, MAX_SIZE, 0);
#endif
	/* make sure it's a proper string */
	switch(Recved_len)
	{
		case -1:
//			perror("recv()");
			break;
		case 0:
//			perror("Client disconnect");
			break;
		default:
			printf("Receive: len=[%d] pRevdata=[%s]\n", Recved_len, pRevdata);
			if(Recved_len != Recv_len)
				printf("Recv_len: %d, but the Recved_len: %d", Recv_len, Recved_len);
			break;
		}

	return Recved_len;
}


int main(int argc, char *argv[])
{
	int		TCP_socketfd;
	struct sockaddr_in serv_addr;
	int		getdata;
	int		ret;
	char	pSendData[MAX_SIZE], pRevdata[MAX_SIZE];
	int		sent_len, Recved_len;

	TCPsocket(&TCP_socketfd, &serv_addr);

	/* connect to the server */
	ret = connect(TCP_socketfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
	if(ret < 0) {
		perror("can't connect to server");
		exit(-1);
	}

	for(;;) {
		printf("Please enter a number: ");
		scanf("%d", &getdata);

		if(3098==getdata)
			break;

		if(0 != getdata)
		{
			/* write a message to the server */
			sprintf(pSendData, "Hi Bob-%d!!", getdata);
			sent_len = SendData(TCP_socketfd, pSendData, MAX_SIZE);

			/* read a message from the client */
			Recved_len = ReceiveData(TCP_socketfd, pRevdata, MAX_SIZE);

			getdata=0;
		}
	}

	ret = close(TCP_socketfd);
	if(0 != ret)
		printf("close ret:%d\n", ret);	//dbg
}

