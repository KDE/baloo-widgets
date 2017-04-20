#!bin/sh

$EXTRACTRC `find . -name \*.rc -o -name \*.ui -o -name \*.kcfg | grep -v "/src/naturalqueryparser/"` >> rc.cpp
$XGETTEXT `find . -name \*.cc -o -name \*.cpp -o -name \*.h -o -name \*.qml | grep -v "/src/naturalqueryparser/"` -o $podir/baloowidgets5.pot
rm -f rc.cpp
