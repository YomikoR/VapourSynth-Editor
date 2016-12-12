#ifndef VAPOURSYNTHSCRIPTPROCESSOR_H
#define VAPOURSYNTHSCRIPTPROCESSOR_H

#include "vs_script_processor_structures.h"
#include "../settings/settings_manager.h"

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

	VapourSynthScriptProcessor(SettingsManager * a_pSettingsManager,
		VSScriptLibrary * a_pVSScriptLibrary, QObject * a_pParent = nullptr);

	virtual ~VapourSynthScriptProcessor();

	bool initialize(const QString& a_script, const QString& a_scriptName);

	bool finalize();

	bool isInitialized() const;

	QString error() const;

	const VSVideoInfo * videoInfo(int a_outputIndex = 0);

	bool requestFrameAsync(int a_frameNumber, int a_outputIndex = 0,
		bool a_needPreview = false);

	bool flushFrameTicketsQueue();

	const QString & script() const;

	const QString & scriptName() const;

	void setScriptName(const QString & a_scriptName);

public slots:

	void slotResetSettings();

signals:

	void signalWriteLogMessage(int a_messageType, const QString & a_message);

	void signalDistributeFrame(int a_frameNumber, int a_outputIndex,
		const VSFrameRef * a_cpOutputFrameRef,
		const VSFrameRef * a_cpPreviewFrameRef);

	void signalFrameRequestDiscarded(int a_frameNumber, int a_outputIndex,
		const QString & a_reason);

	void signalFrameQueueStateChanged(size_t a_inQueue, size_t a_inProcess,
		size_t a_maxThreads);

private slots:

	void slotReceiveFrameAndProcessQueue(
		const VSFrameRef * a_cpFrameRef, int a_frameNumber,
		VSNodeRef * a_pNodeRef, QString a_errorMessage);

private:

	void receiveFrame(const VSFrameRef * a_cpFrameRef, int a_frameNumber,
		VSNodeRef * a_pNodeRef, const QString & a_errorMessage);

	void processFrameTicketsQueue();

	void sendFrameQueueChangeSignal();

	bool recreatePreviewNode(NodePair & a_nodePair);

	void freeFrameTicket(FrameTicket & a_ticket);

	NodePair & getNodePair(int a_outputIndex, bool a_needPreview);

	QString framePropsString(const VSFrameRef * a_cpFrame) const;

	void printFrameProps(const VSFrameRef * a_cpFrame);

	SettingsManager * m_pSettingsManager;

	VSScriptLibrary * m_pVSScriptLibrary;

	QString m_script;

	QString m_scriptName;

	QString m_error;

	bool m_initialized;

	const VSAPI * m_cpVSAPI;

	VSScript * m_pVSScript;

	const VSVideoInfo * m_cpVideoInfo;
	const VSCoreInfo * m_cpCoreInfo;

	std::deque<FrameTicket> m_frameTicketsQueue;
	std::vector<FrameTicket> m_frameTicketsInProcess;
	std::map<int, NodePair> m_nodePairForOutputIndex;

	ResamplingFilter m_chromaResamplingFilter;
	ChromaPlacement m_chromaPlacement;
	double m_resamplingFilterParameterA;
	double m_resamplingFilterParameterB;
	YuvMatrixCoefficients m_yuvMatrix;
};

//==============================================================================

#endif // VAPOURSYNTHSCRIPTPROCESSOR_H
