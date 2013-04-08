#  Makefile -- make ndl source file.
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

include config.mk
include src/client.mk

TEXI:= ${NDL_DOC}.tex
TEXINFO:= $(patsubst %.tex,%.texi,$(TEXI)) 
T:= ${D}
CMD_HTML= ${D}html
IGNORE_INFO= ${D}force

$(NDL_MAN).pdf: MIFLAGS = $(T)$(CMD_HTML)
$(NDL_MAN).pdf: MIFLAGS += $(T)$(IGNORE_INFO) 
$(NDL_MAN).pdf: GRFLAGS = ${D}Tps 
$(NDL_MAN).pdf: GRFLAGS += ${D}m ${NDL_MAN}
$(NDL_MAN).pdf: $(TEXINFO) 
	$(MAKEINFO) $(MIFLAGS)$< 
	$(GROFF) $(GRFLAGS) $(NDL_NAME).$(NDL_MAN) | ps2pdf - $@ 
	(cd $(BASEDIR); $(MAKE))

