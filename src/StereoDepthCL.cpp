#include "CommonHeader.h"
#include "CLHelper.h"
#include "Image2D.h"
#include "ImageDepth.h"


int main(int argc, char *argv[])
{
	if(argc < 3)
	{
		std::cerr << "not enough args imageL imageR" << std::endl;
		return 1; 
	}
	//set the block width and heigth variables
	int blockW = 4;
	int blockH = 4;
	
	//Load Images from files
	
	Image2D imageL;
	Image2D imageR;
	std::string fnL = argv[1];
	std::string fnR = argv[2];
	std::cout << "loading:" << argv[1] << std::endl;
	imageL.FromFile(fnL);
	std::cout << "loading:" << argv[2] << std::endl;
	imageR.FromFile(fnR);
	
	std::cout << "Hi" << std::endl; 
	
	//Init OpenCL
	cl_device_id deviceId; 
	cl_platform_id platformId;
	cl_context ctx;
	cl_command_queue queue;
	bool ok = InitCLFirstGPU(&deviceId, &platformId, &ctx, &queue); 
	
	if(!ok)
	{
		std::cerr << "Failed to init cl" << std::endl; 
		return 0; 
	}

	
	//Create OpenCL input images
	cl_image_format imageFormatCL;
	imageFormatCL.image_channel_order = CL_RGBA;
	imageFormatCL.image_channel_data_type = CL_UNORM_INT8;
	
	cl_int err; 
	cl_image_desc imageDescL = imageL.GetCLDiscriptor();
	cl_mem leftCl = clCreateImage(ctx, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, &imageFormatCL, &imageDescL, imageL.Data(), &err);
	if(err != CL_SUCCESS) std::cerr << "err:clCreateImage:leftCl:"<< GetClErrorString(err) << std::endl;
	cl_image_desc imageDescR = imageR.GetCLDiscriptor();
	cl_mem rightCl = clCreateImage(ctx, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, &imageFormatCL, &imageDescR, imageR.Data(), &err);
	if(err != CL_SUCCESS) std::cerr << "err:clCreateImage:rightCl:"<< GetClErrorString(err) << std::endl;
	
	//Create OpenCL output image
	cl_image_format imageDepthFormatCL;
	imageDepthFormatCL.image_channel_order = CL_LUMINANCE;
	imageDepthFormatCL.image_channel_data_type = CL_UNORM_INT16;
	
	ImageDepth imageOutput;
	imageOutput.Allocate(imageL.Width(), imageL.Height());
	
	cl_image_desc imageDescOut = imageOutput.GetCLDiscriptor();
	imageDescOut.image_row_pitch = 0;
	imageDescOut.image_slice_pitch = 0;
	cl_mem outCl = clCreateImage(ctx, CL_MEM_WRITE_ONLY, &imageDepthFormatCL, &imageDescOut, NULL, &err);
	if(err != CL_SUCCESS) std::cerr << "err:clCreateImage:outCl:"<< GetClErrorString(err) << std::endl;
	
	
	//Create OpenCL program
	std::string programSrc = ""
	"#define blockW 4\n"
	"#define blockH 4\n"
	"#define blockTW 9\n"
	"#define blockA 81\n"
	"#define maxK 200\n"
	"#define camSeparation 0.62f\n"
	"#define camFocal 0.1f\n"
	
	
	"__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;"
	
	
	"float AvPixColor(float4 pix)"
	"{"
	"	return (pix.x + pix.y + pix.z) / 3.0f;"
	"}"
	
	
	"float DepthFromCorrespondence(float cor)"
	"{"
	"	return 1.0f - (camSeparation * camFocal) / cor;"
	"}"
	
	
	"__kernel void ImageSAD(__read_only image2d_t leftImage, __read_only image2d_t rightImage, __write_only image2d_t sadImage)"
	"{"
	"	const int2 pos = {get_global_id(0), get_global_id(1)};"	
	"	float cache[blockA];"
	"	for(int i = -blockW; i <= blockW; i++)"
	"	{"
	"		for(int j = -blockH; j <= blockH; j++)"
	"		{"
	"			int2 posL = (int2)(pos.x + i, pos.y + j);"
	"			float4 pixl = read_imagef(leftImage, sampler, posL);"
	"			cache[(i + blockW) * blockTW + j + blockH] = AvPixColor(pixl);"
	"		}"
	"	}"
	"	float lowestsad = 1000.0f;"
	" 	int lowestsadK = 0;"
	"	for(int k = 0; k < maxK; k++)"
	"	{"
	"		float sad = 0;"
	"		for(int i = -blockW; i <= blockW; i++)"
	"		{"
	"			for(int j = -blockH; j <= blockH; j++)"
	"			{"
	"				int2 posL = (int2)(pos.x + i, pos.y + j);"
	"				int2 posR = (int2)(pos.x - k + i, pos.y + j);"
	"				float4 pixl = cache[(i + blockW) * 9 + j + blockH];"
	"				float4 pixr = read_imagef(rightImage, sampler, posR);"
	"				float avpixl = AvPixColor(pixl);"
	"				float avpixr = AvPixColor(pixr);"
	"				float pixAbsDiff = fabs(avpixl - avpixr);"
	"				sad += pixAbsDiff;"
	"			}"
	"		}"
	"		if(sad < lowestsad)"
	"		{"
	"			lowestsad = sad;"
	"			lowestsadK = k;"
	"		}"
	"	}"
	"	float finalPix = (float)lowestsadK / (float)maxK;"
	"	finalPix = DepthFromCorrespondence(finalPix);"
	"	write_imagef(sadImage, (int2)(pos.x, pos.y), (float4)(finalPix, finalPix, finalPix, 1.0f));"
	"}";
	cl_program  clProgram;
	CreateCLProgram(programSrc, ctx, deviceId, &clProgram); 
	
	//get Kernel
	cl_kernel kernel = clCreateKernel(clProgram, "ImageSAD", &err);
	if(err != CL_SUCCESS) 
	{
		std::cerr << "err:"<< GetClErrorString(err) << std::endl;
		return 0;
	}
	
	//Execute kernel
	cl_event event = NULL;
	err = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&leftCl);
	if(err != CL_SUCCESS) std::cerr << "err:clSetKernelArg:leftCl:"<< GetClErrorString(err) << std::endl;
	err = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&rightCl);
	if(err != CL_SUCCESS) std::cerr << "err:clSetKernelArg:rightCl:"<< GetClErrorString(err) << std::endl;
	err = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&outCl);
	if(err != CL_SUCCESS) std::cerr << "err:clSetKernelArg:outCl:"<< GetClErrorString(err) << std::endl;
	const size_t global_work_size[] = {imageOutput.Width(), imageOutput.Height()};
	const size_t local_work_size[] = {1, 1};
	err = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, global_work_size, local_work_size, 0, NULL, &event);
	if(err != CL_SUCCESS) std::cerr << "err:clEnqueueNDRangeKernel:outCl:"<< GetClErrorString(err) << std::endl;
	err = clWaitForEvents(1, &event);
	
	//read back image
	const size_t offset[] ={0, 0, 0};
	const size_t range[] = {imageOutput.Width(), imageOutput.Height(), 1};
	err = clEnqueueReadImage(queue, outCl, CL_TRUE, offset, range, imageOutput.Width() * 2, imageOutput.Width() * imageOutput.Height() * 2, imageOutput.Data(), 0, NULL, NULL);
	
	//Save Image out
	imageOutput.ToFile("output.png");
	
	return 1;
}