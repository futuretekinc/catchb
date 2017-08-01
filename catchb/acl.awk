BEGIN {
	_count = 0
	_section_count = 0
	_output = "{"
}
{
	switch($1)
	{
	case	"v":
		eval "let $_PARAM"
		break

	case	"s":
		if ( _section_count != 0 )
		{
			_output = _output ","
		}

		_output = _output "\"" $2 "\" : "

		_section_count++
		break

	case	"b":
		_output = _output "  ["
		_count = 0
		break

	case	"e":
		_output = _output "  ]"
		break

	case	"p":
		if ( _count != 0)
		{
			_output = _output ","
		}

		_output = _output "{\"prompt\" : \""
		for(i = 2 ; i <= NF ; i++)
		{
			_output = _output $i " "
		}
		_output = _output "\", "
		break

	case	"c":
		_output = _output "\"command\" : \""
		for(i = 2 ; i <= NF ; i++)
		{
			_output = _output $i " "
		}
		_output = _output "\"}"
		_count=_count+1
		break;
	}
}
END{
	_output = _output "}"
	print _output
}
