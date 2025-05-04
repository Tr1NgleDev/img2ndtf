#include <iostream>
#include <ndtf/ndtf.h>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_HDR
#define STBI_NO_GIF
#define STBI_NO_PNM
#define STBI_MAX_DIMENSIONS (1 << 30)
#include "libs/stb_image.h"

#include <string>
#include <format>
#include <filesystem>

int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		printf(	"Usage: %s <input image> <output ndtf> [flags]\n"
				"Converts an image (jpeg/png/tga/bmp/psd/pic) to an NDTF (N-Dimensional Texture Format).\n"
				"The channel format is always 8-bit.\n"
				"Flags:\n"
				"\t-d <depth> - Specifies the 3rd size of the image. In the input is expected to be going down. Default = 1.\n"
				"\t-i <ind> - Specifies the 4th size of the image. In the input is expected to be going to the right. Default = 1.\n"
				"\t-i2 <ind> - Specifies the 5th size of the image. In the input is expected to be going down. Default = 1.\n"
				"\t-c <channels (1/3/4)> - Specifies the desired amount of channels. R, RGB or RGBA. Default = Matches the input.\n"
				"\t-z - Apply ZLib compression.\n"
			, argv[0]);
		return 1;
	}

	int depth = 1;
	int ind = 1;
	int ind2 = 1;
	int desiredChannels = 4;
	bool zlibCompression = false;

	const char* input = argv[1];
	const char* output = argv[2];
	if (!std::filesystem::exists(input))
	{
		printf("File \"%s\" does not exist!\n", input);
		return 2;
	}

	for (int i = 3; i < argc; ++i)
	{
		if (!strcmp(argv[i], "-d"))
		{
			++i;
			char* n = argv[i];
			depth = std::max(std::stoi(n), 1);
		}
		if (!strcmp(argv[i], "-i"))
		{
			++i;
			char* n = argv[i];
			ind = std::max(std::stoi(n), 1);
		}
		if (!strcmp(argv[i], "-i2"))
		{
			++i;
			char* n = argv[i];
			ind2 = std::max(std::stoi(n), 1);
		}
		if (!strcmp(argv[i], "-c"))
		{
			++i;
			char* n = argv[i];
			desiredChannels = std::max(std::stoi(n), 1);
		}
		if (!strcmp(argv[i], "-z"))
		{
			zlibCompression = true;
		}
	}

	int iW, iH, iC;
	uint8_t* data = stbi_load(input, &iW, &iH, &iC, 0);
	desiredChannels = iC;

	if (!data)
	{
		printf("Failed to load the input image at \"%s\".\n", input);
		return 3;
	}

	int width = iW / ind;
	int height = iH / depth / ind2;

	NDTF_TexelFormat format;
	switch (iC)
	{
	case 1:
	{
		format = NDTF_TEXELFORMAT_R8;
	} break;
	case 3:
	{
		format = NDTF_TEXELFORMAT_RGB888;
	} break;
	case 4:
	{
		format = NDTF_TEXELFORMAT_RGBA8888;
	} break;
	default:
	{
		printf("Input %i amount of color channels is not supported.\n", iC);
		stbi_image_free(data);
		return 4;
	}
	}

	NDTF_TexelFormat desiredFormat;
	switch (desiredChannels)
	{
	case 1:
	{
		desiredFormat = NDTF_TEXELFORMAT_R8;
	} break;
	case 3:
	{
		desiredFormat = NDTF_TEXELFORMAT_RGB888;
	} break;
	case 4:
	{
		desiredFormat = NDTF_TEXELFORMAT_RGBA8888;
	} break;
	default:
	{
		printf("Output %i amount of color channels is not supported.\n", desiredChannels);
		stbi_image_free(data);
		return 5;
	}
	}

	NDTF_File result;
	if (ind2 > 1)
		result = ndtf_file_create_5D(format, width, height, depth, ind, ind2);
	else if (ind > 1)
		result = ndtf_file_create_4D(format, width, height, depth, ind);
	else if (depth > 1)
		result = ndtf_file_create_3D(format, width, height, depth);
	else
		result = ndtf_file_create_2D(format, width, height);

	uint8_t color[4]{ 0 };
	NDTF_Coord coord{ 0 };

	for (size_t xa = 0; xa < iW; ++xa)
	{
		for (size_t ya = 0; ya < iH; ++ya)
		{
			coord.x = xa % width;
			coord.y = ya % height;
			coord.z = (ya / height) % depth;
			coord.w = (xa / width) % ind;
			coord.v = (ya / (depth * height)) % ind2;

			size_t iInd = (xa + ya * iW) * iC;

			for (uint8_t c = 0; c < iC; ++c)
			{
				color[c] = data[iInd + c];
			}

			ndtf_file_setTexel(&result, &coord, color);
		}
	}

	ndtf_file_setZLibCompression(&result, zlibCompression);

	ndtf_file_reformat(&result, desiredFormat);
	if (!ndtf_file_save(&result, output))
	{
		printf("Failed to save the result ndtf to \"%s\"\n", output);
	}
	else
	{
		printf("Saved the result ndtf at \"%s\"\n", output);
	}

	ndtf_file_free(&result);
	stbi_image_free(data);

	return 0;
}
