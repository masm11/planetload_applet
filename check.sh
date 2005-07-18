#!/bin/sh
# $Id: check.sh 293 2005-06-25 12:30:40Z masm $

distdir="$1"
top_srcdir="$2"

find $distdir			\
  -name autom4te.cache -prune -o	\
  -name .svn -prune -o	\
  -type f		\
  \! -name '*~'		\
  \! -name Makefile.in	\
  \! -name configure	\
  \! -name config.h.in	\
  \! -name aclocal.m4	\
  \! -name INSTALL	\
  \! -name COPYING	\
  \! -name depcomp	\
  \! -name compile	\
  \! -name install-sh	\
  \! -name missing	\
  \! -name Makefile.in.in	\
  \! -name ja.po	\
  \! -name ja.gmo	\
  \! -name config.guess	\
  \! -name config.sub	\
  \! -name mkinstalldirs	\
  \! -name intltool-extract.in	\
  \! -name intltool-merge.in	\
  \! -name intltool-update.in	\
  \! -name ltmain.sh	\
  \! -name planetload_applet.pot	\
  \! -name i18n-support.h	\
  -print |
while read path; do
  file=`echo $path | sed 's,.*/,,'`

  if ! grep '[\$]Id: '"$file"' .* [\$]' $path > /dev/null; then
    echo $path: No Id. >&2
    echo false
  fi
  
  case "$file" in
  *.[ch])
    if ! grep '^[^"]*Copyright (C) 2003-2005 Yuuki Harano[^"]*$' $path > /dev/null; then
      echo $path: No copyright. >&2
      echo false
    fi
    ;;
  esac

  case "$path" in
  *.[ch])
    if ! grep '#include "../config.h"' $path > /dev/null; then
      echo $path: config.h not included. >&2
      echo false
    fi
    ;;
  esac
done | grep false > /dev/null

if [ $? -eq 0 ]; then
  exit 1
fi

if ( cd $top_srcdir; svn status ) 2>&1 | fgrep -v "$distdir" | grep .; then
    echo bad svn status >&2
    exit 1
else
    :
fi

exit 0
