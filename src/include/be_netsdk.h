/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef _BE_NETSDK_H
#define _BE_NETSDK_H

#include <inttypes.h>

/*****************************************************************************/
/* Commands that are unique to SDK testing. These commands are sent over the */
/* BiomEval communication facility.                                          */
/*****************************************************************************/

/*
 * Command:	Start
 * Response:	OK, Error
 */
#define BE_NETSDKCMD_START		(BE_NETCMD_USERAREA)

/*
 * Command:	Match
 * Response:	OK, Error
 */
#define BE_NETSDKCMD_MATCH		(BE_NETCMD_USERAREA + 1)

/*
 * Command:	Extract
 * Response:	OK, Error
 */
#define BE_NETSDKCMD_EXTRACT		(BE_NETCMD_USERAREA + 2)

/*
 * Command:	Shutdown
 * Response:	OK
 */
#define BE_NETSDKCMD_SHUTDOWN		(BE_NETCMD_USERAREA + 3)

/*
 * The socket numbers to be used when communicating with the SDK driver
 * services. The SDK Server socket is used by the main SDK server process
 * to listen for commands such as system status, starting an SDK client
 * process, etc. The SDK Driver Start socket is the number of the first
 * socket used by an SDK driver process. The server will pass the actual
 * socket number to the driver and communicate that back to the client.
 */
#define BE_SDKSOCK_SERVER		9000
#define BE_SDKSOCK_DRIVER_START		9001

/*
 * The functions that the actual SDK driver needs to implement.
 */
int
sdkDriverStartup(int argc, char *argv[], int sockfd, void **stateData);

int
sdkDriverMatch(int sockfd, void *stateData);

int
sdkDriverExtract(int sockfd, void *stateData);

void
sdkDriverShutdown(int sockfd, void *stateData);

#endif	/* _BE_NETSDK_H */
