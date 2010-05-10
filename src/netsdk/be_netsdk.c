/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <errno.h>
#include <inttypes.h>
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <be_net.h>
#include <be_netsdk.h>

/*
 * The global pointer to the driver's state data.
 */
static void *driverState;

/*
 * Command parsing.
 */
static int
be_netSdkMapCommand(const char *cmd)
{
	if (strncasecmp(cmd, "Start", 5) == 0)
		return (BE_NETSDKCMD_START);
	if (strncasecmp(cmd, "Match", 5) == 0)
		return (BE_NETSDKCMD_MATCH);
	if (strncasecmp(cmd, "Extract", 7) == 0)
		return (BE_NETSDKCMD_EXTRACT);
	if (strncasecmp(cmd, "Shutdown", 8) == 0)
		return (BE_NETSDKCMD_SHUTDOWN);
	return (beMapCommand(cmd));
}

/*
 * Command processing.
 */
static int
processStart(int sockfd)
{
char *args[2];
args[0] = malloc(32);
args[1] = malloc(32);
strcpy(args[0], "probes");
strcpy(args[1], "gallery");
	if (sdkDriverStartup(2, args, sockfd, &driverState) == 0)
		return (beSocketWriteString(sockfd, "OK"));
	else
		return (beSocketWriteString(sockfd, "Error"));
}

static int
processMatch(int sockfd)
{
	if (sdkDriverMatch(sockfd, driverState) == 0)
		return (beSocketWriteString(sockfd, "OK"));
	else
		return (beSocketWriteString(sockfd, "Error"));
}

static int
processExtract(int sockfd)
{
	if (sdkDriverExtract(sockfd, driverState) == 0)
		return (beSocketWriteString(sockfd, "OK"));
	else
		return (beSocketWriteString(sockfd, "Error"));
}

static int
processShutdown(int sockfd)
{
printf("She's pumping mud! Shutting down!\n");
	sdkDriverShutdown(sockfd, driverState);
	return (beSocketWriteString(sockfd, "OK"));
}

/*
/*
 * Signal handlers.
 */

/* SIGINT and SIGTERM */
static void
sig_int(int signo)
{
}

/* SIGCHLD */
static void
sig_chld(int signo)
{
}

static int
set_sig_handlers()
{
	struct sigaction siga;

	siga.sa_handler = sig_int;
	sigemptyset(&siga.sa_mask);
	siga.sa_flags = 0;
	if (sigaction(SIGINT, &siga, NULL) != 0 ) {
		// XXX log the error
		return (-1);
	}
	if (sigaction(SIGTERM, &siga, NULL) != 0 ) {
		// XXX log the error
		return (-1);
	}
	siga.sa_handler = sig_chld;
	if (sigaction(SIGCHLD, &siga, NULL) != 0 ) {
		// XXX log the error
		return (-1);
	}
}

static void
usage(char *progname)
{
	fprintf(stderr, "%s -p <port>\n", progname);
	exit (EXIT_FAILURE);
}

/*
 * The main processing loop, reading commands from the socket, processing
 * those commands, and quitting when told to do so.
 */
static int
process(int port)
{
	int listenfd, acceptfd;
	int accepting, talking;
	char *buf;

	buf = (char *)malloc(BE_MAX_COMM_BUF_LEN);
	if (buf == NULL) {
		//XXX log error
		return (-1);
	}

	listenfd = beServerListen(port);
	if (listenfd < 0) {
		//XXX log the error
		return (-1);
	}
	//XXX log the success

	accepting = 1;		/* This will go false when shutdown */
	while (accepting) {
		acceptfd = beServerAccept(listenfd);	/* block and wait */
		if (acceptfd < 0) {
			//XXX log the error
		}
		talking = 1;
		while (talking) {
			int len, ret, cmd;
			bzero(buf, BE_MAX_COMM_BUF_LEN);
			len = beSocketReadString(acceptfd, buf);
			if (len == 0) {
				//XXX log the read error
				beSocketClose(acceptfd);
				talking = 0;
				break;
			}
			cmd = be_netSdkMapCommand(buf);
			switch (cmd) {
				case BE_NETCMD_PING:
					beProcessPing(acceptfd);
					break;
				case BE_NETCMD_PUT:
					ret = beProcessPut(acceptfd);
					if (ret != 0) {
						beSocketClose(acceptfd);
						talking = 0;
					}
					break;
				case BE_NETCMD_BYE:
					beSocketClose(acceptfd);
					talking = 0;
					break;
				case BE_NETSDKCMD_START:
					processStart(acceptfd);
					break;
				case BE_NETSDKCMD_MATCH:
					processMatch(acceptfd);
					break;
				case BE_NETSDKCMD_EXTRACT:
					processExtract(acceptfd);
					break;
				case BE_NETSDKCMD_SHUTDOWN:
					processShutdown(acceptfd);
					accepting = talking = 0;
					break;
				default:
					beSocketWriteString(acceptfd,
					    "Unknown Command");
					break;
			}
		}
	}
	beSocketClose(listenfd);
	return (0);
}

/*
 * main() is implemented here, which will start the listen on the socket,
 * and fork off the SDK driver when commands are received on the socket.
 */

int
main(int argc, char *argv[])
{
	int port;
	char ch;
	int p_opt;
	int ret;

	p_opt = 0;
	while ((ch = getopt(argc, argv, "p:")) != -1) {
		switch (ch)  {
			case 'p':
				port = (int)strtol(optarg, NULL, 10);
				/* Make sure it's numerical & positive */
				if ((port == 0 && errno == EINVAL) ||
				    (port < 0))
					usage(argv[0]);
				p_opt = 1;
				break;
			default:
				usage(argv[0]);
				break;
		}
	}
	if (p_opt == 0)
		usage(argv[0]);

	ret = process(port);

	if (ret == 0)
		exit (EXIT_SUCCESS);
	else
		exit (EXIT_FAILURE);
}
