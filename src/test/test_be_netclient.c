/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

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
	int sockfd;
	int ret;
	char buf[BE_MAX_COMM_STRING_LEN];
	size_t len;

	//sockfd = beClientConnect("amsterdam", 9999);
	//sockfd = beClientConnect("tomservo", 9999);
	sockfd = beClientConnect("localhost", 9999);
	printf("Result of beClientConnect() is %d\n", sockfd);
	ret = beSocketWriteString(sockfd, "Message One");
	ret = beSocketWriteString(sockfd, "Message Two");
	beSocketWriteString(sockfd, BE_NETCMD_PING);
	beSocketReadString(sockfd, buf);
	printf("Received [%s] after ping.\n", buf);

	ret = beSendFile(sockfd, "testfile");
	printf("beSendFile() returned %d.\n", ret);

	beSocketWriteString(sockfd, BE_NETCMD_BYE);
	beSocketClose(sockfd);
	exit (EXIT_SUCCESS);
}
