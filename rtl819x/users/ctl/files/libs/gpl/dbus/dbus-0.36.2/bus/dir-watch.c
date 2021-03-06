/* -*- mode: C; c-file-style: "gnu" -*- */
/* dir-watch.c  OS specific directory change notification for message bus
 *
 * Copyright (C) 2003 Red Hat, Inc.
 *
 * Licensed under the Academic Free License version 2.1
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <config.h>

#ifdef DBUS_BUS_ENABLE_DNOTIFY_ON_LINUX 
#define _GNU_SOURCE
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#endif /* DBUS_BUS_ENABLE_DNOTIFY_ON_LINUX */

#include <dbus/dbus-internals.h>
#include "dir-watch.h"


/* D_NOTIFY is available on Linux 2.4 or greater - the actual SIGIO signal is handled in main.c:signal_handler() */
#ifdef DBUS_BUS_ENABLE_DNOTIFY_ON_LINUX 

#define MAX_DIRS_TO_WATCH 128

/* use a static array to avoid handling OOM */
static int fds[MAX_DIRS_TO_WATCH];
static int num_fds = 0;

void
bus_watch_directory (const char *dir, void *userdata)
{
  int fd;

  _dbus_assert (dir != NULL);

  if (num_fds >= MAX_DIRS_TO_WATCH )
    {
      _dbus_warn ("Cannot watch config directory '%s'. Already watching %d directories\n", dir, MAX_DIRS_TO_WATCH);
      goto out;
    }

  fd = open (dir, O_RDONLY);
  if (fd < 0)
    {
      _dbus_warn ("Cannot open directory '%s'; error '%s'\n", dir, _dbus_strerror (errno));
      goto out;
    }

  if (fcntl (fd, F_NOTIFY, DN_DELETE|DN_RENAME|DN_MODIFY) == -1)
    {
      _dbus_warn ("Cannot setup D_NOTIFY for '%s' error '%s'\n", dir, _dbus_strerror (errno));
      close (fd);
      goto out;
    }
  
  fds[num_fds++] = fd;
  _dbus_verbose ("Added watch on config directory '%s'\n", dir);

 out:
  ;
}

void 
bus_drop_all_directory_watches (void)
{
  _dbus_verbose ("Dropping all watches on config directories\n");

  int i;
  
  for (i = 0; i < num_fds; i++)
    {
      if (close (fds[i]) != 0)
	{
	  _dbus_verbose ("Error closing fd %d for config directory watch\n", fds[i]);
	}
    }
  
  num_fds = 0;
}

#else /* fallback to NOP */

void 
bus_drop_all_directory_watches (void)
{
}

void
bus_watch_directory (const char *dir, void *userdata)
{
}

#endif
