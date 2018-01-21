#pragma once 

#include "CommonHeader.h"

class ImageDepth
{		
	protected:
		uint64_t width;
		uint64_t height;
		void* data;
		
	public:
		static bool Init();
		ImageDepth();
		ImageDepth(uint64_t w, uint64_t h);
		~ImageDepth();
		void Allocate(uint64_t w, uint64_t h);
		void Deallocate();
		bool FromFile(std::string fileName);
		bool ToFile(std::string fileName);
		uint64_t Width();
		uint64_t Height();
		void* Data();
		cl_image_desc GetCLDiscriptor();
};

bool ImageDepth::Init()
{
	ilInit();
	ilEnable(IL_ORIGIN_SET);
	ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
	return true;
}

ImageDepth::ImageDepth()
{
	Init(); 
	width = 0;
	height = 0;
	data = 0;
}

ImageDepth::ImageDepth(uint64_t w, uint64_t h)
{
	Allocate(w, h);
}

ImageDepth::~ImageDepth()
{
	Deallocate();
}


void ImageDepth::Allocate(uint64_t w, uint64_t h)
{
	width = w;
	height = h;
	data = new uint16_t[w * h];
}

void ImageDepth::Deallocate()
{
	if(width * height > 0)
	{
		delete[] (uint16_t*)data;
		width = 0;
		height = 0;
	}
}

bool ImageDepth::FromFile(std::string fileName)
{
	return false;
}

bool ImageDepth::ToFile(std::string fileName)
{
	bool loadOK = false;
	if(width > 0 && height > 0)
	{
		ILuint ilIm;
		ilGenImages(1, &ilIm);
		ilBindImage(ilIm);
		ILboolean success = false;
		
		success = ilTexImage(width, height, 1, 1, IL_LUMINANCE, IL_UNSIGNED_SHORT, data);
			
		if(success != true)
		{
			std::cerr << "ImageDepth:" << "failed to copy image data for saving" << std::endl;
		}
		else
		{
			ilEnable(IL_FILE_OVERWRITE);
			std::cout << "ImageDepth:Saving File:" << fileName.c_str() << std::endl;
			ilSaveImage(fileName.c_str());
			ilDisable(IL_FILE_OVERWRITE);
			loadOK = true;
		}
		ilDeleteImages(1, &ilIm);
	}
	else
	{
		std::cerr << "ImageDepth:" << "no data in image" << std::endl;
	}
	return loadOK;
}

uint64_t ImageDepth::Width()
{
	return width;
}

uint64_t ImageDepth::Height()
{
	return height;
}

void* ImageDepth::Data()
{
	return data;
}

cl_image_desc ImageDepth::GetCLDiscriptor()
{
	cl_image_desc imageDiscriptorCL;
	imageDiscriptorCL.image_type = CL_MEM_OBJECT_IMAGE2D; 
	imageDiscriptorCL.image_width = Width();
	imageDiscriptorCL.image_height = Height();
	imageDiscriptorCL.image_depth = 1;
	imageDiscriptorCL.image_row_pitch = Width() * 2; 
	imageDiscriptorCL.image_slice_pitch = Height() * Width() * 2; 
	imageDiscriptorCL.num_mip_levels = 0;
	imageDiscriptorCL.num_samples = 0; 
	imageDiscriptorCL.buffer = NULL; 
	return imageDiscriptorCL; 
}