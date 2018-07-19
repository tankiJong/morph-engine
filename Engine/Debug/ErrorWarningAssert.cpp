//-----------------------------------------------------------------------------------------------
// ErrorWarningAssert.cpp
//

//-----------------------------------------------------------------------------------------------
#ifdef _WIN32
#define PLATFORM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

//-----------------------------------------------------------------------------------------------
#include "Engine/Debug/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include <stdarg.h>
#include <iostream>
#include "Engine/Debug/Log.hpp"

//-----------------------------------------------------------------------------------------------
bool IsDebuggerAvailable()
{
#if defined( PLATFORM_WINDOWS )
	typedef BOOL (CALLBACK IsDebuggerPresentFunc)();

	// Get a handle to KERNEL32.DLL
	static HINSTANCE hInstanceKernel32 = GetModuleHandle( TEXT( "KERNEL32" ) );
	if( !hInstanceKernel32 )
		return false;

	// Get a handle to the IsDebuggerPresent() function in KERNEL32.DLL
	static IsDebuggerPresentFunc* isDebuggerPresentFunc = (IsDebuggerPresentFunc*) GetProcAddress( hInstanceKernel32, "IsDebuggerPresent" );
	if( !isDebuggerPresentFunc )
		return false;

	// Now CALL that function and return its result
	static BOOL isDebuggerAvailable = isDebuggerPresentFunc();
	return( isDebuggerAvailable == TRUE );
#else
	return false;
#endif
}

//-----------------------------------------------------------------------------------------------
void DebuggerPrintf( const char* messageFormat, ... )
{
	va_list variableArgumentList;
	va_start( variableArgumentList, messageFormat );
  //Log::tagv("debug", messageFormat, variableArgumentList);
	va_end( variableArgumentList );
}


//-----------------------------------------------------------------------------------------------
// Converts a SeverityLevel to a Windows MessageBox icon type (MB_etc)
//
#if defined( PLATFORM_WINDOWS )
UINT GetWindowsMessageBoxIconFlagForSeverityLevel( SeverityLevel severity )
{
	switch( severity )
	{
		case SEVERITY_INFORMATION:		return MB_ICONASTERISK;		// blue circle with 'i' in Windows 7
		case SEVERITY_QUESTION:			return MB_ICONQUESTION;		// blue circle with '?' in Windows 7
		case SEVERITY_WARNING:			return MB_ICONEXCLAMATION;	// yellow triangle with '!' in Windows 7
		case SEVERITY_FATAL:			return MB_ICONHAND;			// red circle with 'x' in Windows 7
		default:						return MB_ICONEXCLAMATION;
	}
}
#endif


//-----------------------------------------------------------------------------------------------
const char* FindStartOfFileNameWithinFilePath( const char* filePath )
{
	if( filePath == nullptr )
		return nullptr;

	size_t pathLen = strlen( filePath );
	const char* scan = filePath + pathLen; // start with null terminator after last character
	while( scan > filePath )
	{
		-- scan;

		if( *scan == '/' || *scan == '\\' )
		{
			++ scan;
			break;
		}
	}

	return scan;
}


//-----------------------------------------------------------------------------------------------
void SystemDialogue_Okay( const std::string& messageTitle, const std::string& messageText, SeverityLevel severity )
{
	#if defined( PLATFORM_WINDOWS )
	{
		ShowCursor( TRUE );
		UINT dialogueIconTypeFlag = GetWindowsMessageBoxIconFlagForSeverityLevel( severity );
		MessageBoxA( NULL, messageText.c_str(), messageTitle.c_str(), MB_OK | dialogueIconTypeFlag | MB_TOPMOST );
		ShowCursor( FALSE );
	}
	#endif
}


//-----------------------------------------------------------------------------------------------
// Returns true if OKAY was chosen, false if CANCEL was chosen.
//
bool SystemDialogue_OkayCancel( const std::string& messageTitle, const std::string& messageText, SeverityLevel severity )
{
	bool isAnswerOkay = true;

	#if defined( PLATFORM_WINDOWS )
	{
		ShowCursor( TRUE );
		UINT dialogueIconTypeFlag = GetWindowsMessageBoxIconFlagForSeverityLevel( severity );
		int buttonClicked = MessageBoxA( NULL, messageText.c_str(), messageTitle.c_str(), MB_OKCANCEL | dialogueIconTypeFlag | MB_TOPMOST );
		isAnswerOkay = (buttonClicked == IDOK);
		ShowCursor( FALSE );
	}
	#endif

	return isAnswerOkay;
}


//-----------------------------------------------------------------------------------------------
// Returns true if YES was chosen, false if NO was chosen.
//
bool SystemDialogue_YesNo( const std::string& messageTitle, const std::string& messageText, SeverityLevel severity )
{
	bool isAnswerYes = true;

	#if defined( PLATFORM_WINDOWS )
	{
		ShowCursor( TRUE );
		UINT dialogueIconTypeFlag = GetWindowsMessageBoxIconFlagForSeverityLevel( severity );
		int buttonClicked = MessageBoxA( NULL, messageText.c_str(), messageTitle.c_str(), MB_YESNO | dialogueIconTypeFlag | MB_TOPMOST );
		isAnswerYes = (buttonClicked == IDYES);
		ShowCursor( FALSE );
	}
	#endif

	return isAnswerYes;
}


//-----------------------------------------------------------------------------------------------
// Returns 1 if YES was chosen, 0 if NO was chosen, -1 if CANCEL was chosen.
//
int SystemDialogue_YesNoCancel( const std::string& messageTitle, const std::string& messageText, SeverityLevel severity )
{
	int answerCode = 1;

	#if defined( PLATFORM_WINDOWS )
	{
		ShowCursor( TRUE );
		UINT dialogueIconTypeFlag = GetWindowsMessageBoxIconFlagForSeverityLevel( severity );
		int buttonClicked = MessageBoxA( NULL, messageText.c_str(), messageTitle.c_str(), MB_YESNOCANCEL | dialogueIconTypeFlag | MB_TOPMOST );
		answerCode = (buttonClicked == IDYES ? 1 : (buttonClicked == IDNO ? 0 : -1) );
		ShowCursor( FALSE );
	}
	#endif

	return answerCode;
}


//-----------------------------------------------------------------------------------------------
__declspec( noreturn ) void FatalError( const char* filePath, const char* functionName, int lineNum, const std::string& reasonForError, const char* conditionText )
{
	std::string errorMessage = reasonForError;
	if( reasonForError.empty() )
	{
		if( conditionText )
			errorMessage = Stringf( "ERROR: \"%s\" is false!", conditionText );
		else
			errorMessage = "Unspecified fatal error";
	}

	const char* fileName = FindStartOfFileNameWithinFilePath( filePath );
//	std::string appName = theApplication ? theApplication->GetApplicationName() : "Unnamed Application";
	std::string appName = "Unnamed Application";
	std::string fullMessageTitle = appName + " :: Error";
	std::string fullMessageText = errorMessage;
	fullMessageText += "\n\nThe application will now close.\n";
	bool isDebuggerPresent = (IsDebuggerPresent() == TRUE);
	if( isDebuggerPresent )
	{
		fullMessageText += "\nDEBUGGER DETECTED!\nWould you like to break and debug?\n  (Yes=debug, No=quit)\n";
	}

	fullMessageText += "\n---------- Debugging Details Follow ----------\n";
	if( conditionText )
	{
		fullMessageText += Stringf( "\nThis error was triggered by a run-time condition check:\n  %s\n  from %s(), line %i in %s\n",
			conditionText, functionName, lineNum, fileName );
	}
	else
	{
		fullMessageText += Stringf( "\nThis was an unconditional error triggered by reaching\n line %i of %s, in %s()\n",
			lineNum, fileName, functionName );
	}

	DebuggerPrintf( "\n==============================================================================\n" );
	DebuggerPrintf( "RUN-TIME FATAL ERROR on line %i of %s, in %s()\n", lineNum, fileName, functionName );
	DebuggerPrintf( "%s(%d): %s\n", filePath, lineNum, errorMessage.c_str() ); // Use this specific format so Visual Studio users can double-click to jump to file-and-line of error
	DebuggerPrintf( "==============================================================================\n\n" );

	if( isDebuggerPresent )
	{
		bool isAnswerYes = SystemDialogue_YesNo( fullMessageTitle, fullMessageText, SEVERITY_FATAL );
		ShowCursor( TRUE );
		if( isAnswerYes )
		{
			__debugbreak();
		}
	}
	else
	{
		SystemDialogue_Okay( fullMessageTitle, fullMessageText, SEVERITY_FATAL );
		ShowCursor( TRUE );
	}

	exit( 0 );
}


//-----------------------------------------------------------------------------------------------
void RecoverableWarning( const char* filePath, const char* functionName, int lineNum, const std::string& reasonForWarning, const char* conditionText )
{
	std::string errorMessage = reasonForWarning;
	if( reasonForWarning.empty() )
	{
		if( conditionText )
			errorMessage = Stringf( "WARNING: \"%s\" is false!", conditionText );
		else
			errorMessage = "Unspecified warning";
	}

	const char* fileName = FindStartOfFileNameWithinFilePath( filePath );
//	std::string appName = theApplication ? theApplication->GetApplicationName() : "Unnamed Application";
	std::string appName = "Unnamed Application";
	std::string fullMessageTitle = appName + " :: Warning";
	std::string fullMessageText = errorMessage;

	bool isDebuggerPresent = (IsDebuggerPresent() == TRUE);
	if( isDebuggerPresent )
	{
		fullMessageText += "\n\nDEBUGGER DETECTED!\nWould you like to continue running?\n  (Yes=continue, No=quit, Cancel=debug)\n";
	}
	else
	{
		fullMessageText += "\n\nWould you like to continue running?\n  (Yes=continue, No=quit)\n";
	}

	fullMessageText += "\n---------- Debugging Details Follow ----------\n";
	if( conditionText )
	{
		fullMessageText += Stringf( "\nThis warning was triggered by a run-time condition check:\n  %s\n  from %s(), line %i in %s\n",
			conditionText, functionName, lineNum, fileName );
	}
	else
	{
		fullMessageText += Stringf( "\nThis was an unconditional warning triggered by reaching\n line %i of %s, in %s()\n",
			lineNum, fileName, functionName );
	}

	DebuggerPrintf( "\n------------------------------------------------------------------------------\n" );
	DebuggerPrintf( "RUN-TIME RECOVERABLE WARNING on line %i of %s, in %s()\n", lineNum, fileName, functionName );
	DebuggerPrintf( "%s(%d): %s\n", filePath, lineNum, errorMessage.c_str() ); // Use this specific format so Visual Studio users can double-click to jump to file-and-line of error
	DebuggerPrintf( "------------------------------------------------------------------------------\n\n" );

	if( isDebuggerPresent )
	{
		int answerCode = SystemDialogue_YesNoCancel( fullMessageTitle, fullMessageText, SEVERITY_WARNING );
		ShowCursor( TRUE );
		if( answerCode == 0 ) // "NO"
		{
			exit( 0 );
		}
		else if( answerCode == -1 ) // "CANCEL"
		{
			__debugbreak();
		}
	}
	else
	{
		bool isAnswerYes = SystemDialogue_YesNo( fullMessageTitle, fullMessageText, SEVERITY_WARNING );
		ShowCursor( TRUE );
		if( !isAnswerYes )
		{
			exit( 0 );
		}
	}
}


