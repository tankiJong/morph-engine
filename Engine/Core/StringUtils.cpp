#include "Engine/Core/StringUtils.hpp"
#include <stdarg.h>
#include "Engine/Core/ErrorWarningAssert.hpp"

//-----------------------------------------------------------------------------------------------
const int STRINGF_STACK_LOCAL_TEMP_LENGTH = 2048;


//-----------------------------------------------------------------------------------------------
const std::string Stringf( const char* format, ... )
{
	char textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH ];
	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, STRINGF_STACK_LOCAL_TEMP_LENGTH, _TRUNCATE, format, variableArgumentList );	
	va_end( variableArgumentList );
	textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH - 1 ] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	return std::string( textLiteral );
}


//-----------------------------------------------------------------------------------------------
const std::string Stringf( const int maxLength, const char* format, ... )
{
	char textLiteralSmall[ STRINGF_STACK_LOCAL_TEMP_LENGTH ];
	char* textLiteral = textLiteralSmall;
	if( maxLength > STRINGF_STACK_LOCAL_TEMP_LENGTH )
		textLiteral = new char[ maxLength ];

	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, maxLength, _TRUNCATE, format, variableArgumentList );	
	va_end( variableArgumentList );
	textLiteral[ maxLength - 1 ] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	std::string returnValue( textLiteral );
	if( maxLength > STRINGF_STACK_LOCAL_TEMP_LENGTH )
		delete[] textLiteral;

	return returnValue;
}

std::vector<std::string> split(const char* data, const char* delimiters) {
  // TODO: better way to split string
  char raw[(unsigned char)-1];
  strcpy_s(raw, data);
  std::vector<std::string> vals;

  char* val = nullptr;
  char* rest = nullptr;
  val = strtok_s(raw, delimiters, &rest);
  while(val != NULL) {
    vals.push_back(val);
    val = strtok_s(NULL, delimiters, &rest);
  }

  return vals;
}

unsigned char castHex(char hex) {
  if(hex >= '0' && hex <= '9') {
    return hex - '0';
  }
  if(hex >= 'a' && hex <= 'f') {
    return hex - 'a' + 10;
  }
  if(hex >= 'A' && hex <= 'F') {
    return hex - 'A' + 10;
  }

  ERROR_AND_DIE("illegal hex char");
}





