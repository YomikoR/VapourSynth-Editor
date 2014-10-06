#include "../common/helpers.h"
#include "../common/image.h"

#include "pixmapfromframe.h"

//==============================================================================

QPixmap vsedit::pixmapFromGray1B(const VSAPI * a_cpVSAPI,
	const VSFormat * a_cpFormat, const VSFrameRef * a_cpFrameRef)
{
	(void)a_cpFormat;

	int width = a_cpVSAPI->getFrameWidth(a_cpFrameRef, 0);
	int height = a_cpVSAPI->getFrameHeight(a_cpFrameRef, 0);
	const uint8_t * cpRead = a_cpVSAPI->getReadPtr(a_cpFrameRef, 0);
	int stride = a_cpVSAPI->getStride(a_cpFrameRef, 0);

	std::vector<vsedit::RGB32> image(width * height);
	size_t i = 0;
	const uint8_t * cpLine = cpRead;
	for(size_t h = 0; h < (size_t)height; ++h)
	{
		for(size_t w = 0; w < (size_t)width; ++w)
		{
			image[i].parts.r = cpLine[w];
			image[i].parts.g = cpLine[w];
			image[i].parts.b = cpLine[w];
			image[i].parts.x = 255;
			i++;
		}
		cpLine += stride;
	}

	QImage frameImage((const uchar *)image.data(), width, height,
		QImage::Format_RGB32);
	QPixmap framePixmap = QPixmap::fromImage(frameImage).copy();

	return framePixmap;
}

// END OF QPixmap vsedit::pixmapFromGray1B(const VSAPI * a_cpVSAPI,
//		const VSFormat * a_cpFormat, const VSFrameRef * a_cpFrameRef)
//==============================================================================

QPixmap vsedit::pixmapFromGray2B(const VSAPI * a_cpVSAPI,
	const VSFormat * a_cpFormat, const VSFrameRef * a_cpFrameRef)
{
	(void)a_cpFormat;

	int width = a_cpVSAPI->getFrameWidth(a_cpFrameRef, 0);
	int height = a_cpVSAPI->getFrameHeight(a_cpFrameRef, 0);
	const uint8_t * cpRead = a_cpVSAPI->getReadPtr(a_cpFrameRef, 0);
	int stride = a_cpVSAPI->getStride(a_cpFrameRef, 0);

	std::vector<vsedit::RGB32> image(width * height);
	size_t i = 0;
	const uint16_t * cpLine;
	uint8_t gray;
	for(size_t h = 0; h < (size_t)height; ++h)
	{
		cpLine = (const uint16_t *)cpRead;
		for(size_t w = 0; w < (size_t)width; ++w)
		{
			// Shift with binary rounding
			if(cpLine[w] >= 0xFF80)
				gray = cpLine[w] >> 8;
			else
				gray = (cpLine[w] + 0x0080) >> 8;
			image[i].parts.r = gray;
			image[i].parts.g = gray;
			image[i].parts.b = gray;
			image[i].parts.x = 255;
			i++;
		}
		cpRead += stride;
	}

	QImage frameImage((const uchar *)image.data(), width, height,
		QImage::Format_RGB32);
	QPixmap framePixmap = QPixmap::fromImage(frameImage).copy();

	return framePixmap;
}

// END OF QPixmap vsedit::pixmapFromGray2B(const VSAPI * a_cpVSAPI,
//		const VSFormat * a_cpFormat, const VSFrameRef * a_cpFrameRef)
//==============================================================================

QPixmap vsedit::pixmapFromGrayH(const VSAPI * a_cpVSAPI,
	const VSFormat * a_cpFormat, const VSFrameRef * a_cpFrameRef)
{
	(void)a_cpFormat;

	int width = a_cpVSAPI->getFrameWidth(a_cpFrameRef, 0);
	int height = a_cpVSAPI->getFrameHeight(a_cpFrameRef, 0);
	const uint8_t * cpRead = a_cpVSAPI->getReadPtr(a_cpFrameRef, 0);
	int stride = a_cpVSAPI->getStride(a_cpFrameRef, 0);

	std::vector<vsedit::RGB32> image(width * height);
	size_t i = 0;
	const uint16_t * cpLine;
	float gray;
	for(size_t h = 0; h < (size_t)height; ++h)
	{
		cpLine = (const uint16_t *)cpRead;
		for(size_t w = 0; w < (size_t)width; ++w)
		{
			vsedit::FP16 half;
			half.u = cpLine[w];
			gray = halfToSingle(half).f * 255.0f;
			clamp(gray, 0.0f, 255.0f);
			image[i].parts.r = gray;
			image[i].parts.g = gray;
			image[i].parts.b = gray;
			image[i].parts.x = 255;
			i++;
		}
		cpRead += stride;
	}

	QImage frameImage((const uchar *)image.data(), width, height,
		QImage::Format_RGB32);
	QPixmap framePixmap = QPixmap::fromImage(frameImage).copy();

	return framePixmap;
}

// END OF QPixmap vsedit::pixmapFromGrayH(const VSAPI * a_cpVSAPI,
//		const VSFormat * a_cpFormat, const VSFrameRef * a_cpFrameRef)
//==============================================================================

QPixmap vsedit::pixmapFromGrayS(const VSAPI * a_cpVSAPI,
	const VSFormat * a_cpFormat, const VSFrameRef * a_cpFrameRef)
{
	(void)a_cpFormat;

	int width = a_cpVSAPI->getFrameWidth(a_cpFrameRef, 0);
	int height = a_cpVSAPI->getFrameHeight(a_cpFrameRef, 0);
	const uint8_t * cpRead = a_cpVSAPI->getReadPtr(a_cpFrameRef, 0);
	int stride = a_cpVSAPI->getStride(a_cpFrameRef, 0);

	std::vector<vsedit::RGB32> image(width * height);
	size_t i = 0;
	const float * cpLine;
	float gray;
	for(size_t h = 0; h < (size_t)height; ++h)
	{
		cpLine = (const float *)cpRead;
		for(size_t w = 0; w < (size_t)width; ++w)
		{
			gray = cpLine[w] * 255.0f;
			clamp(gray, 0.0f, 255.0f);
			image[i].parts.r = gray;
			image[i].parts.g = gray;
			image[i].parts.b = gray;
			image[i].parts.x = 255;
			i++;
		}
		cpRead += stride;
	}

	QImage frameImage((const uchar *)image.data(), width, height,
		QImage::Format_RGB32);
	QPixmap framePixmap = QPixmap::fromImage(frameImage).copy();

	return framePixmap;
}

// END OF QPixmap vsedit::pixmapFromGrayS(const VSAPI * a_cpVSAPI,
//		const VSFormat * a_cpFormat, const VSFrameRef * a_cpFrameRef)
//==============================================================================

QPixmap vsedit::pixmapFromYUV1B(const VSAPI * a_cpVSAPI,
	const VSFormat * a_cpFormat, const VSFrameRef * a_cpFrameRef)
{
	int width = a_cpVSAPI->getFrameWidth(a_cpFrameRef, 0);
	int height = a_cpVSAPI->getFrameHeight(a_cpFrameRef, 0);
	const uint8_t * cpReadY = a_cpVSAPI->getReadPtr(a_cpFrameRef, 0);
	const uint8_t * cpReadU = a_cpVSAPI->getReadPtr(a_cpFrameRef, 1);
	const uint8_t * cpReadV = a_cpVSAPI->getReadPtr(a_cpFrameRef, 2);
	int strideY = a_cpVSAPI->getStride(a_cpFrameRef, 0);
	int strideU = a_cpVSAPI->getStride(a_cpFrameRef, 1);
	int strideV = a_cpVSAPI->getStride(a_cpFrameRef, 2);
	int subSamplingW = a_cpFormat->subSamplingW;
	int subSamplingH = a_cpFormat->subSamplingH;

	uint8_t * pUpsampledU = nullptr;
	uint8_t * pUpsampledV = nullptr;
	if( (subSamplingH != 0) || (subSamplingW != 0))
	{
		pUpsampledU = (uint8_t *)malloc(height * width);
		bicubicResize(cpReadU, width >> subSamplingW, height >> subSamplingH,
			strideU, pUpsampledU, width, height, width, (uint8_t)0, (uint8_t)255);
		cpReadU = pUpsampledU;
		strideU = width;
		pUpsampledV = (uint8_t *)malloc(height * width);
		bicubicResize(cpReadV, width >> subSamplingW, height >> subSamplingH,
			strideV, pUpsampledV, width, height, width, (uint8_t)0, (uint8_t)255);
		cpReadV = pUpsampledV;
		strideV = width;
	}

	std::vector<vsedit::RGB32> image(width * height);
	size_t i = 0;
	const uint8_t *cpLineY, *cpLineU, *cpLineV;
	for(size_t h = 0; h < (size_t)height; ++h)
	{
		cpLineY = cpReadY + strideY * h;
		cpLineU = cpReadU + strideU * h;
		cpLineV = cpReadV + strideV * h;
		for(size_t w = 0; w < (size_t)width; ++w)
		{
			image[i] = vsedit::yuvToRgb32(cpLineY[w],
				cpLineU[w], cpLineV[w]);
			i++;
		}
	}

	if(pUpsampledU)
		free(pUpsampledU);
	if(pUpsampledV)
		free(pUpsampledV);

	QImage frameImage((const uchar *)image.data(), width, height,
		QImage::Format_RGB32);
	QPixmap framePixmap = QPixmap::fromImage(frameImage).copy();

	return framePixmap;
}

// END OF QPixmap vsedit::pixmapFromYUV1B(const VSAPI * a_cpVSAPI,
//		const VSFormat * a_cpFormat, const VSFrameRef * a_cpFrameRef)
//==============================================================================

QPixmap vsedit::pixmapFromYUV2B(const VSAPI * a_cpVSAPI,
	const VSFormat * a_cpFormat, const VSFrameRef * a_cpFrameRef)
{
	int width = a_cpVSAPI->getFrameWidth(a_cpFrameRef, 0);
	int height = a_cpVSAPI->getFrameHeight(a_cpFrameRef, 0);
	const uint8_t * cpReadY = a_cpVSAPI->getReadPtr(a_cpFrameRef, 0);
	const uint8_t * cpReadU = a_cpVSAPI->getReadPtr(a_cpFrameRef, 1);
	const uint8_t * cpReadV = a_cpVSAPI->getReadPtr(a_cpFrameRef, 2);
	int strideY = a_cpVSAPI->getStride(a_cpFrameRef, 0);
	int strideU = a_cpVSAPI->getStride(a_cpFrameRef, 1);
	int strideV = a_cpVSAPI->getStride(a_cpFrameRef, 2);
	int subSamplingW = a_cpFormat->subSamplingW;
	int subSamplingH = a_cpFormat->subSamplingH;
	int bitsPerSample = a_cpFormat->bitsPerSample;

	std::vector<vsedit::RGB32> image(width * height);
	size_t i = 0;
	const uint16_t *cpLineY, *cpLineU, *cpLineV;
	for(size_t h = 0; h < (size_t)height; ++h)
	{
		cpLineY = (const uint16_t *)(cpReadY + strideY * h);
		cpLineU = (const uint16_t *)(cpReadU + strideU * (h >> subSamplingH));
		cpLineV = (const uint16_t *)(cpReadV + strideV * (h >> subSamplingH));
		for(size_t w = 0; w < (size_t)width; ++w)
		{
			image[i] = vsedit::yuvToRgb32(cpLineY[w],
				cpLineU[w >> subSamplingW], cpLineV[w >> subSamplingW],
				bitsPerSample);
			i++;
		}
	}

	QImage frameImage((const uchar *)image.data(), width, height,
		QImage::Format_RGB32);
	QPixmap framePixmap = QPixmap::fromImage(frameImage).copy();

	return framePixmap;
}

// END OF QPixmap vsedit::pixmapFromYUV2B(const VSAPI * a_cpVSAPI,
//		const VSFormat * a_cpFormat, const VSFrameRef * a_cpFrameRef)
//==============================================================================

QPixmap vsedit::pixmapFromYUVH(const VSAPI * a_cpVSAPI,
	const VSFormat * a_cpFormat, const VSFrameRef * a_cpFrameRef)
{
	int width = a_cpVSAPI->getFrameWidth(a_cpFrameRef, 0);
	int height = a_cpVSAPI->getFrameHeight(a_cpFrameRef, 0);
	const uint8_t * cpReadY = a_cpVSAPI->getReadPtr(a_cpFrameRef, 0);
	const uint8_t * cpReadU = a_cpVSAPI->getReadPtr(a_cpFrameRef, 1);
	const uint8_t * cpReadV = a_cpVSAPI->getReadPtr(a_cpFrameRef, 2);
	int strideY = a_cpVSAPI->getStride(a_cpFrameRef, 0);
	int strideU = a_cpVSAPI->getStride(a_cpFrameRef, 1);
	int strideV = a_cpVSAPI->getStride(a_cpFrameRef, 2);
	int subSamplingW = a_cpFormat->subSamplingW;
	int subSamplingH = a_cpFormat->subSamplingH;

	std::vector<vsedit::RGB32> image(width * height);
	size_t i = 0;
	const uint16_t *cpLineY, *cpLineU, *cpLineV;
	vsedit::FP16 halfY, halfU, halfV;
	for(size_t h = 0; h < (size_t)height; ++h)
	{
		cpLineY = (const uint16_t *)(cpReadY + strideY * h);
		cpLineU = (const uint16_t *)(cpReadU + strideU * (h >> subSamplingH));
		cpLineV = (const uint16_t *)(cpReadV + strideV * (h >> subSamplingH));
		for(size_t w = 0; w < (size_t)width; ++w)
		{
			halfY.u = cpLineY[w];
			halfU.u = cpLineU[w >> subSamplingW];
			halfV.u = cpLineV[w >> subSamplingW];
			image[i] = vsedit::yuvToRgb32(halfY, halfU, halfV);
			i++;
		}
	}

	QImage frameImage((const uchar *)image.data(), width, height,
		QImage::Format_RGB32);
	QPixmap framePixmap = QPixmap::fromImage(frameImage).copy();

	return framePixmap;
}

// END OF QPixmap vsedit::pixmapFromYUVH(const VSAPI * a_cpVSAPI,
//		const VSFormat * a_cpFormat, const VSFrameRef * a_cpFrameRef)
//==============================================================================

QPixmap vsedit::pixmapFromYUVS(const VSAPI * a_cpVSAPI,
	const VSFormat * a_cpFormat, const VSFrameRef * a_cpFrameRef)
{
	int width = a_cpVSAPI->getFrameWidth(a_cpFrameRef, 0);
	int height = a_cpVSAPI->getFrameHeight(a_cpFrameRef, 0);
	const uint8_t * cpReadY = a_cpVSAPI->getReadPtr(a_cpFrameRef, 0);
	const uint8_t * cpReadU = a_cpVSAPI->getReadPtr(a_cpFrameRef, 1);
	const uint8_t * cpReadV = a_cpVSAPI->getReadPtr(a_cpFrameRef, 2);
	int strideY = a_cpVSAPI->getStride(a_cpFrameRef, 0);
	int strideU = a_cpVSAPI->getStride(a_cpFrameRef, 1);
	int strideV = a_cpVSAPI->getStride(a_cpFrameRef, 2);
	int subSamplingW = a_cpFormat->subSamplingW;
	int subSamplingH = a_cpFormat->subSamplingH;

	std::vector<vsedit::RGB32> image(width * height);
	size_t i = 0;
	const float *cpLineY, *cpLineU, *cpLineV;
	for(size_t h = 0; h < (size_t)height; ++h)
	{
		cpLineY = (const float *)(cpReadY + strideY * h);
		cpLineU = (const float *)(cpReadU + strideU * (h >> subSamplingH));
		cpLineV = (const float *)(cpReadV + strideV * (h >> subSamplingH));
		for(size_t w = 0; w < (size_t)width; ++w)
		{
			image[i] = vsedit::yuvToRgb32(cpLineY[w],
				cpLineU[w >> subSamplingW], cpLineV[w >> subSamplingW]);
			i++;
		}
	}

	QImage frameImage((const uchar *)image.data(), width, height,
		QImage::Format_RGB32);
	QPixmap framePixmap = QPixmap::fromImage(frameImage).copy();

	return framePixmap;
}

// END OF QPixmap vsedit::pixmapFromYUVS(const VSAPI * a_cpVSAPI,
//		const VSFormat * a_cpFormat, const VSFrameRef * a_cpFrameRef)
//==============================================================================

QPixmap vsedit::pixmapFromRGB1B(const VSAPI * a_cpVSAPI,
	const VSFormat * a_cpFormat, const VSFrameRef * a_cpFrameRef)
{
	(void)a_cpFormat;

	int width = a_cpVSAPI->getFrameWidth(a_cpFrameRef, 0);
	int height = a_cpVSAPI->getFrameHeight(a_cpFrameRef, 0);
	const uint8_t * cpReadR = a_cpVSAPI->getReadPtr(a_cpFrameRef, 0);
	const uint8_t * cpReadG = a_cpVSAPI->getReadPtr(a_cpFrameRef, 1);
	const uint8_t * cpReadB = a_cpVSAPI->getReadPtr(a_cpFrameRef, 2);
	int strideR = a_cpVSAPI->getStride(a_cpFrameRef, 0);
	int strideG = a_cpVSAPI->getStride(a_cpFrameRef, 1);
	int strideB = a_cpVSAPI->getStride(a_cpFrameRef, 2);

	std::vector<vsedit::RGB32> image(width * height);
	size_t i = 0;
	for(size_t h = 0; h < (size_t)height; ++h)
	{
		for(size_t w = 0; w < (size_t)width; ++w)
		{
			image[i].parts.r = cpReadR[w];
			image[i].parts.g = cpReadG[w];
			image[i].parts.b = cpReadB[w];
			image[i].parts.x = 255;
			i++;
		}
		cpReadR += strideR;
		cpReadG += strideG;
		cpReadB += strideB;
	}

	QImage frameImage((const uchar *)image.data(), width, height,
		QImage::Format_RGB32);
	QPixmap framePixmap = QPixmap::fromImage(frameImage).copy();

	return framePixmap;
}

// END OF QPixmap vsedit::pixmapFromRGB1B(const VSAPI * a_cpVSAPI,
//		const VSFormat * a_cpFormat, const VSFrameRef * a_cpFrameRef)
//==============================================================================

QPixmap vsedit::pixmapFromRGB2B(const VSAPI * a_cpVSAPI,
	const VSFormat * a_cpFormat, const VSFrameRef * a_cpFrameRef)
{
	int width = a_cpVSAPI->getFrameWidth(a_cpFrameRef, 0);
	int height = a_cpVSAPI->getFrameHeight(a_cpFrameRef, 0);
	const uint8_t * cpReadR = a_cpVSAPI->getReadPtr(a_cpFrameRef, 0);
	const uint8_t * cpReadG = a_cpVSAPI->getReadPtr(a_cpFrameRef, 1);
	const uint8_t * cpReadB = a_cpVSAPI->getReadPtr(a_cpFrameRef, 2);
	int strideR = a_cpVSAPI->getStride(a_cpFrameRef, 0);
	int strideG = a_cpVSAPI->getStride(a_cpFrameRef, 1);
	int strideB = a_cpVSAPI->getStride(a_cpFrameRef, 2);
	int bitsPerSample = a_cpFormat->bitsPerSample;


	uint8_t shift = bitsPerSample - 8;
	uint16_t fraction = 1 << (shift - 1);
	uint16_t threshold = (1 << bitsPerSample) - fraction;

	std::vector<vsedit::RGB32> image(width * height);
	size_t i = 0;
	const uint16_t *cpLineR, *cpLineG, *cpLineB;
	for(size_t h = 0; h < (size_t)height; ++h)
	{
		cpLineR = (const uint16_t *)cpReadR;
		cpLineG = (const uint16_t *)cpReadG;
		cpLineB = (const uint16_t *)cpReadB;
		for(size_t w = 0; w < (size_t)width; ++w)
		{
			if(cpLineR[w] >= threshold)
				image[i].parts.r = cpLineR[w] >> shift;
			else
				image[i].parts.r = (cpLineR[w] + fraction) >> shift;

			if(cpLineG[w] >= threshold)
				image[i].parts.g = cpLineG[w] >> shift;
			else
				image[i].parts.g = (cpLineG[w] + fraction) >> shift;

			if(cpLineB[w] >= threshold)
				image[i].parts.b = cpLineB[w] >> shift;
			else
				image[i].parts.b = (cpLineB[w] + fraction) >> shift;

			image[i].parts.x = 255;
			i++;
		}
		cpReadR += strideR;
		cpReadG += strideG;
		cpReadB += strideB;
	}

	QImage frameImage((const uchar *)image.data(), width, height,
		QImage::Format_RGB32);
	QPixmap framePixmap = QPixmap::fromImage(frameImage).copy();

	return framePixmap;
}

// END OF QPixmap vsedit::pixmapFromRGB2B(const VSAPI * a_cpVSAPI,
//		const VSFormat * a_cpFormat, const VSFrameRef * a_cpFrameRef)
//==============================================================================

QPixmap vsedit::pixmapFromRGBH(const VSAPI * a_cpVSAPI,
	const VSFormat * a_cpFormat, const VSFrameRef * a_cpFrameRef)
{
	(void)a_cpFormat;

	int width = a_cpVSAPI->getFrameWidth(a_cpFrameRef, 0);
	int height = a_cpVSAPI->getFrameHeight(a_cpFrameRef, 0);
	const uint8_t * cpReadR = a_cpVSAPI->getReadPtr(a_cpFrameRef, 0);
	const uint8_t * cpReadG = a_cpVSAPI->getReadPtr(a_cpFrameRef, 1);
	const uint8_t * cpReadB = a_cpVSAPI->getReadPtr(a_cpFrameRef, 2);
	int strideR = a_cpVSAPI->getStride(a_cpFrameRef, 0);
	int strideG = a_cpVSAPI->getStride(a_cpFrameRef, 1);
	int strideB = a_cpVSAPI->getStride(a_cpFrameRef, 2);

	std::vector<vsedit::RGB32> image(width * height);
	size_t i = 0;
	const uint16_t *cpLineR, *cpLineG, *cpLineB;
	vsedit::FP16 half;
	float single;
	for(size_t h = 0; h < (size_t)height; ++h)
	{
		cpLineR = (const uint16_t *)cpReadR;
		cpLineG = (const uint16_t *)cpReadG;
		cpLineB = (const uint16_t *)cpReadB;
		for(size_t w = 0; w < (size_t)width; ++w)
		{
			half.u = cpLineR[w];
			single = halfToSingle(half).f * 255.0f;
			clamp(single, 0.0f, 255.0f);
			image[i].parts.r = single;

			half.u = cpLineG[w];
			single = halfToSingle(half).f * 255.0f;
			clamp(single, 0.0f, 255.0f);
			image[i].parts.g = single;

			half.u = cpLineB[w];
			single = halfToSingle(half).f * 255.0f;
			clamp(single, 0.0f, 255.0f);
			image[i].parts.b = single;

			image[i].parts.x = 255;
			i++;
		}
		cpReadR += strideR;
		cpReadG += strideG;
		cpReadB += strideB;
	}

	QImage frameImage((const uchar *)image.data(), width, height,
		QImage::Format_RGB32);
	QPixmap framePixmap = QPixmap::fromImage(frameImage).copy();

	return framePixmap;
}

// END OF QPixmap vsedit::pixmapFromRGBH(const VSAPI * a_cpVSAPI,
//		const VSFormat * a_cpFormat, const VSFrameRef * a_cpFrameRef)
//==============================================================================

QPixmap vsedit::pixmapFromRGBS(const VSAPI * a_cpVSAPI,
	const VSFormat * a_cpFormat, const VSFrameRef * a_cpFrameRef)
{
	(void)a_cpFormat;

	int width = a_cpVSAPI->getFrameWidth(a_cpFrameRef, 0);
	int height = a_cpVSAPI->getFrameHeight(a_cpFrameRef, 0);
	const uint8_t * cpReadR = a_cpVSAPI->getReadPtr(a_cpFrameRef, 0);
	const uint8_t * cpReadG = a_cpVSAPI->getReadPtr(a_cpFrameRef, 1);
	const uint8_t * cpReadB = a_cpVSAPI->getReadPtr(a_cpFrameRef, 2);
	int strideR = a_cpVSAPI->getStride(a_cpFrameRef, 0);
	int strideG = a_cpVSAPI->getStride(a_cpFrameRef, 1);
	int strideB = a_cpVSAPI->getStride(a_cpFrameRef, 2);

	std::vector<vsedit::RGB32> image(width * height);
	size_t i = 0;
	const float *cpLineR, *cpLineG, *cpLineB;
	float single;
	for(size_t h = 0; h < (size_t)height; ++h)
	{
		cpLineR = (const float *)cpReadR;
		cpLineG = (const float *)cpReadG;
		cpLineB = (const float *)cpReadB;
		for(size_t w = 0; w < (size_t)width; ++w)
		{
			single = cpLineR[w] * 255.0f;
			clamp(single, 0.0f, 255.0f);
			image[i].parts.r = single;

			single = cpLineG[w] * 255.0f;
			clamp(single, 0.0f, 255.0f);
			image[i].parts.g = single;

			single = cpLineB[w] * 255.0f;
			clamp(single, 0.0f, 255.0f);
			image[i].parts.b = single;

			image[i].parts.x = 255;
			i++;
		}
		cpReadR += strideR;
		cpReadG += strideG;
		cpReadB += strideB;
	}

	QImage frameImage((const uchar *)image.data(), width, height,
		QImage::Format_RGB32);
	QPixmap framePixmap = QPixmap::fromImage(frameImage).copy();

	return framePixmap;
}

// END OF QPixmap vsedit::pixmapFromRGBS(const VSAPI * a_cpVSAPI,
//		const VSFormat * a_cpFormat, const VSFrameRef * a_cpFrameRef)
//==============================================================================

QPixmap vsedit::pixmapFromCompatBGR32(const VSAPI * a_cpVSAPI,
	const VSFormat * a_cpFormat, const VSFrameRef * a_cpFrameRef)
{
	(void)a_cpFormat;

	int frameWidth = a_cpVSAPI->getFrameWidth(a_cpFrameRef, 0);
	int frameHeight = a_cpVSAPI->getFrameHeight(a_cpFrameRef, 0);
	int stride = a_cpVSAPI->getStride(a_cpFrameRef, 0);
	const uint8_t * pFrameReadPointer =
		a_cpVSAPI->getReadPtr(a_cpFrameRef, 0);

	// Creating QImage from memory doesn't copy or modify the data.
	QImage frameImage(pFrameReadPointer, frameWidth, frameHeight, stride,
		QImage::Format_RGB32);
	// But when creating the pixmap - we use mirrored copy.
	QPixmap framePixmap = QPixmap::fromImage(frameImage.mirrored());

	return framePixmap;
}

// END OF QPixmap vsedit::pixmapFromCompatBGR32(const VSAPI * a_cpVSAPI,
//		const VSFormat * a_cpFormat, const VSFrameRef * a_cpFrameRef)
//==============================================================================

QPixmap vsedit::pixmapFromCompatYUY2(const VSAPI * a_cpVSAPI,
	const VSFormat * a_cpFormat, const VSFrameRef * a_cpFrameRef)
{
	(void)a_cpFormat;

	int width = a_cpVSAPI->getFrameWidth(a_cpFrameRef, 0);
	int height = a_cpVSAPI->getFrameHeight(a_cpFrameRef, 0);
	const uint8_t * cpRead = a_cpVSAPI->getReadPtr(a_cpFrameRef, 0);
	int stride = a_cpVSAPI->getStride(a_cpFrameRef, 0);

	std::vector<vsedit::RGB32> image(width * height);
	size_t i = 0;
	const vsedit::YUY2 * cpLine;
	for(size_t h = 0; h < (size_t)height; ++h)
	{
		cpLine = (const vsedit::YUY2 *)cpRead;
		for(size_t w = 0; w < (size_t)width; ++w)
		{
			if(w & 1)
			{
				image[i] = yuvToRgb32(cpLine[w >> 1].parts.y1,
					cpLine[w >> 1].parts.u, cpLine[w >> 1].parts.v);
			}
			else
			{
				image[i] = yuvToRgb32(cpLine[w >> 1].parts.y0,
					cpLine[w >> 1].parts.u, cpLine[w >> 1].parts.v);
			}
			i++;
		}
		cpRead += stride;
	}

	QImage frameImage((const uchar *)image.data(), width, height,
		QImage::Format_RGB32);
	QPixmap framePixmap = QPixmap::fromImage(frameImage).copy();

	return framePixmap;
}

// END OF QPixmap vsedit::pixmapFromCompatYUY2(const VSAPI * a_cpVSAPI,
//		const VSFormat * a_cpFormat, const VSFrameRef * a_cpFrameRef)
//==============================================================================
