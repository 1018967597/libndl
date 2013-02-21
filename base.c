/* base.c -- this file is a some simple base function, it 
   called library programing interface.

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


#include "inner.h"

int
getfile (FILE *file, 
         const char *filename,     
         int lno, 
         char *buf,
         int buflen)
{
  int c, i;
  char *p;
  
  p = buf;
  buflen--;
  
  i = 0;
   
  for (;;)
    {
      if (i == buflen)
        {
          ndl_printw (0, 0, -1, "too long!");
          goto j_badline; 
        }
       
      c = getc (file);
 
      if (!c)
        {
          ndl_printw (0, 0, -1, "%s:%d: line contains null!", filename, lno);
          goto j_badline;
        }
      else if (c == '\n')
        {
          break;  
        }
      else if (c == EOF)
        {
          if (ferror(file))
            {
              ndl_printb (0, 0, -1, "the file read error!");
              return -1;
            }
          
          if (!i) return -1;
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
    
    j_badline:
      return -2;
}
