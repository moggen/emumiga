#!/bin/bash

cd `git rev-parse --show-toplevel`

filebase=`date '+emumiga_source_%Y%m%d'`
filename="${filebase}.tar.gz"

echo "Filename: $filename"

if [ -f distribute/$filename ]
then
	echo "Overwriting existing file"
	rm distribute/$filename
fi

#find * -maxdepth 0 '!' -path 'attic' -a '!' -path 'distribute'| \
#  xargs zip -9 -r distribute/$filename

find * -type f '!' -path 'old/*' -a '!' -path 'distribute/*' -a '!' -path 'TAGS' -a '!' -path '*~' -a '!' -path '*mmakefile'| \
  sort | xargs tar c --xform "s,^,${filebase}/," -f distribute/${filebase}.tar

gzip -9 distribute/${filebase}.tar
