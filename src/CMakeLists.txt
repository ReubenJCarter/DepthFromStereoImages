
set(StereoDepthCL
	StereoDepthCL.cpp
)	

add_executable(StereoDepthCL ${StereoDepthCL})

target_link_libraries(StereoDepthCL DevIL)
target_link_libraries(StereoDepthCL OpenCL)