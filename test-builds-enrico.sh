#!/bin/bash

function build {
    echo "*** Using $* ***"
    make distclean 2> /dev/null > /dev/null
    $* CONFIG+=silent
    make -j2
    if [ $? -ne 0 ]; then
        echo error with "$*"
       exit
    fi
    ls -l --color=yes fotowall
    #mv fotowall fotowall-XX
}

function build_dr {
    QTVER=`$* -v | grep Qt | awk -F\  '{print $4}'`
    build $1 CONFIG+=debug
    mv fotowall fotowall-$QTVER-d
    build $1 CONFIG+=release
    mv fotowall fotowall-$QTVER-r
}

echo "===== Testing compilers ====="
export PATH=/usr/lib/ccache/bin:$PATH
if [ "$1" != "" ]; then
    build $*
    exit
fi
build_dr /dati/qt/4.5.3/bin/qmake
build_dr qmake
build_dr /dati/qt/4.7/bin/qmake
