/* boot.c -- configuration file parsing, management of global state.

   Copyright (C) 2012 2013 meng shi.  
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>

#include <netdb.h>
#include <arpa/inet.h>

#include "inner.h"

/*
typedef union
{
  FILE *file;
  const char *text;
} glctx;
*/

static void
ndl_putserv (ndl_sd nsd, 
         struct in_addr ia)
{
  int i;
  struct serv *s;

  for (i = 0; i < nsd->servers; i++)
    {
      if (nsd->server[i].addr.s_addr == ia.s_addr)
        {
          ndl_printb (nsd, 0, -1, "duplacte nameserver %s ignored", inet_ntoa (ia));
          return;
        }
    }
   
  if (nsd->servers >= SERVERS)
    {
      ndl_printd (nsd, 0, -1, "too many nameservers, ignoring %s", inet_ntoa (ia));
      return;
    }
  
  s = nsd->server + nsd->servers;
  s->addr = ia;
  nsd->servers++;  
  ndl_printw (nsd, 0, -1, "number of servers:%d and address:%s", nsd->servers, inet_ntoa (s->addr));
}

static int
ndl_initnet (ndl_sd nsd)
{
  struct in_addr ia;
  struct protoent *pe;
  int i;
  
  if (!nsd->servers)
    {
      if (nsd->file)
        fprintf (nsd->file, "no nameservers, using localhost\n");
      ia.s_addr = htonl (INADDR_LOOPBACK);
      ndl_putserv (nsd, ia);   
    }
  
  pe = getprotobyname ("udp");
  
  if (!pe)
    {
      i = ENOPROTOOPT;
      goto j_free;
    }
  
  nsd->protoc.udp = socket (AF_INET, SOCK_DGRAM, pe->p_proto);
  if (nsd->protoc.udp < 0)
    {
      i = errno;
      goto j_free;
    }
  //else
  //ndl_printw (nsd, 0, -1, "fd:%d", nsd->protoc.udp);
  
  i = setnonblock (nsd, nsd->protoc.udp);
  if (i)
    {
      i = errno;
      goto j_close_udp;
      //printf ("r:%d\n", r);
    }
  //else 
   // printf ("r:%d\n", r);
  
  return 0;
   
  j_close_udp:
    close (nsd->protoc.udp);
  j_free:
    free (nsd);
    return i;
}

static void
ndl_puterr (ndl_sd nsd,
        int en)
{
  if (!nsd->conferrno)
    nsd->conferrno = en; 
}
//ndl_deconf
static void
ndl_printcfg (ndl_sd nsd, 
              const char *fn, 
              int lno, 
              const char *format, 
              ...)
{
  va_list vl;
  
  ndl_puterr (nsd, EINVAL);
  
  if (!nsd->file)
    return;

  if (lno == -1)
    fprintf (nsd->file, "ndl: %s: ", fn);
  else
    fprintf (nsd->file, "ndl: %s:%d: ", fn, lno);
  
  va_start (vl, format);
  vfprintf (nsd->file, format, vl);
  va_end (vl);
  
  fputc ('\n', nsd->file);
}

int
setnonblock (ndl_sd nsd,
             int fd)
{
  int f;
  
  f = fcntl (fd, F_GETFL, 0);
  if (f < 0)
    return errno;
  //return flags;
  f |= O_NONBLOCK;
  //return flags; 
  f = fcntl (fd, F_SETFL, f);
  //return flags;
  if (f < 0)
    return errno;  
  //else
   // return flags; 
  return 0;
}

static void
getnameserv (ndl_sd nsd,
             const char *fn,
             int lno,
             const char *buf)
{
  struct in_addr ia;
  if (!inet_aton (buf, &ia))
    {
      ndl_printcfg (nsd, fn, lno, "invaild nameserver address: %s\n", buf);
      return;
    } 

  ndl_printd (nsd, 0, -1, "using name server %s", inet_ntoa (ia));
  ndl_putserv (nsd, ia);
  return;
}

static const struct cmdinfo 
{
  const char *name;
  void (*fn) (ndl_sd nsd, const char *fn, int lno, const char *buf);
} cmdinfos[] =
  {
    {"nameserver",  getnameserv},
#if 0
    { "domain"/*,      getsearch*/ },
    { "search"/*,      getdomain*/ }, 
    { "sortlist"/*,    getslist*/ },
    { "options"/*,     getopts*/  },
#endif
    {0                            } 
  }; 

static void 
ndl_fparse (ndl_sd nsd, 
            const char *filename, 
            int (*getline) (FILE *file, 
                            const char *filename, 
                            int lno, 
                            char *buf, 
                            int buflen), 
            FILE *file) 
{
  char linebuf[2048], *p, *q;
  int lno, l, dirl;
  const struct cmdinfo *cip;
  
  for (lno = 1; (l = getline (file, filename, 
       lno, linebuf, sizeof (linebuf))) != -1; lno++)
    {
      if (l == -2)
        continue;
      while (l > 0 && deblank (linebuf[l-1])) 
        l--;
      linebuf[l] = 0;
      
      p = linebuf;
      while (deblank (*p))
        p++;
      if (*p == '#' || !*p)
        continue;
      q = p;
      
      while (*q && !deblank(*q))
        q++;
      dirl = q - p;
      
      for (cip = cmdinfos; cip->name 
           && !(strlen (cip->name) == dirl && !memcmp (cip->name, p, q-p));
           cip++);
     
      if (!cip->name)
        {
         ndl_printd (nsd, 0, -1, "%s:%d: unknown configuration directive!`%.*s", filename, lno, q-p, p);
          continue; 
        }   
           
      while (deblank (*q))
        q++;
      cip->fn (nsd, filename, lno, q); 
    } 
}

static void 
ndl_readconf (ndl_sd nsd,
          const char *filename)
{
  FILE *file; 
  file = fopen (filename, "r");
  if (!file)
    {
      if (errno == ENOENT)
        {
          ndl_printb (nsd, 0, -1, "configure file %s is does not exist!", filename);
          return; 
        }
      
     ndl_puterr (nsd, errno);
     ndl_printd (nsd, 0, -1, "can not open configuration file `%s': %s", filename, strerror (errno));
     
      return;
    }
  
  ndl_fparse (nsd, filename, getfile, file); 

  fclose(file);
}

static int
ndl_state_load (ndl_sd *nsd,
                FILE *configfile)
{
  ndl_sd in;
  
  in = malloc (sizeof (*in));
  if (!in)
    return errno;
  
  INIT_QUEUE (in->time_wait); 
  INIT_QUEUE (in->child_wait);
  INIT_QUEUE (in->output);
  
  in->servers = in->sortlist = 0;
  in->protoc.udp = in->protoc.tcp = -1;
  in->next_id = 0x311f;
  INIT_CACHE (&in->tcp_write);
  INIT_CACHE (&in->tcp_read);
  
  in->tcp_status = disconnected;
  
  in->file = configfile;
  
  *nsd = in;
  return 0;
} 

int
dnsinit (ndl_sd* nsd,
         FILE *configfile)
{
  ndl_sd in;
  int i; 
  
  i = ndl_state_load (&in, configfile ? configfile : stderr);
  if (i)
    return i;
  
  if (configfile)
    {
      ndl_readconf (in, configfile);
    }
  else
    {
      ndl_readconf (in, "/etc/resolv.conf"); 
    }
  
  i = ndl_initnet (in);
  if (i)
    return i;
  *nsd = in; 

  return 0;
}

/*
static void
pusherr (nsd, errno)
      ndl_sd nsd;
      int errno;
{

}

static void
confparseerr ()
{

}

static int
nextword ()

{

}

static void
getslist ()

{

}

static void
getopts ()

{

}

static void
getclear ()

{

}

static int
gettext ()

{

}

static const char *
getenvir()

{

}

static void
readcfgtext ()

{

}

static void
readcfgenv ()

{

}

static void
readcfgenvtext ()

{

}

static void
initabort ()

{

}

void
dnsquit ()

{

}
*/
