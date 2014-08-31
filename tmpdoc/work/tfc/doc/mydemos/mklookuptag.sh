#!/bin/bash

echo -e "!_TAG_FILE_SORTED\t2\t/2=foldcase/" >filenametags
find . -not -regex '.*\.\(png\|gif\)' -type f -printf "%f\t%p\t1\n" | \
sort -f >>filenametags
