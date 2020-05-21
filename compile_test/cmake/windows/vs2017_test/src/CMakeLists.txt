
project(YouMeCommon)
INCLUDE_DIRECTORIES (.)

FILE (GLOB header_files *.h *.hpp)
FILE (GLOB source_files *.c *.cc *.cpp)
 
IF(WIN32) 
	ADD_LIBRARY (YouMeCommon ${header_files} ${source_files})
ENDIF(WIN32)




