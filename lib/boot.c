/* boot.c -- configuration file parsing, initialize the various parameters.

   Copyright (C) 2012 2013 S.meng.  
   
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
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>

#include <netdb.h>
#include <arpa/inet.h>

#include "inside.h"

typedef union 
{
  FILE *file;
} get_text;

static void 
puterr (ndl_sd nsd, int en) 
{
  if (!nsd->conferrno) 
    nsd->conferrno = en;
}

static int 
fgetline (ndl_sd nsd, 
          get_text *src_io, 
          const char *filename,
          int lno, 
          char *buf, 
          int buflen) 
{
  FILE *file = src_io->file;
  int c, i;
  char *p;

  p = buf;
  buflen--;
  i = 0;
    
  for (;;) 
    { 
      if (i == buflen) 
        {
          printd (nsd,-1,0,"%s:%d: line too long, ignored", filename, lno);
          goto j_bad_line;
        }
 
    c = getc(file);
    if (!c) 
      {
        printd (nsd,-1,0,"%s:%d: line contains nul, ignored", filename, lno);
        goto j_bad_line;
      } 
    else if (c == '\n') 
      {
        break;
      } 
    else if (c == EOF) 
      {
        if (ferror (file)) 
          {
	    puterr (nsd, errno);
	    printd (nsd, -1, 0, "%s:%d: read error: %s", filename, lno, strerror(errno));
	    return -1;
          }

        if (!i) 
          return -1;

        break;
      } 
    else 
      {
        *p++ = c;
        i++;
      }
    }

  *p++ = 0;
  return i;

 j_bad_line:
   puterr (nsd, EINVAL);
   while ((c = getc (file)) != EOF && c != '\n');
   return -2;
}

static void 
printcfg (ndl_sd nsd, 
          const char *fn, 
          int lno,
	  const char *fmt, 
          ...) 
{
  va_list al;

  puterr (nsd,EINVAL);

  if (!nsd->defile) 
    return;

  if (lno == -1) 
    fprintf (nsd->defile,"Dns: %s: ", fn);
  else 
    fprintf (nsd->defile,"Dns: %s:%d: ", fn, lno);

  va_start (al,fmt);
  vfprintf (nsd->defile, fmt, al);
  va_end (al);
  fputc ('\n',nsd->defile);
}

static void 
putserv (ndl_sd nsd, 
         struct in_addr addr) 
{
  int i;
  struct server *s;
  
  for (i=0; i < nsd->nservers; i++) 
    {
      if (nsd->servers[i].addr.s_addr == addr.s_addr) 
        {
          printd (nsd, -1, 0, "duplicate nameserver %s ignored",inet_ntoa(addr));
          return;
        }
    }
  
  if (nsd->nservers >= SERVERS) 
    {
      printd (nsd, -1, 0, "too many nameservers, ignoring %s", inet_ntoa(addr));
      return;
    }

  s = nsd->servers+nsd->nservers;
  s->addr = addr;
  nsd->nservers++;
}

static void 
nameserver (ndl_sd nsd, 
            const char *fn, 
            int lno, 
            const char *buf) 
{
  struct in_addr ia;
  
  if (!inet_aton (buf,&ia)) 
  {
    printcfg (nsd, fn, lno, "invalid nameserver address '%s'", buf);
    return;
  }
  printd (nsd, -1, 0, "using nameserver %s", inet_ntoa(ia));
  putserv (nsd,ia);
}

static const struct config 
{
  const char *name;
  void (*fn)(ndl_sd nsd, const char *fn, int lno, const char *buf);
} config_tables[] = {
  CONF_LOAD ("nameserver", nameserver),
  CONF_LOAD (0,0) 
};

static void 
fparse(ndl_sd nsd, 
       const char *filename,
       int (*getline)(ndl_sd nsd, 
                      get_text*,
		      const char *filename, 
                      int lno,
		      char *buf, 
                      int buflen),
       get_text gt) 
{
  char linebuf[2000], *p, *q;
  int lno, l, cmdlen;
  const struct config *cp;

  for (lno = 1;
       (l = getline (nsd, &gt, filename,lno, linebuf, sizeof (linebuf))) != -1;
       lno++) 
    {
      if (l == -2) 
        continue;

      while (l>0 && deblank (linebuf[l-1])) 
        l--;

      linebuf[l] = 0;
      p = linebuf;

      while (deblank (*p)) 
        p++;

      if (*p == '#' || !*p) 
        continue;

      q = p;

      while (*q && !deblank (*q)) 
        q++;

      cmdlen = q-p;
      for (cp = config_tables;
	   cp->name && !(strlen (cp->name) == cmdlen && !memcmp (cp->name, p, q-p));
	   cp++);

      if (!cp->name) 
        {
          printd (nsd, -1, 0, "%s:%d: unknown configuration command '%.*s'",
	  	  filename, lno, q-p, p);
          continue;
        }

      while (deblank (*q)) 
        q++;

      cp->fn (nsd, filename, lno, q);
    }
}

static void 
readconf (ndl_sd nsd, 
          const char *filename) 
{
  get_text gt;
  
  gt.file = fopen (filename,"r");

  if (!gt.file) 
    {
      if (errno == ENOENT) 
        {
          printb (nsd, -1, 0,"configuration file `%s' does not exist", filename);
          return;
        }

      puterr (nsd,errno);
      printd (nsd, -1, 0, "cannot open configuration file '%s': %s",
	       filename, strerror(errno));
      return;
    }

  fparse (nsd, filename, fgetline, gt);
  
  fclose (gt.file);
}

int 
set_nonblock (ndl_sd nsd, 
              int fd) 
{
  int r;
  
  r = fcntl (fd, F_GETFL, 0); 

  if (r < 0) 
    return errno;

  r |= O_NONBLOCK;

  r = fcntl (fd,F_SETFL,r); 
  
  if (r < 0) 
    return errno;

  return 0;
}

static int 
ndl_state_load (ndl_sd *nsd_r, 
                FILE *defile,
                query_flag qf) 
{
  ndl_sd nsd;
  
  nsd = malloc (sizeof (*nsd)); 
 
  if (!nsd)  
    return errno;

  nsd->defile = defile;
  INIT_LIST (nsd->list);
  INIT_LIST (nsd->output_list);
  nsd->protoc.udp = -1;
  nsd->next_id = qf;
  nsd->nservers = 0;
  nsd->q_opcode = nsd->q_rd = 0;
  nsd->class = -1;

  *nsd_r = nsd;
  return 0; 
}

static int 
initnet (ndl_sd nsd) 
{
  struct in_addr ia;
  struct protoent *pe;
  int r;
  
  if (!nsd->nservers) 
    {
      if (nsd->defile)
        fprintf (nsd->defile, "Dns: Did not find a suitable nameservers, using localhost\n");

      ia.s_addr = htonl (INADDR_LOOPBACK);
      putserv (nsd,ia);
    }

  pe = getprotobyname ("udp"); 
 
  if (!pe) 
    { 
      r = ENOPROTOOPT; 
      goto j_free; 
    }

  nsd->protoc.udp = socket (AF_INET, SOCK_DGRAM, pe->p_proto);

  if (nsd->protoc.udp < 0) 
    { 
      r = errno; 
      goto j_free; 
    }

  r = set_nonblock (nsd, nsd->protoc.udp);

  if (r) 
    { 
      r = errno; 
      goto j_close_udp; 
    }
  
  return 0;

 j_close_udp:
   close (nsd->protoc.udp);

 j_free:
   free (nsd);
   return r;
}

int 
dnsinit (ndl_sd *nsd_r, 
         FILE *defile,
         query_flag qf) 
{
  ndl_sd nsd;
  int r;
  
  r = ndl_state_load (&nsd, defile ? defile : stderr, qf);
  if (r) 
    return r;

  readconf (nsd, "/etc/resolv.conf");

  if (nsd->conferrno && nsd->conferrno != EINVAL) 
    {
      r = nsd->conferrno;
      free (nsd);
      return r;
    }

  r = initnet (nsd);
  if (r) 
    return r;

  *nsd_r = nsd;
  return 0;
}

void 
dnsend (ndl_sd nsd) 
{
  for (;;) 
    {
      if (nsd->list.head) 
        {
          dnsquit (nsd->list.head);
        }
      else if (nsd->output_list.head) 
        {
          dnsquit (nsd->output_list.head);
        }
      else
        { 
          break;
        }
    }

  close (nsd->protoc.udp);
  free (nsd);
}
