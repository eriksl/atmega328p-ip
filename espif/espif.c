#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <poll.h>
#include <getopt.h>

static int resolve(const char * hostname, int port, struct sockaddr_in6 *saddr)
{
	struct addrinfo hints;
	struct addrinfo *res;
	char service[16];
	int s;

	snprintf(service, sizeof(service), "%u", port);
	memset(&hints, 0, sizeof(hints));

	hints.ai_family		=	AF_INET6;
	hints.ai_socktype	=	SOCK_DGRAM;
	hints.ai_flags		=	AI_NUMERICSERV | AI_V4MAPPED;

	if((s = getaddrinfo(hostname, service, &hints, &res)))
	{
		freeaddrinfo(res);
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		return(0);
	}

	*saddr = *(struct sockaddr_in6 *)res->ai_addr;
	freeaddrinfo(res);

	return(1);
}

static void usage(void)
{
	fprintf(stderr, "usage: espif [options] host string [string...]\n");
	fprintf(stderr, "-C|--conntr    set connect attempts [4]\n");
	fprintf(stderr, "-c|--connto    set connect / send timeout in milliseconds [2000]\n");
	fprintf(stderr, "-r|--recvto1   set initial receive timeout in milliseconds [2000]\n");
	fprintf(stderr, "-R|--recvto2   set subsequent receive timeout in milliseconds [100]\n");
	fprintf(stderr, "-s|--sendtr    set send/receive retries [8]\n");
	fprintf(stderr, "-v|--verbose   enable verbose output on stderr\n");
}

int main(int argc, char ** argv)
{
	static const char *shortopts = "C:c:R:r:s:v";
	static const struct option longopts[] =
	{
		{ "conntr", required_argument, 0, 'C' },
		{ "connto", required_argument, 0, 'c' },
		{ "recvto2", required_argument, 0, 'R' },
		{ "recvto1", required_argument, 0, 'r' },
		{ "sendtr", required_argument, 0, 's' },
		{ "verbose", no_argument, 0, 'v' },
		{ 0, 0, 0, 0 }
	};

	int					fd;
	struct sockaddr_in6	saddr;
	char				buf[128 * 1024];
	ssize_t				buflen;
	struct pollfd		pfd;
	int					attempt;
	int					arg;
	int					current;
	int					verbose = 0;
	int					connto = 2000;
	int					conntr = 4;
	int					recvto1 = 2000;
	int					recvto2 = 100;
	int					sendtr = 8;

	while((current = getopt_long(argc, argv, shortopts, longopts, 0)) != -1)
	{
		switch(current)
		{
			case('C'):
			{
				conntr = atoi(optarg);

				break;
			}

			case('c'):
			{
				connto = atoi(optarg);

				break;
			}

			case('R'):
			{
				recvto2 = atoi(optarg);

				break;
			}

			case('r'):
			{
				recvto1 = atoi(optarg);

				break;
			}

			case('s'):
			{
				sendtr = atoi(optarg);

				break;
			}

			case('v'):
			{
				verbose = 1;
				break;
			}

			default:
			{
				usage();
				exit(1);
			}
		}
	}

	if((argc - optind) <= 0)
	{
		usage();
		exit(1);
	}

	if(!resolve(argv[optind], 23, &saddr))
		exit(1);

	fd = -1;

	for(attempt = conntr; attempt > 0; attempt--)
	{
		if(fd >= 0)
			close(fd);

		if((fd = socket(AF_INET6, SOCK_STREAM | SOCK_NONBLOCK, 0)) < 0)
		{
			if(verbose)
				perror("socket");

			printf("ERROR\n");

			exit(1);
		}

		pfd.fd = fd;
		pfd.events = POLLOUT;

		if(!connect(fd, (const struct sockaddr *)&saddr, sizeof(saddr)) || (errno == EINPROGRESS))
		{
			if(poll(&pfd, 1, connto) != 1)
			{
				if(verbose)
					perror("connect-poll");

				goto retry;
			}

			int so_error = 0;
			socklen_t so_size = sizeof(so_error);

			if(getsockopt(fd, SOL_SOCKET, SO_ERROR, &so_error, &so_size) < 0)
			{
				if(verbose)
					perror("getsockopt(SO_ERROR)");

				goto retry;
			}

			if(so_error != 0)
			{
				if(verbose)
					fprintf(stderr, "socket error: %s\n", strerror(so_error));

				goto retry;
			}

			break;
		}
		else
		{
			if(verbose)
				perror("connect");
		}

retry:
		if(verbose)
			fprintf(stderr, "connect: retry attempt %d\n", attempt);

		usleep(connto * 1000);
	}

	if((attempt == 0) || (fd < 0))
	{
		if(verbose)
			fprintf(stderr, "no more attempts to connect\n");

		close(fd);
		exit(1);
	}

	pfd.fd = fd;

	for(arg = optind + 1; argv[arg]; arg++)
	{
		strncpy(buf, argv[arg], sizeof(buf));
		buf[sizeof(buf) - 1] = '\0';
		strncat(buf, "\r\n", sizeof(buf));
		buf[sizeof(buf) - 1] = '\0';

		for(attempt = sendtr; attempt > 0; attempt--)
		{
			if(verbose)
				fprintf(stderr, "host %s cmd \"%s\" attempt %d\n", argv[1], argv[arg], attempt);

			pfd.events = POLLOUT;

			if(poll(&pfd, 1, connto) != 1)
			{
				if(verbose)
					perror("poll-write");

				continue;
			}

			if(write(fd, buf, strlen(buf)) != strlen(buf))
			{
				if(verbose)
					perror("write");

				continue;
			}

			pfd.events = POLLIN;

			if(poll(&pfd, 1, recvto1) != 1)
			{
				if(verbose)
					perror("poll-read");

				continue;
			}

			if((buflen = read(fd, buf, sizeof(buf))) <= 0)
			{
				if(verbose)
					perror("read");

				continue;
			}

			current = buflen;
			buf[current] = '\0';

			for(;;)
			{
				if(poll(&pfd, 1, recvto2) != 1)
				{
					if(verbose)
						perror("poll-read2");

					break;
				}

				if((buflen = read(fd, buf + current, sizeof(buf) - current)) <= 0)
					break;

				current += buflen;
				buf[current] = '\0';
			}

			fputs(buf, stdout);

			break;
		}

		if(verbose && (attempt == 0))
		{
			fprintf(stderr, "no more attempts\n");
			printf("ERROR\n");
			break;
		}
	}

	close(fd);

	return(0);
}
