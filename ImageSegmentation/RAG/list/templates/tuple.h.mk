#!/usr/local/bin/tcsh -f

echo ""
echo "#ifndef _TUPLE_H"
echo "#define _TUPLE_H"
echo \#include \"tuple_macro.h\"

foreach incl ($*)
	echo \#include \"$incl\"
end;

echo "#endif"
