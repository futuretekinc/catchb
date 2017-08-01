#!/bin/sh

_PROMPT=
_INPUT=
_SECTION_COUNT=0
_COUNT=0

make_script()
{
	cat $1  |  while read line; do
	
		case $_CMD in
		v*)
			_PARAM=$(echo $line | awk '{ for(i=2;i<NF;i++)printf $i " "; printf $NF}')
			eval "let $_PARAM"
			;;
		*)
			eval echo \"$line\"
			;;
		esac
	done | awk -f ./acl.awk
}

make_script $1
