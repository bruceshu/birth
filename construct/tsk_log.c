/*
 bruce 2020-7-28
 */


#include <stddef.h>
#include <time.h>
#include <stdarg.h>
#include <stdio.h>

#include "tsk_common.h"
#include "tsk_log.h"
#include "tsk_mutex.h"

#define TSK_LOG_BUFSIZE 2048


static uint32_t gMaxLogLevelConsole = TSK_LOG_INFO;
static uint32_t gMaxLogLevelFile = TSK_LOG_INFO;
static FILE *gLogFile = NULL;
static tsk_mutex_handle_t* gLogFileMutex = tsk_null;
static uint64_t gLogFileSize = 0;
static uint64_t gMaxLogFileSize = 10*1024*1024; // 10MB by default
static char *gLogFilePath = NULL;
static char *gLogFilePathBak = NULL;






void tsk_log_imp (const char *pszFun, const char *pszFile, int dLine, int dLevel, const char *pszFormat, ...)
{
    tsk_bool_t writeToConsole = tsk_false;
    tsk_bool_t writeToFile = tsk_false;
    if (dLevel <= gMaxLogLevelConsole) {
        writeToConsole = tsk_true;
    }
    if (dLevel <= gMaxLogLevelFile) {
        writeToFile = tsk_true;
    }
    
    if (!writeToConsole && !writeToFile) {
        return;
    }

    char strtime[20] = { 0 };
    time_t timep = time (NULL);
    struct tm *p_tm = localtime (&timep);
    strftime (strtime, sizeof (strtime), "%Y-%m-%d %H:%M:%S", p_tm);
    char buffer[TSK_LOG_BUFSIZE + 1] = {0};
#ifdef WIN32
	int dSize = _snprintf(buffer, TSK_LOG_BUFSIZE, "thread: %d %s %03d %s ",
		GetCurrentThreadId(), strtime, (int)(tsk_gettimeofday_ms() % 1000), LogLevelToString (dLevel));

	if (dSize < TSK_LOG_BUFSIZE)
	{
		va_list args;
		va_start (args, pszFormat);
		dSize += vsnprintf (buffer + dSize, TSK_LOG_BUFSIZE - dSize, pszFormat, args);
		va_end (args);
	}
	int iFileNameBegin = tsk_strLastIndexOf(pszFile, strlen(pszFile), "\\");

	const char *szFileName = pszFile + iFileNameBegin + 1;
	if (dSize < TSK_LOG_BUFSIZE)
	{
		_snprintf(buffer + dSize, TSK_LOG_BUFSIZE - dSize, " [%s#%s:%d]\n", pszFun, szFileName, dLine);
	}
#else
	int dSize = snprintf(buffer, TSK_LOG_BUFSIZE, "thread：%lu %s.%03d %-8s ",
		(unsigned long)pthread_self(), strtime, (int)(tsk_gettimeofday_ms() % 1000), LogLevelToString(dLevel));

	if (dSize < TSK_LOG_BUFSIZE)
	{
		va_list args;
		va_start(args, pszFormat);
		dSize += vsnprintf(buffer + dSize, TSK_LOG_BUFSIZE - dSize, pszFormat, args);
		va_end(args);
	}

	int iFileNameBegin = tsk_strLastIndexOf(pszFile, strlen(pszFile), "/");
	const char *szFileName = pszFile + iFileNameBegin + 1;
	if (dSize < TSK_LOG_BUFSIZE)
	{
		snprintf(buffer + dSize, TSK_LOG_BUFSIZE - dSize, " [%s#%s:%d]\n", pszFun, szFileName, dLine);
	}
#endif // WIN32

  
    if (writeToConsole) {
#if ANDROID
        int iAndroidLogLevel = LogLevelToAndroidLevel (dLevel);
        __android_log_write (iAndroidLogLevel, "YOUME", buffer);
#elif defined(WIN32)
	    OutputDebugString(buffer);
#else
        printf ("%s", buffer);
#endif
    }
    
    if (gLogFileMutex) {
        tsk_mutex_lock(gLogFileMutex);
        if (gLogFile && writeToFile) {
            if (gLogFileSize >= gMaxLogFileSize) {
                if (gLogFilePathBak) {
                    fclose(gLogFile);
                    remove(gLogFilePathBak);
                    rename(gLogFilePath, gLogFilePathBak);
                    gLogFile = fopen (gLogFilePath, "w+");
                    gLogFileSize = 0;
                }else {
                    fclose(gLogFile);
                    remove(gLogFilePath);
                    gLogFile = fopen (gLogFilePath, "w+");
                    gLogFileSize = 0;
                }
            }
            if (gLogFile) {
                int logInfoLen = strlen (buffer);
                gLogFileSize += logInfoLen;
                fwrite (buffer, 1, logInfoLen, gLogFile);
                fflush (gLogFile);
            }
        }
        tsk_mutex_unlock(gLogFileMutex);
    }
}