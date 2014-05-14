cmake_minimum_required(VERSION 2.8)
set(APP_NAME VolumeRenderGLES30)
project(${APP_NAME})


if(IOS)
#you could call cmake . -GXcode -DIOS=1 to get this to be true
MESSAGE("IN IOS")
endif(IOS)
  
MESSAGE("THIS WILL NOT COMPILE ON THE SIMULATOR.  ONLY ON DEVICES")

#Supress ZERO_CHECK target in IDE's
set(CMAKE_SUPPRESS_REGENERATION TRUE)

set(SOURCES
    Classes/OpenGLRenderer.h
    Classes/OpenGLRenderer.m
    Classes/EAGLView.h
    Classes/EAGLView.m
    Classes/ES3Renderer.h
    Classes/ES3Renderer.m
    Classes/VolumeRenderIOSAppDelegate.h
    Classes/VolumeRenderIOSAppDelegate.m
    Classes/MainViewController.h
    Classes/MainViewController.m
    UtilSrc/glUtil.h
    UtilSrc/imageUtil.h
    UtilSrc/matrixUtil.h
    UtilSrc/modelUtil.h
    UtilSrc/sourceUtil.h
    UtilSrc/vectorUtil.h
    UtilSrc/imageUtil.m
    UtilSrc/matrixUtil.c
    UtilSrc/modelUtil.c
    UtilSrc/sourceUtil.c
    UtilSrc/vectorUtil.c
    main.m
)

set(SHADERS
    Shaders/character.vsh
    Shaders/character.fsh
    Shaders/reflect.vsh
    Shaders/reflect.fsh
)

set(PLIST_FILE
    ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Template-Info.plist
)
set(RESOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Resources/demon.model
    ${CMAKE_CURRENT_SOURCE_DIR}/Resources/demon.png
    ${CMAKE_CURRENT_SOURCE_DIR}/Resources/MainWindow-iPad.xib
    ${CMAKE_CURRENT_SOURCE_DIR}/Resources/MainWindow.xib
)


set(FRAMEWORKS
    QuartzCore
    CoreGraphics
    Foundation
    UIKit
    OpenGLES
)


include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/Classes
    ${CMAKE_CURRENT_SOURCE_DIR}/UtilSrc
)

add_executable(
    ${APP_NAME}
    MACOSX_BUNDLE
    ${SOURCES}
    ${SHADERS}
    ${RESOURCES}
)


#this has to be after add_executable because that sets the target.
set_target_properties(${APP_NAME} PROPERTIES XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iPhone Developer")
#set_target_properties(${APP_NAME} PROPERTIES XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY[variant=Debug] "iPhone Developer")
set_target_properties(${APP_NAME} PROPERTIES XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET "7.1")
set_target_properties(${APP_NAME} PROPERTIES XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY 2) #1 - iPhone, 2 - iPad, 3 - Universal?
set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.blackicegames.${APP_NAME}")

#Put files into resources so they copy to the app when built
set_source_files_properties(
    ${SHADERS}
    PROPERTIES
    MACOSX_PACKAGE_LOCATION Resources
)

#Put files into resources so they copy to the app when built
set_source_files_properties(
    ${RESOURCES}
    PROPERTIES
    MACOSX_PACKAGE_LOCATION Resources
)

#This is a template for Info.plist.  Cmake variables in the file 
#will be replaced with their values.
set_target_properties(
    ${APP_NAME}
    PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST
    ${PLIST_FILE}
    )

#THIS IS TO SETUP ALL THE PATHS TO SDK's
###############################################################################
# Setup iOS developer location
# Because the Platform was chosen to be iPhoneOS.platform this will only compile
# on the device... not the simulator.
  set (IOS_PLATFORM iPhoneOS) #Device...not simulator

  if( ${IOS_PLATFORM} STREQUAL iPhoneOS)
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -arch armv7")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -arch armv7")
    SET(CMAKE_OSX_ARCHITECTURES "$(ARCHS_STANDARD)") #Choosing default arch.
    set(CMAKE_OSX_SYSROOT iphoneos) #FORCING DEVICE NOT SIMULATOR
  else()
  endif()

	set (CMAKE_IOS_DEVELOPER_ROOT "/Applications/Xcode.app/Contents/Developer/Platforms/${IOS_PLATFORM}.platform/Developer")
# Find and use the most recent iOS sdk 
if (NOT DEFINED CMAKE_IOS_SDK_ROOT)
	file (GLOB _CMAKE_IOS_SDKS "${CMAKE_IOS_DEVELOPER_ROOT}/SDKs/*")
	if (_CMAKE_IOS_SDKS) 
		list (SORT _CMAKE_IOS_SDKS)
		list (REVERSE _CMAKE_IOS_SDKS)
		list (GET _CMAKE_IOS_SDKS 0 CMAKE_IOS_SDK_ROOT) #Assign CMAKE_IOS_SDK_ROOT
	else (_CMAKE_IOS_SDKS)
		message (FATAL_ERROR "No iOS SDK's found in default seach path ${CMAKE_IOS_DEVELOPER_ROOT}. Manually set CMAKE_IOS_SDK_ROOT or install the iOS SDK.")
	endif (_CMAKE_IOS_SDKS)
	message (STATUS "Toolchain using default iOS SDK: ${CMAKE_IOS_SDK_ROOT}")
endif (NOT DEFINED CMAKE_IOS_SDK_ROOT)
#set (CMAKE_IOS_SDK_ROOT ${CMAKE_IOS_SDK_ROOT} CACHE PATH "Location of the selected iOS SDK")

# Set the find root to the iOS developer roots and to user defined paths
#set (CMAKE_FIND_ROOT_PATH ${CMAKE_IOS_DEVELOPER_ROOT} ${CMAKE_IOS_SDK_ROOT} ${CMAKE_PREFIX_PATH} CACHE string  "iOS find search path root")
set (CMAKE_FIND_ROOT_PATH ${CMAKE_IOS_DEVELOPER_ROOT} ${CMAKE_IOS_SDK_ROOT} ${CMAKE_PREFIX_PATH})

# default to searching for frameworks first
set (CMAKE_FIND_FRAMEWORK FIRST)

message("CMAKE_IOS_SDK_ROOT  " ${CMAKE_IOS_SDK_ROOT})
#The section below looks like overkill.  So commented out.  But kept just in-
#case it isn't.
# set up the default search directories for frameworks
# USED BY ADD_FRAMEWORK MACRO!
#set (CMAKE_SYSTEM_FRAMEWORK_PATH
#	${CMAKE_IOS_SDK_ROOT}/Developer/Library/Frameworks
#	${CMAKE_IOS_SDK_ROOT}/System/Library/Frameworks
#	${CMAKE_IOS_SDK_ROOT}/System/Library/PrivateFrameworks
#)
#
## only search the iOS sdks, not the remainder of the host filesystem
#set (CMAKE_FIND_ROOT_PATH_MODE_PROGRAM ONLY)
#set (CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
#set (CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
###############################################################################

##AS YOU CAN TELL BY THE NAME OF THE MACRO THIS IS A BAD WAY TO DO THIS.
#macro(ADD_FRAMEWORK_STUPID fwname appname)
##This is simpler than ADD_FRAMEWORK but you have to hard code the sdk and the 
##subdirectories off of it.
#
#  set(sdk "iPhoneOS7.1.sdk")
#  set(sdk_path "/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/")
#  set(sdk_fw_suffix "System/Library/Frameworks/")
#
#  include_directories(${sdk_fw_suffix}${fwname}.framework/Headers)
##target_link_libraries(${APP_NAME} ${sdk_path}${sdk}/${sdk_fw_suffix}${fwname}.framework)
#  target_link_libraries(${APP_NAME} ${sdk_path}${sdk}/${sdk_fw_suffix}${fwname}.framework)
#
#  MESSAGE(STATUS "Framework ${fwname} set to link to ${sdk_path}${sdk}/${sdk_fw_suffix}${fwname}.framework")
#endmacro(ADD_FRAMEWORK_STUPID)

#set(CMAKE_IOS_SDK_ROOT "/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS7.1.sdk")
macro(ADD_FRAMEWORK fwname appname)
#THIS IS LIKE CALLING find_library(FRAMEWORK_IOKIT IOkit).  FRAMEWORK_IOKIT will
#store the path to the framework.
  find_library(FRAMEWORK_${fwname} ${fwname})
#We don't need the command below because the SDK and search paths were setup
#
#  find_library(FRAMEWORK_${fwname}
#    NAMES ${fwname}
#    PATHS ${CMAKE_SYSTEM_FRAMEWORK_PATH}
#    PATH_SUFFIXES Frameworks
#    )
  if( ${FRAMEWORK_${fwname}} STREQUAL FRAMEWORK_${fwname}-NOTFOUND)
    MESSAGE(ERROR ": Framework ${fwname} not found")
  else()
    include_directories("${FRAMEWORK_${fwname}}.framework/Headers")
    TARGET_LINK_LIBRARIES(${APP_NAME} "${FRAMEWORK_${fwname}}")
    MESSAGE(STATUS "Framework ${fwname} found at ${FRAMEWORK_${fwname}}")
  endif()
endmacro(ADD_FRAMEWORK)


#these have to be after add_executable
#ADD ALL OF THE FRAMEWORKS
foreach(fw ${FRAMEWORKS} )
  add_framework(${fw} ${APP_NAME})
endforeach( fw ) 

MESSAGE("THIS WILL NOT COMPILE ON THE SIMULATOR.  ONLY ON DEVICES")
