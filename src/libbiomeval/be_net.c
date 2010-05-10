/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#include <be_net.h>
#include <biomeval.h>

/*
 * 
 */
int
beMapCommand(const char *cmd)
{
	if (strncasecmp(cmd, "Ping", 4) == 0)
		return (BE_NETCMD_PING);
	if (strncasecmp(cmd, "Put", 3) == 0)
		return (BE_NETCMD_PUT);
	if (strncasecmp(cmd, "Bye", 3) == 0)
		return (BE_NETCMD_BYE);
	return (-1);
}


int
beMapResponse(const char *resp)
{
	if (strncasecmp(resp, "OK", 2) == 0)
		return (BE_NETRSP_OK);
	if (strncasecmp(resp, "Error", 5) == 0)
		return (BE_NETRSP_ERROR);
	return (-1);
}

/*
 * 
 */
int
beClientConnect(char *server, int port)
{
	int sockfd;
	int ret;
	struct hostent *host;
	struct sockaddr_in saddr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		return (-1);

	host = gethostbyname(server);
	if (host == NULL)
		return (-1);
	
	bzero((char *)&saddr, sizeof(saddr));
	bcopy(host->h_addr, (char *)&saddr.sin_addr, host->h_length);
	saddr.sin_family = host->h_addrtype;
	saddr.sin_port = htons(port);
	ret = connect(sockfd, (struct sockaddr *)&saddr, sizeof(saddr));
	if (ret != 0) {
		close(sockfd);
		return (-1);
	}
	return (sockfd);
}

/*
 *
 */
int
beServerListen(int port)
{
	int sockfd;
	int ret;
	struct sockaddr_in saddr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		return (-1);

	bzero((char *)&saddr, sizeof(saddr));
	saddr.sin_port = htons(port);
	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = INADDR_ANY;
	ret = bind(sockfd, (struct sockaddr *)&saddr, sizeof(saddr));
	if (ret != 0) {
		close(sockfd);
		return (-1);
	}
	ret = listen(sockfd, BE_MAX_SERVER_CONNECTIONS);
	if (ret != 0) {
		close(sockfd);
		return (-1);
	}
	return (sockfd);
}

/*
 *
 */
int
beServerAccept(int listenfd)
{
	struct sockaddr caddr;
	socklen_t caddrlen;
	int acceptfd;

	caddrlen = sizeof(caddr);
	acceptfd = accept(listenfd, (struct sockaddr *)&caddr, &caddrlen);
	if (acceptfd < 0) {
		//XXX log the error
		return (-1);
	} else {
		return (acceptfd);
	}
}

/*
 *
 */
int
beSocketClose(int sockfd)
{
	return (close(sockfd));
}

/*
 *
 */
int
beSocketReadString(int sockfd, char *str)
{
	char c;
	int idx;
	
	/*
	 * Read a character at a time from the socket, until we run out
	 * of space or encounter a newline. At that point, terminate the
	 * buffer with a nul and return.
	 */
	idx = 0;
	while (read(sockfd, &c, 1) &&
	    (c != EOF) && (idx < BE_MAX_COMM_STRING_LEN - 1)) {
		if (c < 0)
			return (-1);

 		if (c == '\r')		/* throw away carriage returns */
			continue;

		if (c == '\n') {
        		str[idx] = '\0';
        		break;
		}
		str[idx++] = c;
	}
	if (c == EOF)
		return (0);
	else
		return (idx);
}

/*
 *
 */
int
beSocketWriteString(int sockfd, char *str)
{

	int len, idx;
	char *buf;

	buf = (char *)malloc(BE_MAX_COMM_BUF_LEN);
	if (buf == NULL) {
		//XXX log the error
		return (-1);
	}
	len = strlen(str);
	if (len > BE_MAX_COMM_STRING_LEN) {
		//XXX log the error
		goto err_out;
	}

	/* Copy the source string and add the line-feed terminator. The copy
	 * is done so we can perform a single write and avoid the case where
	 * the string is written but the separate write of the new-line fails.
	 */
	strncpy(buf, str, len);
	buf[len] = '\n';

	/* Write the string to the socket, checking for errors. */
	if (write(sockfd, buf, len + 1) == -1) {
		//XXX log the error
		goto err_out;
	}

	free(buf);
	return (0);
err_out:
	free(buf);
	return (-1);
}

/*
 *
 */
int
beProcessPing(int sockfd)
{
	return (beSocketWriteString(sockfd, "OK"));
}

/*
 *
 */
int
beSendFile(int sockfd, char *filename)
{

	char *buf;
	FILE *fp;
	struct stat sb;
	size_t len;

	buf = (char *)malloc(BE_MAX_COMM_BUF_LEN);
	if (buf == NULL) {
		//XXX log the error
		return (-1);
	}
	fp = fopen(filename, "rb");
	if (fp == NULL) {
		//XXX log the error
		goto err_out;
	}

	stat(filename, &sb);
	sprintf(buf, "%u", sb.st_size);
	beSocketWriteString(sockfd, "Put");
	beSocketWriteString(sockfd, filename);
	beSocketWriteString(sockfd, buf);		/* The file length */
	while (1) {
		len = fread(buf, 1, BE_MAX_COMM_BUF_LEN, fp);
		if (len == 0)
			break;
		len = write(sockfd, buf, len);
		if (len == -1) {
			//XXX log the error
			goto err_out;
		}
	}
	fclose(fp);
	beSocketReadString(sockfd, buf);
	if (beMapResponse(buf) != BE_NETRSP_OK) {
		//XXX log the error
		goto err_out;
	}
	free(buf);
	return (0);
err_out:
	free(buf);
	return (-1);
}

/*
 *
 */
int
beProcessPut(int sockfd)
{
	char fn[BE_MAX_COMM_STRING_LEN];
	char *buf;
	FILE *outfp;
	ssize_t len;
	size_t outsz, outlen, todo;
	struct rlimit rl;

	outfp = NULL;
	buf = (char *)malloc(BE_MAX_COMM_BUF_LEN);
	if (buf == NULL) {
		//XXX log the error
		return (-1);
	}
	/* Obtain the file name */
	if (beSocketReadString(sockfd, fn) == -1)
		//XXX log the error
		goto err_out;

	/* Obtain the file length */
	if (beSocketReadString(sockfd, buf) == -1)
		//XXX log the error
		goto err_out;

	outlen = (size_t)strtoimax(buf, NULL, 10);
	getrlimit(RLIMIT_FSIZE, &rl);
	if (outlen > rl.rlim_cur) {
	//XXX log the error
		goto err_out;
	}

	/* Open the new file */
	outfp = fopen(fn, "wb");
//XXX if this open fails, we probably should read off the file data
//XXX and throw it away anyway
	if (outfp == NULL)
		goto err_out;

	/*
	 * XXX Need to decide what to do when a write fails, possibly
	 * when a file is too large, or the disk is full. One possibility
	 * is to close the socket here, or let the caller do that. Either
	 * way, we have to clean up after ourselves; delete the file if 
	 * any block of it fails to write.
	 */
	/* Read the file data from the socket and write to file. */
	while (1) {
		if (outlen < BE_MAX_COMM_BUF_LEN)
			todo = outlen;
		else
			todo = BE_MAX_COMM_BUF_LEN;
		len = read(sockfd, buf, todo); 
		if (len == 0)		/* Read EOF */
			goto err_out;
		if (len == -1)
			goto err_out;
		outsz = fwrite(buf, 1, len, outfp);
		if (outsz != len)
			goto err_out;
		outlen -= len;
		if (outlen <= 0)
			break;
	}

	/* Close the file and send status back over the socket. */
	fclose(outfp);
	free(buf);
	return (beSocketWriteString(sockfd, "OK"));
err_out:
	//XXX delete the file?
	if (outfp != NULL)
		fclose(outfp);
	free(buf);
	return (-1);
}
