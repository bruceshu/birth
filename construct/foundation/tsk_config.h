/*
 bruce 2020-7-28
 */


#ifndef TSK_CONFIG_H
#define TSK_CONFIG_H

/* Guards against C++ name mangling */
#ifdef __cplusplus
#	define TSK_BEGIN_DECLS extern "C" {
#	define TSK_END_DECLS }
#else
#	define TSK_BEGIN_DECLS 
#	define TSK_END_DECLS
#endif

/* Used on Windows and Symbian systems to export/import public functions and global variables.
*/
#if !defined(__GNUC__) && defined(TINYSAK_EXPORTS)
# 	define TINYSAK_API		__declspec(dllexport)
#	define TINYSAK_GEXTERN	extern __declspec(dllexport)
#elif !defined(__GNUC__) && !defined(TINYSAK_IMPORTS_IGNORE)
# 	define TINYSAK_API		__declspec(dllimport)
#	define TINYSAK_GEXTERN	__declspec(dllimport)
#else
#	define TINYSAK_API
#	define TINYSAK_GEXTERN	extern
#endif

// Windows (XP/Vista/7/CE and Windows Mobile) macro definition.
#if defined(WIN32)|| defined(_WIN32) || defined(_WIN32_WCE)
#	define TSK_UNDER_WINDOWS	1
#	if defined(_WIN32_WCE) || defined(UNDER_CE)
#		define TSK_UNDER_WINDOWS_CE	1
#		define TSK_STDCALL			__cdecl
#	else
#		define TSK_STDCALL 
#	endif
#	if defined(WINAPI_FAMILY) && (WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP || WINAPI_FAMILY == WINAPI_FAMILY_APP)
#		define TSK_UNDER_WINDOWS_RT		1
#	endif
#else
#	define TSK_STDCALL
#endif


#endif