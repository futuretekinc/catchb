#!/bin/bash

_PROMPT=
_INPUT=
_SECTION_COUNT=0
_COUNT=0

make_script()
{
	echo "{"
	cat $1  |  while read line; do
		_CMD=$(echo $line | awk '{print $1}')
		_PARAM=$(echo $line | awk '{ for(i=2;i<NF;i++)printf $i " "; printf $NF}')
	
		case $_CMD in
		"v")
			eval "let $_PARAM"
			;;
	
		"s")
			if [ "$_SECTION_COUNT" != "0" ]
			then
				echo ","
			fi
	
			echo "	\"$_PARAM\" : "
			let _SECTION_COUNT=$_COUNT+1
			;;
	
		"b")
			echo "	["
			let _COUNT=0
			;;
	
		"e")
			echo "	]"
			;;
	
		"p")
			if [ "$_COUNT" != "0" ]
			then
				echo ","
			fi
	
			_PROMPT=$(eval echo \""$_PARAM"\")
			echo "		{"
			echo "			\"prompt\" : \"$_PROMPT\","
			;;
	
		"c")
			_INPUT=$(eval echo \""$_PARAM"\")
			echo "			\"command\" : \"$_INPUT\""
			echo "		}"
			let _COUNT=$_COUNT+1
			;;
		esac
	done
	echo "}"
}

ACL=$(make_script $1)

echo $ACL
