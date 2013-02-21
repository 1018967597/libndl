#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NAMES 255
#define LABELS 63
static inline int dedigital (int c)
{
  return c >= '0' && c <= '9';
}

int
main ()
{
 /*
  typedef struct
  {
    unsigned int c1:1;
    unsigned int c2:1;
    unsigned int c3:1;
    unsigned int c4:1;
  } flags; 
  
  flags x;
  x.c1 = 0;
  x.c2 = 1;
  x.c3 = 0;
  x.c4 = 0;
  
  printf ("sizeof is %d\n", sizeof (x));
  printf ("x is %d\n", x.c1);
  return 0;
 */
  const char *name;
  unsigned char names[NAMES];
  //ndl_si nsi;
  const char *p, *q;
  int ll, c, clabel, nl; 
  //nsi = ndl_make_header ();
  //if (nsi)
   // return nsi;
  
  //INIT_QUERY ();
  name = "www.baidu111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111.com"; 
  nl = strlen (name);
  printf ("nl:%d\n", nl); 
  p = name;
  q = p + nl;
 
  printf ("p:%d, q:%d\n", *p, *q);
  clabel = 0;
  while (p != q)
    {
      ll = 0;
      while (p != q && (c = *p++) != '.')
        {
          if (c == '\\')
            {
              printf ("ss!\n");

             // if ()
             //   return;
              if (dedigital (p[0]))
                {
                  if (dedigital (p[1]) && dedigital (p[2]))
                    {
                      c = (*p++ - '0') * 100 + (*p++ - '0') * 10 + (*p++ -'0');
                      if (c > NAMES)
                        //return query_name_too_long; 
                        printf ("query_name_too_long!");
                    }
                  else
                    {
                      //return query_name_invaild;
                      printf ("query_name_invaild");
                    }
                } 
              else if (! (c = *p++))
                {
                  //return query_name_invaild;
                  printf ("query_name_invaild"); 
                }
            }
          /*
          if ()
            {
              if ()
                {
                  if ()
                    return; 
                }
              else if ()
                {
          
                }
            }
          */
          if (ll == sizeof (names))
            //return query_name_invalid;
            printf ("query_name_invaild");  
          names[ll++] = c;
        } 
      if (!ll)
        //return query_name_invalid;
        printf ("query_name_invaild"); 
      if (clabel++ > LABELS)
       // return query_label_too_long;
        printf ("query_name_too_long");
    } 
  
 // nsi = ndl_make_footer ();
 // return dns_ok;
  printf ("dns_ok\n");
 // printf ("name:%s\n", name);
  return 0;
}
