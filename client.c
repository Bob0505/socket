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

int main(int argc, char *argv[])
{
	int		sockfd;
	struct sockaddr_in serv_addr;
	char	*serv_host = "localhost";
	struct hostent *host_ptr;
	int		port;
	int		buff_size = 0;
//<asus-bob+>
	char	msg[200];
	int		getdata;
	int		len;
//<asus-bob+>

  /* command line: client [host [port]]*/
	if(argc >= 2) 
		serv_host = argv[1]; /* read the host if provided */
	if(argc == 3)
		sscanf(argv[2], "%d", &port); /* read the port if provided */
	else 
		port = SERV_TCP_PORT;

	/* get the address of the host */
	if((host_ptr = gethostbyname(serv_host)) == NULL) {
		printf("gethostbyname error");
		exit(1);
	}
  
	if(host_ptr->h_addrtype !=  AF_INET) {
		printf("unknown address type");
		exit(1);
	}

	/* address */
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = 
		((struct in_addr *)host_ptr->h_addr_list[0])->s_addr;
	serv_addr.sin_port = htons(port);

	/* open a TCP socket */
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("can't open stream socket");
		exit(1);
	}
	printf("sockfd: %d\n", sockfd);	//dbg

	/* connect to the server */    
	if(connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		printf("can't connect to server");
		exit(1);
	}
//<asus-bob+>
	for(;;) {
		scanf("%d", &getdata);

		if(0!=getdata)
		{
			/* write a message to the server */
			sprintf(msg, "Hi Bob-%d!!", getdata);
			printf("send [%s]\n", msg);	//dbg
			write(sockfd, msg, sizeof(msg));

			/* read a message from the client */
			len = read(sockfd, msg, MAX_SIZE); 
			/* make sure it's a proper string */
			msg[len] = 0;
			printf("%s\n", msg);

			getdata=0;
		}
	}
	close(sockfd);
//  write(sockfd, "hello world", sizeof("hello world"));
//<asus-bob->
}


//			strcpy(msg,"Hi Bob!!");
