#  Makefile -- make ndl library source file.
#  
#  Copyright (C) 2012 2013 S.meng.
#    
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>. 

include ../config.mk 

NDL_LIB:= boot detect event messages 
NDL_H:= ndl.h inside.h
NDL_LIB_OBJS:= $(foreach n,$(NDL_LIB),$(n).o) 
NDL_LIB_C:= $(NDL_LIB_OBJS:.o=.c)

lib$(NDL_NAME).a: $(NDL_LIB_OBJS) 
	$(AR) rv $@ $? 

.c.o:
	$(CC) $(CFLAGS) -c $<

$(NDL_LIB_OBJS): $(NDL_H)

clean:
	$(RM) *.o 

