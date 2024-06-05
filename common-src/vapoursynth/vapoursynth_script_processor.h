#ifndef VAPOURSYNTHSCRIPTPROCESSOR_H
#define VAPOURSYNTHSCRIPTPROCESSOR_H

#include "vs_script_processor_structures.h"
#include "../settings/settings_manager_core.h"
#include "../helpers_vs.h"

#include <vapoursynth/VSScript4.h>

#include <QObject>
#include <deque>
#include <vector>
#include <map>

class VSScriptLibrary;

//==============================================================================

class VapourSynthScriptProcessor : public QObject
{
	Q_OBJECT

public:

	VapourSynthScriptProcessor(SettingsManagerCore * a_pSettingsManager,
		VSScriptLibrary * a_pVSScriptLibrary, QObject * a_pParent = nullptr);

	virtual ~VapourSynthScriptProcessor();

	bool initialize(const QString& a_script, const QString& a_scriptName,
		int a_outputIndex, ProcessReason a_reason);

	bool finalize();

	bool isInitialized() const;

	QString error() const;

	std::vector<int> getOutputIndices() const;

	VSNodeInfo nodeInfo(int a_outputIndex = 0);

	bool requestFrameAsync(int a_frameNumber, int a_outputIndex = 0,
		bool a_needPreview = false);

	bool flushFrameTicketsQueue();

	const QString & script() const;

	const QString & scriptName() const;

	void setScriptName(const QString & a_scriptName);

	QString framePropsString(const VSFrame * a_cpFrame) const;

	bool clearCoreCaches();

public slots:

	void slotResetSettings();

signals:

	void signalWriteLogMessage(int a_messageType, const QString & a_message);

	void signalDistributeFrame(int a_frameNumber, int a_outputIndex,
		const VSFrame * a_cpOutputFrame,
		const VSFrame * a_cpPreviewFrame);

	void signalFrameRequestDiscarded(int a_frameNumber, int a_outputIndex,
		const QString & a_reason);

	void signalFrameQueueStateChanged(size_t a_inQueue, size_t a_inProcess,
		size_t a_maxThreads, double a_usedCacheRatio);

	void signalFinalized();

private slots:

	void slotReceiveFrameAndProcessQueue(
		const VSFrame * a_cpFrame, int a_frameNumber,
		VSNode * a_pNode, QString a_errorMessage);

private:

	void receiveFrame(const VSFrame * a_cpFrame, int a_frameNumber,
		VSNode * a_pNode, const QString & a_errorMessage);

	void processFrameTicketsQueue();

	void sendFrameQueueChangeSignal();

	bool recreatePreviewNode(NodePair & a_nodePair);

	bool recreateAudioPreviewNode(NodePair & a_nodePair);

	void freeFrameTicket(FrameTicket & a_ticket);

	NodePair & getNodePair(int a_outputIndex, bool a_needPreview);

	SettingsManagerCore * m_pSettingsManager;

	VSScriptLibrary * m_pVSScriptLibrary;

	QString m_script;

	QString m_scriptName;

	QString m_error;

	bool m_initialized;

	const VSAPI * m_cpVSAPI;

	VSScript * m_pVSScript;
	VSCore * m_pCore;

	VSNodeInfo m_nodeInfo;
	VSCoreInfo m_cpCoreInfo;

	std::deque<FrameTicket> m_frameTicketsQueue;
	std::vector<FrameTicket> m_frameTicketsInProcess;
	std::map<int, NodePair> m_nodePairForOutputIndex;

	ResamplingFilter m_chromaResamplingFilter;
	ChromaPlacement m_chromaPlacement;
	double m_resamplingFilterParameterA;
	double m_resamplingFilterParameterB;
	YuvMatrixCoefficients m_yuvMatrix;
	DitherType m_ditherType;

	bool m_finalizing;
};

//==============================================================================

#endif // VAPOURSYNTHSCRIPTPROCESSOR_H
