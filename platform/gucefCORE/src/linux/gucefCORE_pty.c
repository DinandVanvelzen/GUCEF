/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "gucefCORE_pty.h"

#ifdef GUCEF_LINUX_BUILD

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

//#define HAVE_SYSV_PTYS
//#define HAVE_STROPTS_H
//#define HAVE_PTY_H

/*
 * Some OS's don't seem to prototype these functions properly
 */
extern int grantpt( int __fd );
extern int unlockpt( int __fd );
extern char *ptsname( int __fd );
extern int posix_openpt( int flags );

#if ( _POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600 )

int
GUCEF_pty_open( int* fdm, int* fds )
{
    int masterfd, slavefd;
    char *slavedevice;

    /*
     *  O_RDWR = Open the device for both reading and writing. It is usual to specify this flag.
     *  O_NOCTTY = Do not make this device the controlling terminal for the process.
     */
    int masterfd = posix_openpt( O_RDWR | O_NOCTTY );
    if ( masterfd < 0 ) 
    {
        /* fallback to /dev/ptmx if posix_openpt not available */
        masterfd = open( "/dev/ptmx", O_RDWR | O_NOCTTY );
    }

    if ( masterfd == -1 ||
         grantpt (masterfd) == -1 ||
         unlockpt (masterfd) == -1 ||
         (slavedevice = ptsname (masterfd)) == NULL)
        return 1;

    slavefd = open(slavedevice, O_RDWR|O_NOCTTY);
    if (slavefd < 0)
        return 1;

    /* success */
    *fdm = masterfd;
    *fds = slavefd;
    return 0;
}

#elif defined( HAVE_PTY_H )

#include <pty.h>

int
GUCEF_pty_open(int *fdm, int *fds)
{
    if ( -1 == openpty( fdm, fds, NULL, NULL, NULL ) )
    {
        /* something went wrong, no pseudo terminals available? */
        return 1;
    }
    return 0;
}

#else

#if defined( HAVE_SYSV_PTYS ) && defined( HAVE_STROPTS_H )

#include <stropts.h>

static int
ptym_open(char *pts_name)
{
    char	*ptr;
    int		fdm;

    strcpy(pts_name, "/dev/ptmx");
    if ((fdm = open(pts_name, O_RDWR)) < 0)
	return(-1);
    if (grantpt(fdm) < 0) {
	close(fdm);
	return(-1);
    }
    if (unlockpt(fdm) < 0) {
	close(fdm);
	return(-1);
    }
    if ((ptr = ptsname(fdm)) == NULL) {
	close(fdm);
	return(-1);
    }

    strcpy(pts_name, ptr);
    return(fdm);
}

static int
ptys_open(int fdm, char *pts_name)
{
    int		fds;

    if ((fds = open(pts_name, O_RDWR)) < 0) {
	close(fdm);
	return(-1);
    }
    if (ioctl(fds, I_PUSH, "ptem") < 0) {
	close(fdm);
	close(fds);
	return(-1);
    }
    if (ioctl(fds, I_PUSH, "ldterm") < 0) {
	close(fdm);
	close(fds);
	return(-1);
    }

    return(fds);
}

#elif defined( HAVE_BSD_PTYS ) && defined( HAVE_GRP_H )

#include <grp.h>

static int
ptym_open(char *pts_name)
{
    int		fdm;
    char	*ptr1, *ptr2;

    strcpy(pts_name, "/dev/ptyXY");
    for (ptr1 = "pqrstuvwxyzPQRST"; *ptr1 != 0; ptr1++) {
	pts_name[8] = *ptr1;
	for (ptr2 = "0123456789abcdef"; *ptr2 != 0; ptr2++) {
	    pts_name[9] = *ptr2;

	    if ((fdm = open(pts_name, O_RDWR)) < 0) {
		if (errno == ENOENT)
		    return(-1);
		else
		    continue;
	    }

	    pts_name[5] = 't';
	    return(fdm);
	}
    }

    return(-1);
}

static int
ptys_open(int fdm, char *pts_name)
{
    struct group	*grptr;
    int			gid, fds;

    gid = (grptr = getgrnam("tty")) ? grptr->gr_gid : -1;
    chown(pts_name, getuid(), gid);
    chmod(pts_name, S_IRUSR | S_IWUSR | S_IWGRP);

    if ((fds = open(pts_name, O_RDWR)) < 0) {
	close(fdm);
	return(-1);
    }

    return(fds);
}

#else

static int
ptym_open(char *pts_name)
{
    return(-1);
}

static int
ptys_open(int fdm, char *pts_name)
{
    return(-1);
}

#endif

int
GUCEF_pty_open(int *fdm, int *fds)
{
    char	pts_name[20];

    if ((*fdm = ptym_open(pts_name)) < 0)
	return 1;

    if ((*fds = ptys_open(*fdm, pts_name)) < 0) {
	close(*fdm);
	return 1;
    }

    return 0;
}

#endif

#endif /* GUCEF_LINUX_BUILD ? */
