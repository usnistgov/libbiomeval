/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef BE_WINDOWSFIXUP_H_
#define BE_WINDOWSFIXUP_H_

#ifdef _WIN32

#include <windows.h>

using mode_t = unsigned int;

/* 1: arpa/inet.h equivalent for htons, etc. */
#include <winsock.h>

/* 2: use windows basename(3)/dirname(3) */
char* basename(const char*);
char* dirname(const char*);

/* 3: unistd.h */
#include <io.h>

/* Windows only implements localtime and localtime_s, but _s is better */
struct tm*
localtime_r(
    const time_t* clock,
    struct tm* result);

/* 4: Mainly in windows.h */

/*
 * 5: No dirent on Windows. Use open-source version compiled in NBIS.
 *    Also contains some #defines usually found in sys/stat.h.
 */
#include <be_dirent_windows.h>
#ifndef S_IRWXU
#define S_IRWXU S_IRUSR | S_IWUSR | S_IXUSR
#endif
#ifndef S_IRWXG
#define S_IRWXG S_IRGRP | S_IWGRP | S_IXGRP
#endif
#ifndef S_IRWXO
#define S_IRWXO S_IROTH | S_IWOTH | S_IXOTH
#endif

#ifndef F_OK
#define F_OK 00
#endif

#ifndef R_OK
#define R_OK 04
#endif

#ifndef W_OK
#define W_OK 02
#endif

/* 6: mkdir() */
#include <direct.h>
/* ...except Windows mkdir doesn't take a mode_t */
int mkdir(const char*, mode_t);
int mkstemp(char*);

/* 7: Symlinks aren't quite the same on Windows */
int lstat(const char*, struct stat*);

/* 8: Missing or alternate names with required C linkage for NBIS */

#ifdef __cplusplus
extern "C" {
#endif

int strncasecmp(const char* s1, const char* s2, size_t n);
char* index(const char* s, int c);

/* 9: No gettimeofday */
int gettimeofday(struct timeval*, struct timezone*);

/* 10: Skipping for now (sys/wait.h) */

#ifdef __cplusplus
}
#endif

#else

/* 1 */
#include <arpa/inet.h>

/* 2 */

#include <libgen.h>	/*  for basename(3) and dirname(3) */
#ifdef basename		/* GNU has this macro irresponsibly defined */
#undef basename
#endif

/* 3 */
#include <unistd.h>

/* 4 */
#include <sys/param.h>

/* 5 */
#include <dirent.h>

/* 9 */
#include <sys/time.h>

/* 10 */
#include <sys/wait.h>

#endif /* _WIN32 */

#endif /* BE_WINDOWSFIXUP_H_ */
