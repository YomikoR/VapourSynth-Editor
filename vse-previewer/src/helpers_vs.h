#ifndef HELPERS_VS_H_INCLUDED
#define HELPERS_VS_H_INCLUDED

#include <vapoursynth/VapourSynth4.h>

template <typename TV, typename TA>
class VSMediaTypePicker
{
protected:
	const TV * m_pTV;
	const TA * m_pTA;
	int m_mediaType;

public:
	VSMediaTypePicker()
		: m_pTV(nullptr)
		, m_pTA(nullptr)
		, m_mediaType(-1)
	{}

	VSMediaTypePicker(const TV * a_pTV) : VSMediaTypePicker()
	{
		if(a_pTV)
		{
			m_pTV = a_pTV;
			m_mediaType = mtVideo;
		}
	}

	VSMediaTypePicker(const TA * a_pTA) : VSMediaTypePicker()
	{
		if(a_pTA)
		{
			m_pTA = a_pTA;
			m_mediaType = mtAudio;
		}
	}

	const void * get() const
	{
		switch (m_mediaType)
		{
		case mtAudio:
			return reinterpret_cast<const void *>(m_pTA);
		case mtVideo:
			return reinterpret_cast<const void *>(m_pTV);
		default:
			return nullptr;
		}
	}

	const TV * getAsVideo() const
	{
		return m_pTV;
	}

	const TA * getAsAudio() const
	{
		return m_pTA;
	}

	void set(const TV * a_pTV)
	{
		m_pTV = a_pTV;
		m_pTA = nullptr;
		m_mediaType = a_pTV ? mtVideo : -1;
	}

	void set(const TA * a_pTA)
	{
		m_pTV = nullptr;
		m_pTA = a_pTA;
		m_mediaType = a_pTA ? mtAudio : -1;
	}

	void setNull()
	{
		m_pTV = nullptr;
		m_pTA = nullptr;
		m_mediaType = -1;
	}

	bool isAudio() const
	{
		return m_mediaType == mtAudio;
	}

	bool isVideo() const
	{
		return m_mediaType == mtVideo;
	}

	bool isInvalid() const
	{
		return (!isVideo()) && (!isAudio());
	}

	int mediaType() const
	{
		return m_mediaType;
	}
};

class VSNodeInfo : public VSMediaTypePicker<VSVideoInfo, VSAudioInfo>
{
public:
	VSNodeInfo() : VSMediaTypePicker<VSVideoInfo, VSAudioInfo>() {}

	VSNodeInfo(VSNode * a_pNode, const VSAPI * a_cpVSAPI)
	{
		int mediaType = a_cpVSAPI->getNodeType(a_pNode);
		if(mediaType == mtAudio)
			set(a_cpVSAPI->getAudioInfo(a_pNode));
		else
			set(a_cpVSAPI->getVideoInfo(a_pNode));
	}

	int numFrames() const
	{
		switch (m_mediaType)
		{
		case mtAudio:
			return m_pTA->numFrames;
		case mtVideo:
			return m_pTV->numFrames;
		default:
			return -1;
		}
	}

};

class VSFrameFormat : public VSMediaTypePicker<VSVideoFormat, VSAudioFormat>
{
public:
	VSFrameFormat() : VSMediaTypePicker<VSVideoFormat, VSAudioFormat>() {}

	VSFrameFormat(const VSFrame * a_cpFrame, const VSAPI * a_cpVSAPI)
	{
		int mediaType = a_cpVSAPI->getFrameType(a_cpFrame);
		if(mediaType == mtAudio)
			set(a_cpVSAPI->getAudioFrameFormat(a_cpFrame));
		else
			set(a_cpVSAPI->getVideoFrameFormat(a_cpFrame));
	}
};

inline bool isVariableSize(const VSVideoInfo *vi)
{
	return vi->width == 0 && vi->height == 0;
}

inline bool isVariableFPS(const VSVideoInfo *vi)
{
	return vi->fpsDen == 0 && vi->fpsNum == 0;
}

inline bool isVariableFormat(const VSVideoInfo *vi)
{
	return vi->format.colorFamily == cfUndefined ||
		vi->format.bitsPerSample == 0 ||
		vi->format.bytesPerSample == 0 ||
		vi->format.numPlanes == 0;
}

#endif
