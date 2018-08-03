#include "vs_script_processor_structures.h"

//==============================================================================

Frame::Frame(int a_number, int a_outputIndex,
	const VSFrameRef * a_cpOutputFrameRef,
	const VSFrameRef * a_cpPreviewFrameRef):
	  number(a_number)
	, outputIndex(a_outputIndex)
	, cpOutputFrameRef(a_cpOutputFrameRef)
	, cpPreviewFrameRef(a_cpPreviewFrameRef)
{
}

bool Frame::operator==(const Frame & a_other) const
{
	return ((number == a_other.number) && (outputIndex == a_other.outputIndex));
}

//==============================================================================

FrameTicket::FrameTicket(int a_frameNumber, int a_outputIndex,
		VSNodeRef * a_pOutputNode, bool a_needPreview,
		VSNodeRef * a_pPreviewNode):
	frameNumber(a_frameNumber)
	, outputIndex(a_outputIndex)
	, pOutputNode(a_pOutputNode)
	, needPreview(a_needPreview)
	, pPreviewNode(a_pPreviewNode)
	, cpOutputFrameRef(nullptr)
	, cpPreviewFrameRef(nullptr)
	, discard(false)
{
}

//==============================================================================

bool FrameTicket::isComplete() const
{
	bool complete = (cpOutputFrameRef != nullptr);
	if(needPreview)
		complete = complete && (cpPreviewFrameRef != nullptr);
	return complete;
}

//==============================================================================

NodePair::NodePair():
	  outputIndex(-1)
	, pOutputNode(nullptr)
	, pPreviewNode(nullptr)
{
}

//==============================================================================

NodePair::NodePair(int a_outputIndex, VSNodeRef * a_pOutputNode,
	VSNodeRef * a_pPreviewNode):
	  outputIndex(a_outputIndex)
	, pOutputNode(a_pOutputNode)
	, pPreviewNode(a_pPreviewNode)
{
}

//==============================================================================

bool NodePair::isNull() const
{
	return ((outputIndex == -1) && (pOutputNode == nullptr) &&
		(pPreviewNode == nullptr));
}

//==============================================================================

bool NodePair::isValid() const
{
	return ((outputIndex >= 0) && (pOutputNode != nullptr) &&
		(pPreviewNode != nullptr));
}

//==============================================================================
