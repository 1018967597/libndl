/* client.c -- simple test program, not part of the library.
 
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


#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>

#include "ndl.h"

#define OWNER "www.google.com"

char usage[] = "Usage: %s [-h] [-v] [-o owner] [-l]\n";
static char *progname;

int 
main (int argc, 
      char *const *argv) 
{
  ndl_sd nsd;
  query_sd *qsd;
  answer_sd *asd;
  const char *domain;
  int i, j, opt, f, d, z, v, u, w, m, e;  
  ndl_si r;
  type_values *type;
  class_values cv;
  query_flag qf;
 
  progname = argv[0];
  f = d = u = w = m = e = 1;
  z = v = 0; 
   
  while ((opt  = getopt (argc, argv, "o:hvl")) != EOF)
    switch (opt)
      {
        case 'h':
                 fputs ("This is libndl's dome -- client, distrabution DNS datagram.\n", stderr);
                 fprintf (stderr, usage, progname);
                 fputs ("-h, help\t\tGet help, print a summary of the options.\n", stderr);
                 fputs ("-v, version\t\tPrint the version number of client.\n", stderr); 
                 fputs ("-o, owner\t\tInput domain that you want to query.\n", stderr);
                 fputs ("-l, library\t\tOutput the version number of library.\n", stderr);
                goto j_exit;
         case 'l':printf ("libNDL(vtransf) version - 0.0.1\n"); 
         goto j_exit;
         case 'v':printf ("Client version - 1.0.0\n"); 
         goto j_exit;
         case 'o': domain = optarg;
         goto j_start;
         default:return 1;
      }
  
  domain = OWNER;

 j_start:
  type = A; 
  cv   = IN; 
  qf   = SQUERY; 

  qsd = malloc (sizeof (qsd));

  if (!qsd) 
    { 
      perror ("malloc qsd"); 
      exit (3); 
    }

  r = vtransf (nsd, &qsd, &asd, domain, type, cv, qf);
 
  if (r)
    goto j_error;  
        
  fprintf (stdout, "## Send query (%d bytes);\n", asd->query_dglen);
  fprintf (stdout, "## Header:\n");
  fprintf (stdout, ">> id: %d, opcode: %s, flags: %s;\n", asd->q_id, asd->q_opcode, asd->q_rd);
  fprintf (stdout, ">> QDCOUNT: %d, ANCOUNT: %d, NSCOUNT: %d, ARCOUNT: %d;\n", (query_flag)QDCOUNT, (query_flag)ANCOUNT, (query_flag)NSCOUNT, (query_flag)ARCOUNT);
  fprintf (stdout, "## Question:\n");
  fprintf (stdout, ">> owner: %s, class: %s, type: %s;\n", domain, asd->class_name, asd->type_name);
  
  fprintf (stdout, "-- Send original datagram --\n");
    
  for (i = 0; i < asd->query_dglen; i++)
    {
        
      if (i == v)
        { 
          printf ("%.4x  ", i);
          v = w * 16; 
          w++;
        }         

      fprintf (stdout, "%.2x ", *(asd->query_dgram)++);

      if (i == (8 * f) - 1)
        {
          printf ("  ");  
          f = f + 2;
        }

      if ((i + 1) == m * 16)
        {
          printf ("\n");  
          m++;
        }
    }

  printf ("\n");
  fprintf (stdout, "\n## Got answer (%d bytes);\n", asd->recv_dglen); 
  fprintf (stdout, "## Messages:\n");
  fprintf (stdout, ">> status: %s, id: %d, opcode: %s, flags: %s, %s, %s;\n", asd->a_rcode, asd->a_id, asd->a_opcode, asd->a_qr ? asd->a_qr : "", asd->a_rd ? asd->a_rd : "", asd->a_ra ? asd->a_ra : "");
  fprintf (stdout, ">> QDCOUNT: %d, ANCOUNT: %d, NSCOUNT: %d, ARCOUNT: %d;\n", asd->qdc, asd->anc, asd->nsc, asd->arc);
  printf ("-- Receive original datagram --\n");
   
  for (j = 1; j <= asd->recv_dglen; j++)
    {
        
      if ((j - 1) == z)
        { 
          printf ("%.4x  ", (j - 1));
          z = u * 16; 
          u++;
        }         

      printf ("%.2x ", (asd->answer_dgram)[j - 1]);     

      if ((j - 1) == (8 * d) - 1)
        {
          printf ("  ");  
          d = d + 2;
        }

      if (j == e * 16)
        {
          printf ("\n");  
          e++;
        }
    }

 j_error:
  fprintf (stdout, "\n%s.\n", ostatinfo (asd->nsi));
 
  exit(0);
 j_exit:
  exit(0);
}
