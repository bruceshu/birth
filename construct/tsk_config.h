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




#endif