#!/bin/bash

#
#   gurucomkz/evcp
#
#   Copyright (C) 2012-2016 Sergey Shevchenko
#
#   This source code is provided 'as-is', without any express or implied
#   warranty. In no event will the author be held liable for any damages
#   arising from the use of this software.
#
#   Permission is granted to anyone to use this software for any purpose,
#   including commercial applications, and to alter it and redistribute it
#   freely, subject to the following restrictions:
#
#   1. The origin of this source code must not be misrepresented; you must not
#      claim that you wrote the original source code. If you use this source code
#      in a product, an acknowledgment in the product documentation would be
#      appreciated but is not required.
#
#   2. Altered source versions must be plainly marked as such, and must not be
#      misrepresented as being the original source code.
#
#   3. This notice may not be removed or altered from any source distribution.
#
#   Sergey Shevchenko sergey.aegis@gmail.com
#



#source_files="sp_cronjobs sp_timer sp_curl splog mgr_oslist mgr_ip sp_sysuctrl sp_xml sp_session sp_stats sp_firstrun base64 mgr mgr_acc mgr_cts connection request sys config sp_common mydb smain" 
source_files="sp_cronjobs sp_timer sp_curl splog sp_sysuctrl sp_xml sp_session sp_stats sp_firstrun base64 mgr mgr_sitegroups mgr_sites mgr_siteurls mgr_acc connection request sys config sp_common mydb smain" 

    SETCOLOR_SUCCESS="echo -en \\033[1;32m"
    SETCOLOR_FAILURE="echo -en \\033[1;31m"
    SETCOLOR_WARNING="echo -en \\033[1;33m"
    SETCOLOR_NORMAL="echo -en \\033[0;39m"
function outok {
    echo -ne "$1\t:\t" ; $SETCOLOR_SUCCESS ; echo -n $2 ; $SETCOLOR_NORMAL; echo
}
function outfail {
    echo -ne "$1\t:\t" ; $SETCOLOR_FAILURE ; echo -n $2 ; $SETCOLOR_NORMAL; echo
}
function outwarn {
    echo -ne "$1\t:\t" ; $SETCOLOR_WARNING ; echo -n $2 ; $SETCOLOR_NORMAL; echo
}
SRCEXT=.cpp
rm -f .deps/*.Po
HDREXT=.h
OBJSRC=.o
OBJPATH="obj/"
objlist=""
for srcfile in $source_files
do
   objlist="${objlist} $OBJPATH$srcfile${OBJSRC}"
   if ! [ -a $OBJPATH$srcfile${OBJSRC} ] || \
	[[ `stat -c%y $OBJPATH$srcfile${OBJSRC}` != `stat -c%y $srcfile${SRCEXT}` ]] || \
	[[ -a $srcfile${HDREXT} && `stat -c%y $OBJPATH$srcfile${OBJSRC}` != `stat -c%y $srcfile${HDREXT}` ]]
   then
	if source="$srcfile${SRCEXT}" object="$OBJPATH$srcfile${OBJSRC}" libtool=no \
	   depfile=".deps/${srcfile}.Po" tmpdepfile=".deps/${srcfile}.TPo" \
	   depmode=gcc3 /bin/sh ./depcomp \
	   /usr/bin/g++ --ansi -O2 -Wno-deprecated \
		-I/usr/include/libxml2 \
		-I. -I/usr/local/openssl/include \
		-g -c -o $OBJPATH$srcfile${OBJSRC} `test -f '$srcfile${SRCEXT}' || echo './'`$srcfile${SRCEXT} 
	then
		touch --reference=$OBJPATH$srcfile${OBJSRC} $srcfile${SRCEXT} 
		if [ -a $srcfile${HDREXT} ]; then 
		   touch --reference=$OBJPATH$srcfile${OBJSRC} $srcfile${HDREXT} 
		fi
		outok $srcfile compiled
	fi
   else
	outwarn $srcfile "no-change"
   fi
done

if g++ -g -O2 --ansi -o spanel -Wall  ${objlist}  \
	-lpthread -levent -I. -DHAVE_SSL=1 \
	-L/usr/local/openssl/lib -lssl -lcrypt -lcrypto \
	`curl-config --libs` \
	-lz -lxml2 -lxslt  -lnsl /usr/local/lib/libsqlite3.a
	
then
	outok spanel compiled
fi

