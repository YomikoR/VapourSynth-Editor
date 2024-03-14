#ifndef VS_SCRIPT_PROCESSOR_STRUCTURES_H_INCLUDED
#define VS_SCRIPT_PROCESSOR_STRUCTURES_H_INCLUDED

#include <vapoursynth/VapourSynth4.h>

//==============================================================================

struct Frame
{
	int number;
	int outputIndex;
	const VSFrame * cpOutputFrame;
	const VSFrame * cpPreviewFrame;

	Frame(int a_number, int a_outputIndex,
		const VSFrame * a_cpOutputFrame,
		const VSFrame * a_cpPreviewFrame = nullptr);
	bool operator==(const Frame & a_other) const;
};

//==============================================================================

struct FrameTicket
{
	int frameNumber;
	int outputIndex;
	VSNode * pOutputNode;
	bool needPreview;
	VSNode * pPreviewNode;
	const VSFrame * cpOutputFrame;
	const VSFrame * cpPreviewFrame;
	bool discard;

	FrameTicket(int a_frameNumber, int a_outputIndex,
		VSNode * a_pOutputNode, bool a_needPreview = false,
		VSNode * a_pPreviewNode = nullptr);

	bool isComplete() const;
};

//==============================================================================

struct NodePair
{
	int outputIndex;
	VSNode * pOutputNode;
	VSNode * pPreviewNode;

	NodePair();
	NodePair(int a_outputIndex, VSNode * a_pOutputNode,
		VSNode * a_pPreviewNode);

	bool isNull() const;
	bool isValid() const;
};

//==============================================================================

#endif // VS_SCRIPT_PROCESSOR_STRUCTURES_H_INCLUDED
