/* Copyright 2006 Sun Microsystems, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons
 * to whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT
 * OF THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * HOLDERS INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL
 * INDIRECT OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING
 * FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Except as contained in this notice, the name of a copyright holder
 * shall not be used in advertising or otherwise to promote the sale, use
 * or other dealings in this Software without prior written authorization
 * of the copyright holder.
 */ 

/*
 * Generated by dtrace(1M), and then modified for backwards compatibility
 * with older versions of dtrace.   Used if dtrace -h fails.
 * (Since _ENABLED support was added after dtrace -h, this  assumes if 
 *  dtrace -h fails, _ENABLED will too.)
 */

#ifndef	_XSERVER_DTRACE_H
#define	_XSERVER_DTRACE_H

#include <unistd.h>

#ifdef	__cplusplus
extern "C" {
#endif

#define	XSERVER_CLIENT_AUTH(arg0, arg1, arg2, arg3) \
	winDebug("XSERVER_CLIENT_AUTH: %d %s %d %d\n",arg0, arg1, arg2, arg3)
#define	XSERVER_CLIENT_CONNECT(arg0, arg1) \
	winDebug("XSERVER_CLIENT_CONNECT: %d %d\n",arg0, arg1)
#define	XSERVER_CLIENT_DISCONNECT(arg0) \
	winDebug("XSERVER_CLIENT_DISCONNECT: %d %d\n",arg0)
#define	XSERVER_REQUEST_DONE(arg0, arg1, arg2, arg3, arg4) \
	winDebug("XSERVER_REQUEST_DONE: %s %d %d %d %d\n",arg0, arg1, arg2, arg3, arg4)
#define	XSERVER_REQUEST_START(arg0, arg1, arg2, arg3, arg4) \
	winDebug("XSERVER_REQUEST_START: %s %d %d %d ->%p\n",arg0, arg1, arg2, arg3, arg4)
#define	XSERVER_RESOURCE_ALLOC(arg0, arg1, arg2, arg3) \
	winDebug("XSERVER_RESOURCE_ALLOC: 0x%x 0x%x ->%p %s\n",arg0, arg1, arg2, arg3)
#define	XSERVER_RESOURCE_FREE(arg0, arg1, arg2, arg3) \
	winDebug("XSERVER_RESOURCE_FREE: 0x%x 0x%x ->%p %s\n",arg0, arg1, arg2, arg3)
#define	XSERVER_SEND_EVENT(arg0, arg1, arg2) \
	winDebug("XSERVER_SEND_EVENT: 0x%x 0x%x ->%p\n",arg0, arg1, arg2)

#define	XSERVER_CLIENT_AUTH_ENABLED() (1)
#define	XSERVER_CLIENT_CONNECT_ENABLED() (1)
#define	XSERVER_CLIENT_DISCONNECT_ENABLED() (1)
#define	XSERVER_REQUEST_DONE_ENABLED() (1)
#define	XSERVER_REQUEST_START_ENABLED() (1)
#define	XSERVER_RESOURCE_ALLOC_ENABLED() (1)
#define	XSERVER_RESOURCE_FREE_ENABLED() (1)
#define	XSERVER_SEND_EVENT_ENABLED() (1)

#ifdef	__cplusplus
}
#endif

#endif	/* _XSERVER_DTRACE_H */