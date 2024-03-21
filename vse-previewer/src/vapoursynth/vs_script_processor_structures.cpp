#include "vs_script_processor_structures.h"

//==============================================================================

Frame::Frame(int a_number, int a_outputIndex,
	const VSFrame * a_cpOutputFrame,
	const VSFrame * a_cpPreviewFrame):
	  number(a_number)
	, outputIndex(a_outputIndex)
	, cpOutputFrame(a_cpOutputFrame)
	, cpPreviewFrame(a_cpPreviewFrame)
{
}

bool Frame::operator==(const Frame & a_other) const
{
	return ((number == a_other.number) && (outputIndex == a_other.outputIndex));
}

//==============================================================================

FrameTicket::FrameTicket(int a_frameNumber, int a_outputIndex,
		VSNode * a_pOutputNode,
		VSNode * a_pPreviewNode):
	frameNumber(a_frameNumber)
	, outputIndex(a_outputIndex)
	, pOutputNode(a_pOutputNode)
	, pPreviewNode(a_pPreviewNode)
	, cpOutputFrame(nullptr)
	, cpPreviewFrame(nullptr)
	, discard(false)
{
}

//==============================================================================

bool FrameTicket::isComplete() const
{
	bool complete = (cpOutputFrame != nullptr);
	complete = complete && (cpPreviewFrame != nullptr);
	return complete;
}

//==============================================================================

NodePair::NodePair(const VSAPI * a_cpVSAPI):
	  outputIndex(-1)
	, pOutputNode(nullptr)
	, pPreviewNode(nullptr)
	, cpVSAPI(a_cpVSAPI)
{
}

//==============================================================================

NodePair::NodePair(int a_outputIndex, VSNode * a_pOutputNode,
	VSNode * a_pPreviewNode, const VSAPI * a_cpVSAPI):
	  outputIndex(a_outputIndex)
	, pOutputNode(a_pOutputNode)
	, pPreviewNode(a_pPreviewNode)
	, cpVSAPI(a_cpVSAPI)
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

NodePair::~NodePair()
{
	if(cpVSAPI)
	{
		cpVSAPI->freeNode(pOutputNode);
		cpVSAPI->freeNode(pPreviewNode);
	}
}

//==============================================================================
