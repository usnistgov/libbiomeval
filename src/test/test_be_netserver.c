/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <sys/types.h>
#include <sys/uio.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <be_net.h>
#include <biomeval.h>

/*
 * 
 */
int
main(int argc, char *argv[])
{
	int listenfd, acceptfd;
	char buf[BE_MAX_COMM_BUF_LEN];
	int len;
	int ret;
	int cmd;
	int cont;

	listenfd = beServerListen(9999);
	printf("Result of beServerListen() is %d\n", listenfd);
	while (1) {
		acceptfd = beServerAccept(listenfd);
		if (acceptfd < 0) {
			printf("Error accepting on socket: %s.\n",
			    strerror(errno));
			return (EXIT_FAILURE);
		}
		cont = 1;
		while (cont) {
			bzero(buf, BE_MAX_COMM_BUF_LEN);
			len = beSocketReadString(acceptfd, buf);
			if (len < 0) {
				printf("Error reading from socket.\n");
				beSocketClose(acceptfd);
				return (EXIT_FAILURE);
			}
			printf("Read %d bytes, [%s]\n", len, buf);
			if (len == 0) {
				printf("Read no data; socket closing.\n");
				beSocketClose(acceptfd);
				cont = 0;
				break;
			}
			cmd = beMapCommand(buf);
			switch (cmd) {
				case BE_NETCMD_PING:
					printf("Client pinged.\n");
					beProcessPing(acceptfd);
					break;
				case BE_NETCMD_PUT:
					ret = beProcessPut(acceptfd);
					printf("processPut returns %d\n", ret);
					if (ret != 0) {
						beSocketClose(acceptfd);
						cont = 0;
						break;
					}
				case BE_NETCMD_BYE:
					printf("Client said bye.\n");
					beSocketClose(acceptfd);
					cont = 0;
					break;
			}
		}
	}
	beSocketClose(listenfd);
	return (EXIT_SUCCESS);
}
