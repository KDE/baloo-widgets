#!bin/sh

$XGETTEXT `find . -name \*.cc -o -name \*.cpp -o -name \*.h -o -name \*.qml` -o $podir/baloowidgets5.pot
rm -f rc.cpp
