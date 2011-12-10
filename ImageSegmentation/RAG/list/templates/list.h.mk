#!/usr/local/bin/tcsh -f

echo ""
echo "#ifndef _LIST_H"
echo "#define _LIST_H"
echo "#ifdef __cplusplus"
echo extern \"C\" "{"
echo "#endif"

echo \#include \"list_macro.h\"

foreach incl ($*)
	echo \#include \"$incl\"
end;

echo "#ifdef __cplusplus"
echo "}"
echo "#endif"
echo "#endif"
