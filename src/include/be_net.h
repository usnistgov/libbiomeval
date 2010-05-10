/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef _BE_NET_H
#define _BE_NET_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <biomeval.h>

/*
 * Network-related defines.
 */

#define BE_MAX_SERVER_CONNECTIONS	8
#define BE_MAX_COMM_BUF_LEN		4096
#define BE_MAX_COMM_STRING_LEN		255

/******************************************************************************/
/* The common commands supported by the library. Applications can             */
/* add their own commands as needed.                                          */
/******************************************************************************/

/*
 * The start of numbering for commands define by users of this package.
 */
#define BE_NETCMD_USERAREA		33
/*
 * Command:	Ping
 * Response:	OK
 */
#define BE_NETCMD_PING			1

/*
 * Command:	Put
 * Data:	<Filename>
 *		<File length>
 *		<File data ... >
 * Response:	OK, Error
 */
#define BE_NETCMD_PUT			2

/*
 * Command:	Bye
 * Response:	None
 */
#define BE_NETCMD_BYE			3

/******************************************************************************/
/* The common responses to the commands.                                      */
/******************************************************************************/

/*
 * Response:	OK
 */
#define BE_NETRSP_OK			0

/*
 * Response:	Error
 * Data:	<Information string>
 */
#define BE_NETRSP_ERROR			1

/*
 * Functions to map command and response strings to their numeric values.
 * Strings are compared case-insensitve.
 *
 * Parameters:
 *	cmd/resp	The command or reponse string
 * Return:
 *	The integer representing the command/response
 *	-1 if invalid command/response
 */
int
beMapCommand(const char *cmd);

int
beMapResponse(const char *resp);

/*
 * Declare the interface for the network functionality to be used by
 * the biometric evaluation SDK drivers.
 */

/*
 * Create a TCP socket connection to the named host. This is used by
 * clients.
 *
 * Parameters:
 *	host	The host name of the server
 *	port	The port the server is listening on
 * Return:
 *	The socket descriptor
 *	-1 on failure
 */
int
beClientConnect(char *host, int port);

/*
 * Create a TCP socket connection to listen on. This is used by servers.
 *
 * Parameters:
 *	port	The port the server is to listen on
 * Return:
 *	The socket descriptor
 *	-1 on failure
 */
int
beServerListen(int port);

/*
 * Accept a a connection on a listening socket.
 *
 * Parameters:
 *	listenfd	The socket the server is listening on
 * Return:
 *	The socket descriptor of the accepted connection.
 *	-1 on failure
 */
int
beServerAccept(int listenfd);

/*
 * Close a TCP socket connection.
 *
 * Parameters:
 *	sockfd	The descriptor of the socket to be closed.
 * Return:
 *	0	Success
 *	-1	Failure
 */
int
beSocketClose(int sockfd);

/*
 * Read a line of text from the open socket. A line of text is one that
 * is terminated by a line-feed character, and is less than
 * BE_MAX_COMM_BUF_LEN bytes in length. On success, the returned buffer
 * will contain the string that was read, terminated with a nul character.
 *
 * Parameters:
 *	sockfd	The descriptor of the socket to be read.
 *	str	The buffer for the line that was read.
 * Return:
 *	0	End-of-file
 *	-1	Failure
 *	other	Length of the data read
 */
int
beSocketReadString(int sockfd, char *str);

/*
 * Write a line of text to the open socket. A line of text is one that
 * is terminated by a line-feed character, and is less than
 * BE_MAX_COMM_BUF_LEN bytes in length. This function takes a traditional
 * C string and writes it to the socket, adding the line-feed.
 *
 * Parameters:
 *	sockfd	The descriptor of the socket to be written.
 *	str	The buffer for the string to write.
 * Return:
 *	0	End-of-file
 *	-1	Failure
 */
int
beSocketWriteString(int sockfd, char *str);

/*
 * Send a file to the server.
 *
 * Parameters:
 *	sockfd		The descriptor of the socket to be written.
 *	filename	The name of the file to send.
 * Return:
 *	0	End-of-file
 *	-1	Failure
 */
int
beSendFile(int sockfd, char *filename);

/******************************************************************************/
/* Functions to handle the common commands supported by this library.         */
/******************************************************************************/

/*
 * Process a Ping command by responding over the given socket.
 *
 * Parameters:
 *	sockfd	The descriptor of the socket to be read.
 * Return:
 *	0	Success
 *	-1	Failure
 */
int
beProcessPing(int sockfd);

/*
 * Process a Put command by reading the data to be saved, including the file
 * name, and responding with a status string, over the given socket.
 *
 * Parameters:
 *	sockfd	The descriptor of the socket to be read.
 * Return:
 *	0	Success
 *	-1	Failure
 */
int
beProcessPut(int sockfd);


#endif /* _BE_NET_H */
