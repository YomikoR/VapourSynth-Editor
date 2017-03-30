#ifndef JOB_H_INCLUDED
#define JOB_H_INCLUDED

#include "../settings/settings_definitions.h"
#include "../common/chrono.h"
#include "../common/helpers.h"
#include "../log/vs_editor_log.h"
#include "../vapoursynth/vs_script_processor_structures.h"

#include <QObject>
#include <QUuid>
#include <QDateTime>
#include <QProcess>
#include <vector>

class SettingsManager;
class VSScriptLibrary;
class VapourSynthScriptProcessor;
class FrameHeaderWriter;

namespace vsedit
{

class Job : public QObject
{
	Q_OBJECT

public:

	Job(const JobProperties & a_properties = JobProperties(),
		SettingsManager * a_pSettingsManager = nullptr,
		VSScriptLibrary * a_pVSScriptLibrary = nullptr,
		QObject * a_pParent = nullptr);
	virtual ~Job();

	enum class EncodingState
	{
		Idle,
		CheckingEncoderSanity,
		StartingEncoder,
		WritingHeader,
		WaitingForFrames,
		WritingFrame,
		EncoderCrashed,
		Finishing,
		Aborting,
	};

	static QString typeName(JobType a_type);

	static QString stateName(JobState a_state);

	virtual void start();
	virtual void pause();
	virtual void abort();

	virtual QUuid id() const;
	virtual bool setId(const QUuid & a_id);

	virtual JobType type() const;
	virtual bool setType(JobType a_type);

	virtual QString scriptName() const;
	virtual bool setScriptName(const QString & a_scriptName);

	virtual EncodingHeaderType encodingHeaderType() const;
	virtual bool setEncodingHeaderType(EncodingHeaderType a_headerType);

	virtual QString executablePath() const;
	virtual bool setExecutablePath(const QString & a_path);

	virtual QString arguments() const;
	virtual bool setArguments(const QString & a_arguments);

	virtual QString shellCommand() const;
	virtual bool setShellCommand(const QString & a_command);

	virtual JobState state() const;
	virtual bool setState(JobState a_state);

	virtual std::vector<QUuid> dependsOnJobIds() const;
	virtual bool setDependsOnJobIds(const std::vector<QUuid> & a_ids);

	virtual QString subject() const;

	virtual std::vector<vsedit::VariableToken> variables() const;

	virtual int framesProcessed() const;
	virtual int framesTotal() const;

	virtual JobProperties properties() const;
	virtual bool setProperties(const JobProperties & a_properties);

signals:

	void signalStateChanged(JobState a_newState, JobState a_oldState);

	void signalProgressChanged();

	void signalLogMessage(const QString & a_message,
		const QString & a_style = LOG_STYLE_DEFAULT);

protected slots:

	virtual void slotProcessStarted();
	virtual void slotProcessFinished(int a_exitCode,
		QProcess::ExitStatus a_exitStatus);
	virtual void slotProcessError(QProcess::ProcessError a_error);
	virtual void slotProcessReadChannelFinished();
	virtual void slotProcessBytesWritten(qint64 a_bytes);
	virtual void slotProcessReadyReadStandardError();

	virtual void slotWriteLogMessage(int a_messageType,
		const QString & a_message);
	virtual void slotFrameQueueStateChanged(size_t a_inQueue,
		size_t a_inProcess, size_t a_maxThreads);
	virtual void slotCoreFramebufferUsedBytes(int64_t a_bytes);
	virtual void slotReceiveFrame(int a_frameNumber, int a_outputIndex,
		const VSFrameRef * a_cpOutputFrameRef,
		const VSFrameRef * a_cpPreviewFrameRef);
	virtual void slotFrameRequestDiscarded(int a_frameNumber,
		int a_outputIndex, const QString & a_reason);

protected:

	virtual void fillVariables();

	virtual void changeStateAndNotify(JobState a_state);

	virtual void startEncodeScriptCLI();
	virtual void startRunProcess();
	virtual void startRunShellCommand();

	virtual QString decodeArguments(const QString & a_arguments) const;

	virtual void cleanUpEncoding();

	virtual void clearFramesCache();

	virtual void processFramesQueue();

	JobProperties m_properties;

	QProcess m_process;

	std::vector<char> m_framebuffer;

	int m_lastFrameProcessed;
	int m_lastFrameRequested;

	std::vector<vsedit::VariableToken> m_variables;

	EncodingState m_encodingState;

	size_t m_bytesToWrite;
	size_t m_bytesWritten;

	SettingsManager * m_pSettingsManager;

	VSScriptLibrary * m_pVSScriptLibrary;

	VapourSynthScriptProcessor * m_pVapourSynthScriptProcessor;

	const VSAPI * m_cpVSAPI;

	const VSVideoInfo * m_cpVideoInfo;

	FrameHeaderWriter * m_pFrameHeaderWriter;

	std::list<Frame> m_framesCache;
	size_t m_cachedFramesLimit;
};

}

#endif // JOB_H_INCLUDED
