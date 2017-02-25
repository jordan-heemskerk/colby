# To find OpenCV 3 library visit http://opencv.willowgarage.com/wiki/
#
# The follwoing variables are optionally searched for defaults
#  OpenCV3_ROOT_DIR:                   Base directory of OpenCV 3 tree to use.
#
# The following are set after configuration is done:
#  OpenCV3_FOUND
#  OpenCV3_INCLUDE_DIRS
#  OpenCV3_LIBRARIES
#
# This file should be removed when CMake will provide an equivalent

#MESSAGE(STATUS "Looking for OpenCV3...")

# typical root dirs of installations, exactly one of them is used
IF(WIN32)
    SET(OpenCV3_POSSIBLE_ROOT_DIRS
        "$ENV{OpenCV_ROOT_DIR}"
        )
ELSE(WIN32)
    SET(OpenCV3_POSSIBLE_ROOT_DIRS
        "$ENV{ROS_ROOT}/../vision_opencv/opencv2/opencv"
	"$ENV{OpenCV_ROOT_DIR}"                         # *NIX: custom install location (like ROS)
        /usr/local                                      # Linux: default dir by CMake
        /usr                                            # Linux
        /opt/local                                      # OS X: default MacPorts location
	)
    MESSAGE(STATUS ${OpenCV3_POSSIBLE_ROOT_DIRS})
ENDIF(WIN32)


# select exactly ONE OpenCV 2 base directory
# to avoid mixing different version headers and libs
FIND_PATH(OpenCV3_ROOT_DIR
          NAMES include/opencv2/opencv.hpp
          PATHS ${OpenCV3_POSSIBLE_ROOT_DIRS}
          )

FIND_PATH(OpenCV3_CORE_INCLUDE_DIR
          NAMES core_c.h core.hpp wimage.hpp eigen.hpp
          PATHS "${OpenCV3_ROOT_DIR}/include/opencv2/core")
FIND_PATH(OpenCV3_IMGPROC_INCLUDE_DIR
          NAMES imgproc_c.h imgproc.hpp
          PATHS "${OpenCV3_ROOT_DIR}/include/opencv2/imgproc")
FIND_PATH(OpenCV3_FEATURES2D_INCLUDE_DIR
          NAMES features2d.hpp
          PATHS "${OpenCV3_ROOT_DIR}/include/opencv2/features2d")
FIND_PATH(OpenCV3_FLANN_INCLUDE_DIR
          NAMES flann.hpp
          PATHS "${OpenCV3_ROOT_DIR}/include/opencv2/flann")
FIND_PATH(OpenCV3_CALIB3D_INCLUDE_DIR
          NAMES calib3d.hpp
          PATHS "${OpenCV3_ROOT_DIR}/include/opencv2/calib3d")
FIND_PATH(OpenCV3_OBJDETECT_INCLUDE_DIR
          NAMES objdetect.hpp
          PATHS "${OpenCV3_ROOT_DIR}/include/opencv2/objdetect")
FIND_PATH(OpenCV3_HIGHGUI_INCLUDE_DIR
          NAMES   highgui_c.h highgui.hpp
          PATHS "${OpenCV3_ROOT_DIR}/include/opencv2/highgui")
FIND_PATH(OpenCV3_ML_INCLUDE_DIR
          NAMES ml.hpp
          PATHS "${OpenCV3_ROOT_DIR}/include/opencv2/ml")
FIND_PATH(OpenCV3_VIDEO_INCLUDE_DIR
          NAMES tracking.hpp background_segm.hpp
          PATHS "${OpenCV3_ROOT_DIR}/include/opencv2/video")

# absolute path to all libraries
SET(OpenCV3_LIBRARY_SEARCH_PATHS "${OpenCV3_ROOT_DIR}/lib")

IF(WIN32)
    SET(OpenCV3_LIBRARY_SEARCH_PATHS
        ${OpenCV3_LIBRARY_SEARCH_PATHS}
        "${OpenCV3_ROOT_DIR}/lib/Release"
        "${OpenCV3_ROOT_DIR}/lib/Debug"
        )
    FIND_LIBRARY(OpenCV3_CORE_LIBRARY
                 NAMES opencv_core310
                 PATHS ${OpenCV3_LIBRARY_SEARCH_PATHS})
    FIND_LIBRARY(OpenCV3_IMGPROC_LIBRARY
                 NAMES opencv_imgproc310
                 PATHS ${OpenCV3_LIBRARY_SEARCH_PATHS})
    FIND_LIBRARY(OpenCV3_FEATURES2D_LIBRARY
                 NAMES opencv_features2d310
                 PATHS ${OpenCV3_LIBRARY_SEARCH_PATHS})
    FIND_LIBRARY(OpenCV3_FLANN_LIBRARY
                 NAMES opencv_flann310
                 PATHS ${OpenCV3_LIBRARY_SEARCH_PATHS})
    FIND_LIBRARY(OpenCV3_CALIB3D_LIBRARY
                 NAMES opencv_calib3d310
                 PATHS ${OpenCV3_LIBRARY_SEARCH_PATHS})
    FIND_LIBRARY(OpenCV3_OBJDETECT_LIBRARY
                 NAMES opencv_objdetect310
                 PATHS ${OpenCV3_LIBRARY_SEARCH_PATHS})
    FIND_LIBRARY(OpenCV3_HIGHGUI_LIBRARY
                 NAMES opencv_highgui310
                 PATHS ${OpenCV3_LIBRARY_SEARCH_PATHS})
    FIND_LIBRARY(OpenCV3_ML_LIBRARY
                 NAMES opencv_ml310
                 PATHS ${OpenCV3_LIBRARY_SEARCH_PATHS})
    FIND_LIBRARY(OpenCV3_VIDEO_LIBRARY
                 NAMES opencv_video310
                 PATHS ${OpenCV3_LIBRARY_SEARCH_PATHS})
    FIND_LIBRARY(OpenCV3_FFMPEG_LIBRARY
                 NAMES opencv_ffmpeg310 opencv_ffmpeg310_64
                 PATHS ${OpenCV3_LIBRARY_SEARCH_PATHS})
ELSE(WIN32)
    FIND_LIBRARY(OpenCV3_CORE_LIBRARY       NAMES opencv_core       PATHS ${OpenCV3_LIBRARY_SEARCH_PATHS})
    FIND_LIBRARY(OpenCV3_IMGPROC_LIBRARY    NAMES opencv_imgproc    PATHS ${OpenCV3_LIBRARY_SEARCH_PATHS})
    FIND_LIBRARY(OpenCV3_FEATURES2D_LIBRARY NAMES opencv_features2d PATHS ${OpenCV3_LIBRARY_SEARCH_PATHS})
    FIND_LIBRARY(OpenCV3_FLANN_LIBRARY      NAMES opencv_flann      PATHS ${OpenCV3_LIBRARY_SEARCH_PATHS})
    FIND_LIBRARY(OpenCV3_CALIB3D_LIBRARY    NAMES opencv_calib3d    PATHS ${OpenCV3_LIBRARY_SEARCH_PATHS})
    FIND_LIBRARY(OpenCV3_OBJDETECT_LIBRARY  NAMES opencv_objdetect  PATHS ${OpenCV3_LIBRARY_SEARCH_PATHS})
    FIND_LIBRARY(OpenCV3_HIGHGUI_LIBRARY    NAMES opencv_highgui    PATHS ${OpenCV3_LIBRARY_SEARCH_PATHS})
    FIND_LIBRARY(OpenCV3_ML_LIBRARY         NAMES opencv_ml         PATHS ${OpenCV3_LIBRARY_SEARCH_PATHS})
    FIND_LIBRARY(OpenCV3_VIDEO_LIBRARY      NAMES opencv_video      PATHS ${OpenCV3_LIBRARY_SEARCH_PATHS})
ENDIF(WIN32)

SET(OpenCV3_INCLUDE_DIRS
    ${OpenCV3_ROOT_DIR}/include
    ${OpenCV3_CORE_INCLUDE_DIR}
    ${OpenCV3_IMGPROC_INCLUDE_DIR}
    ${OpenCV3_FEATURES2D_INCLUDE_DIR}
    ${OpenCV3_FLANN_INCLUDE_DIR}
    ${OpenCV3_CALIB3D_INCLUDE_DIR}
    ${OpenCV3_OBJDETECT_INCLUDE_DIR}
    ${OpenCV3_HIGHGUI_INCLUDE_DIR}
    ${OpenCV3_ML_INCLUDE_DIR}
    ${OpenCV3_VIDEO_INCLUDE_DIR}
    )

SET(OpenCV3_LIBRARIES
    ${OpenCV3_CORE_LIBRARY}
    ${OpenCV3_IMGPROC_LIBRARY}
    ${OpenCV3_FEATURES2D_LIBRARY}
    ${OpenCV3_FLANN_LIBRARY}
    ${OpenCV3_CALIB3D_LIBRARY}
    ${OpenCV3_OBJDETECT_LIBRARY}
    ${OpenCV3_HIGHGUI_LIBRARY}
    ${OpenCV3_ML_LIBRARY}
    ${OpenCV3_VIDEO_LIBRARY}
    )
IF(WIN32)
    SET(OpenCV3_INCLUDE_DIRS
        ${OpenCV3_ROOT_DIR}/include
        ${OpenCV3_INCLUDE_DIRS}
        )
    SET(OpenCV3_LIBRARIES
        ${OpenCV3_LIBRARIES}
        ${OpenCV3_FFMPEG_LIBRARY}
        )
ENDIF(WIN32)

SET(OpenCV3_FOUND ON)
FOREACH(NAME ${OpenCV3_INCLUDE_DIRS})
    IF(NOT EXISTS ${NAME})
        SET(OpenCV3_FOUND OFF)
    ENDIF(NOT EXISTS ${NAME})
ENDFOREACH(NAME)
FOREACH(NAME ${OpenCV3_LIBRARIES})
    IF(NOT EXISTS ${NAME})
        SET(OpenCV3_FOUND OFF)
    ENDIF(NOT EXISTS ${NAME})
ENDFOREACH(NAME)

MARK_AS_ADVANCED(FORCE
                 OpenCV3_ROOT_DIR
                 OpenCV3_CORE_INCLUDE_DIR
                 OpenCV3_IMGPROC_INCLUDE_DIR
                 OpenCV3_FEATURES2D_INCLUDE_DIR
                 OpenCV3_FLANN_INCLUDE_DIR
                 OpenCV3_CALIB3D_INCLUDE_DIR
                 OpenCV3_OBJDETECT_INCLUDE_DIR
                 OpenCV3_HIGHGUI_INCLUDE_DIR
                 OpenCV3_ML_INCLUDE_DIR
                 OpenCV3_VIDEO_INCLUDE_DIR
                 OpenCV3_CORE_LIBRARY
                 OpenCV3_IMGPROC_LIBRARY
                 OpenCV3_FEATURES2D_LIBRARY
                 OpenCV3_FLANN_LIBRARY
                 OpenCV3_CALIB3D_LIBRARY
                 OpenCV3_OBJDETECT_LIBRARY
                 OpenCV3_HIGHGUI_LIBRARY
                 OpenCV3_ML_LIBRARY
                 OpenCV3_VIDEO_LIBRARY
                 )
IF(WIN32)
    MARK_AS_ADVANCED(FORCE
                     OpenCV3_FFMPEG_LIBRARY
                     )
ENDIF(WIN32)

# display help message
IF(NOT OpenCV3_FOUND)
    # make FIND_PACKAGE friendly
    IF(OpenCV3_FIND_REQUIRED)
        MESSAGE(FATAL_ERROR "OpenCV 3 not found. Please specify it's location with the OpenCV3_ROOT_DIR env. variable.")
    ELSE(OpenCV3_FIND_REQUIRED)
        MESSAGE(STATUS "OpenCV 3 not found.")
    ENDIF(OpenCV3_FIND_REQUIRED)
ENDIF(NOT OpenCV3_FOUND)
