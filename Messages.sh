#!bin/sh
 
$EXTRACTRC `find . -name \*.rc -o -name \*.ui -o -name \*.kcfg` >> rc.cpp
$PREPARETIPS > tips.cpp
$XGETTEXT `find . -name \*.cc -o -name \*.cpp -o -name \*.h -name \*.qml` -o $podir/nepomukwidgets.pot
