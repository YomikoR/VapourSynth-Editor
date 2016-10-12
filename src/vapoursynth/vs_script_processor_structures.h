#ifndef VS_SCRIPT_PROCESSOR_STRUCTURES_H_INCLUDED
#define VS_SCRIPT_PROCESSOR_STRUCTURES_H_INCLUDED

#include <vapoursynth/VSScript.h>

//==============================================================================

struct Frame
{
	int number;
	int outputIndex;
	const VSFrameRef * cpOutputFrameRef;
	const VSFrameRef * cpPreviewFrameRef;

	Frame(int a_number, int a_outputIndex,
		const VSFrameRef * a_cpOutputFrameRef,
		const VSFrameRef * a_cpPreviewFrameRef = nullptr);
	bool operator==(const Frame & a_other) const;
};

//==============================================================================

struct FrameTicket
{
	int frameNumber;
	int outputIndex;
	VSNodeRef * pOutputNode;
	bool needPreview;
	VSNodeRef * pPreviewNode;
	const VSFrameRef * cpOutputFrameRef;
	const VSFrameRef * cpPreviewFrameRef;
	bool discard;

	FrameTicket(int a_frameNumber, int a_outputIndex,
		VSNodeRef * a_pOutputNode, bool a_needPreview = false,
		VSNodeRef * a_pPreviewNode = nullptr);

	bool isComplete() const;
};

//==============================================================================

struct NodePair
{
	int outputIndex;
	VSNodeRef * pOutputNode;
	VSNodeRef * pPreviewNode;

	NodePair();
	NodePair(int a_outputIndex, VSNodeRef * a_pOutputNode,
		VSNodeRef * a_pPreviewNode);

	bool isNull() const;
	bool isValid() const;
};

//==============================================================================

#endif // VS_SCRIPT_PROCESSOR_STRUCTURES_H_INCLUDED
