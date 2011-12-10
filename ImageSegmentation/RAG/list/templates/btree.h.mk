#!/usr/local/bin/tcsh -f

echo ""
echo "#ifndef _BTREE_H"
echo "#define _BTREE_H"
echo "#ifdef __cplusplus"
echo extern \"C\" "{"
echo "#endif"

echo \#include \"btree_macro.h\"

foreach incl ($*)
	echo \#include \"$incl\"
end;

echo "#ifdef __cplusplus"
echo "}"
echo "#endif"
echo "#endif"
