#!/bin/sh

AIM_VERSION=2.0.1


BUILD_PLATFORM=`uname -srm`
BUILD_DATE=`date +"%Y-%m-%d %H:%M"`

if [ -d "../.git" ]; then
    GIT_REVISION=`git show-ref --head -s | head -n 1`
else
    GIT_REVISION="N/A"
fi

LFILE=../LICENSE
VFILE=version.c

license() {
    echo "/*" >$1
    cat $LFILE | nl -b a -s ' * ' | cut -c7- >>$1
    echo " */" >>$1
    echo >>$1
}

license $VFILE
echo '#include "version.h"' >>$VFILE
echo >>$VFILE
echo "const char* aim_version = \"$AIM_VERSION\";" >>$VFILE 
echo "const char* git_revision = \"$GIT_REVISION\";" >>$VFILE 
echo "const char* build_platform = \"$BUILD_PLATFORM\";" >>$VFILE 
echo "const char* build_date = \"$BUILD_DATE\";" >>$VFILE 
echo >>$VFILE
