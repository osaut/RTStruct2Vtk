project(RTSReader) 
cmake_minimum_required(VERSION 2.8)

SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR})

# On charge GDCM
find_package(GDCM REQUIRED)
IF(GDCM_FOUND)
	MESSAGE("Ok pour GDCM")
	INCLUDE(${GDCM_USE_FILE})
ELSE(GDCM_FOUND)
	MESSAGE("GDCM non trouvé !")
ENDIF(GDCM_FOUND)


INCLUDE(FindVTK)
FIND_PACKAGE(VTK)
IF (VTK_FOUND)
    INCLUDE (${VTK_USE_FILE})
ELSE(VTK_FOUND)
    MESSAGE(STATUS "VTK non trouvé !")
ENDIF (VTK_FOUND)

SET(EXENAME "rtsr.out")
add_executable(${EXENAME} main.cc)
target_link_libraries(${EXENAME} gdcmCommon)
target_link_libraries(${EXENAME} gdcmMSFF)
target_link_libraries(${EXENAME} vtkgdcm)
target_link_libraries(${EXENAME} vtkgdcm)
target_link_libraries(${EXENAME} ${VTK_LIBRARIES})



