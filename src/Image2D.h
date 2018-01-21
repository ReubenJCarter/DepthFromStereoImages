#pragma once 

#include "CommonHeader.h"

class Image2D
{		
	protected:
		uint64_t width;
		uint64_t height;
		void* data;
		
	public:
		static bool Init();
		Image2D();
		Image2D(uint64_t w, uint64_t h);
		~Image2D();
		void Allocate(uint64_t w, uint64_t h);
		void Deallocate();
		bool FromFile(std::string fileName);
		bool ToFile(std::string fileName);
		uint64_t Width();
		uint64_t Height();
		void* Data();
		void Copy(Image2D& sourceIm);
		cl_image_desc GetCLDiscriptor();
		inline uint8_t Pixel(uint64_t x, uint64_t y, uint64_t c)
		{
			return ((uint8_t*)data)[(width * y + x) * 4 + c];
		}
		
};

bool Image2D::Init()
{
	ilInit();
	ilEnable(IL_ORIGIN_SET);
	ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
	return true;
}

Image2D::Image2D()
{
	Init(); 
	width = 0;
	height = 0;
	data = 0;
}

Image2D::Image2D(uint64_t w, uint64_t h)
{
	Allocate(w, h);
}

Image2D::~Image2D()
{
	Deallocate();
}


void Image2D::Allocate(uint64_t w, uint64_t h)
{
	width = w;
	height = h;
	data = new uint8_t[w * h * 4];
}

void Image2D::Deallocate()
{
	if(width * height > 0)
	{
		delete[] (uint8_t*)data;
		width = 0;
		height = 0;
	}
}

bool Image2D::FromFile(std::string fileName)
{
	//make new deil image and load from file name
	ILuint ilIm;
	ilGenImages(1, &ilIm);
	ilBindImage(ilIm);
	bool loadedImageOK;
	loadedImageOK = ilLoadImage(fileName.c_str());
	if(loadedImageOK)
	{
		//get image info
		int widthIL = ilGetInteger(IL_IMAGE_WIDTH);
        int heightIL = ilGetInteger(IL_IMAGE_HEIGHT);
		
		//convert data to B4 type and format
		ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE); 
		Deallocate();
		
		//copy data
		uint8_t* dataPtr = ilGetData();
		if(dataPtr == NULL)
		{
			loadedImageOK = false;
			std::cerr << "Image2D:" << "No data for image:" << fileName << std::endl;
		}
		else
		{
			Allocate(widthIL, heightIL);
			for(unsigned int i = 0; i < width * height * 4; i++)
			{
				((uint8_t*)data)[i] = dataPtr[i];
			}
		}
	}
	else
	{
		std::cerr << "Image2D:" << "Could not load image file:" << fileName << std::endl;
	}
	ilBindImage(0);
	ilDeleteImages(1, &ilIm);
	ilBindImage(ilIm);
	
	
	return loadedImageOK;
}

bool Image2D::ToFile(std::string fileName)
{
	bool loadOK = false;
	if(width > 0 && height > 0)
	{
		ILuint ilIm;
		ilGenImages(1, &ilIm);
		ilBindImage(ilIm);
		ILboolean success = false;
		
		success = ilTexImage(width, height, 1, 4, IL_RGBA, IL_UNSIGNED_BYTE, data);
			
		if(success != true)
		{
			std::cerr << "Image2D:" << "failed to copy image data for saving" << std::endl;
		}
		else
		{
			ilEnable(IL_FILE_OVERWRITE);
			std::cout << "Image2D:Saving File:" << fileName.c_str() << std::endl;
			ilSaveImage(fileName.c_str());
			ilDisable(IL_FILE_OVERWRITE);
			loadOK = true;
		}
		ilDeleteImages(1, &ilIm);
	}
	else
	{
		std::cerr << "Image2D:" << "no data in image" << std::endl;
	}
	return loadOK;
}

uint64_t Image2D::Width()
{
	return width;
}

uint64_t Image2D::Height()
{
	return height;
}

void* Image2D::Data()
{
	return data;
}

void Image2D::Copy(Image2D& sourceIm)
{
	uint8_t* srcData = (uint8_t*)sourceIm.Data();
	
	for(uint64_t i = 0; i < Width() * Height() * 4; i++)
	{
		((uint8_t*)data)[i] = srcData[i];
	}
}

cl_image_desc Image2D::GetCLDiscriptor()
{
	cl_image_desc imageDiscriptorCL;
	imageDiscriptorCL.image_type = CL_MEM_OBJECT_IMAGE2D; 
	imageDiscriptorCL.image_width = Width();
	imageDiscriptorCL.image_height = Height();
	imageDiscriptorCL.image_depth = 1;
	imageDiscriptorCL.image_row_pitch = Width() * 4; 
	imageDiscriptorCL.image_slice_pitch = Height() * Width() * 4; 
	imageDiscriptorCL.num_mip_levels = 0;
	imageDiscriptorCL.num_samples = 0; 
	imageDiscriptorCL.buffer = NULL; 
	return imageDiscriptorCL; 
}