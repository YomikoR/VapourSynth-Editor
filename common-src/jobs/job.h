#ifndef JOB_H_INCLUDED
#define JOB_H_INCLUDED

#include "../../../common-src/settings/settings_definitions_core.h"
#include "../../../common-src/chrono.h"
#include "../../../common-src/helpers.h"
#include "../../../common-src/log/styled_log_view_core.h"
#include "../../../common-src/log/vs_editor_log_definitions.h"
#include "../../../common-src/vapoursynth/vs_script_processor_structures.h"
#include "../../../common-src/jobs/job_variables.h"

#include <QObject>
#include <QUuid>
#include <QDateTime>
#include <QProcess>
#include <vector>

class SettingsManagerCore;
class VSScriptLibrary;
class VapourSynthScriptProcessor;
class FrameHeaderWriter;

namespace vsedit
{

class Job : public QObject, public JobVariables
{
	Q_OBJECT

public:

	Job(const JobProperties & a_properties = JobProperties(),
		SettingsManagerCore * a_pSettingsManager = nullptr,
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

	virtual bool isActive() const;

	virtual QUuid id() const;
	virtual bool setId(const QUuid & a_id);

	virtual JobType type() const;
	virtual bool setType(JobType a_type);

	virtual QString scriptName() const;
	virtual bool setScriptName(const QString & a_scriptName);

	virtual QString scriptText() const;
	virtual bool setScriptText(const QString & a_scriptText);

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

	virtual int firstFrame() const;
	virtual bool setFirstFrame(int a_frame);

	virtual int lastFrame() const;
	virtual bool setLastFrame(int a_frame);

	virtual int framesProcessed() const;
	virtual int framesTotal() const;
	virtual double fps() const;
	virtual double secondsToFinish() const;

	virtual size_t framesInQueue() const;
	virtual size_t framesInProcess() const;
	virtual size_t maxThreads() const;

	virtual JobProperties properties() const;
	virtual bool setProperties(const JobProperties & a_properties);

	virtual const VSVideoInfo * videoInfo() const;

	virtual bool initialize();

	virtual void cleanUpEncoding();

public slots:

	virtual void start();
	virtual void pause();
	virtual void abort();

signals:

	void signalPropertiesChanged();
	void signalStateChanged(JobState a_newState, JobState a_oldState);
	void signalProgressChanged();
	void signalStartTimeChanged();
	void signalEndTimeChanged();

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
		size_t a_inProcess, size_t a_maxThreads, double a_usedCacheRatio);
	virtual void slotScriptProcessorFinalized();
	virtual void slotReceiveFrame(int a_frameNumber, int a_outputIndex,
		const VSFrame * a_cpOutputFrame,
		const VSFrame * a_cpPreviewFrame);
	virtual void slotFrameRequestDiscarded(int a_frameNumber,
		int a_outputIndex, const QString & a_reason);

protected:

	virtual void fillVariables() override;

	virtual void changeStateAndNotify(JobState a_state);

	virtual void startEncodeScriptCLI();
	virtual void startRunProcess();
	virtual void startRunShellCommand();

	virtual QString decodeArguments(const QString & a_arguments) const;

	virtual void clearFramesCache();

	virtual void processFramesQueue();

	virtual void finishEncodingCLI();

	virtual void memorizeEncodingTime();

	virtual void updateFPS();

	virtual double currentEncodingRangeTime() const;

	JobProperties m_properties;

	QProcess m_process;

	std::vector<char> m_framebuffer;

	int m_lastFrameProcessed;
	int m_lastFrameRequested;

	EncodingState m_encodingState;

	size_t m_bytesToWrite;
	size_t m_bytesWritten;

	SettingsManagerCore * m_pSettingsManager;

	VSScriptLibrary * m_pVSScriptLibrary;

	VapourSynthScriptProcessor * m_pVapourSynthScriptProcessor;

	const VSAPI * m_cpVSAPI;

	const VSVideoInfo * m_cpVideoInfo;

	FrameHeaderWriter * m_pFrameHeaderWriter;

	std::list<Frame> m_framesCache;
	size_t m_cachedFramesLimit;

	size_t m_framesInQueue;
	size_t m_framesInProcess;
	size_t m_maxThreads;

	hr_time_point m_encodeRangeStartTime;
	double m_memorizedEncodingTime;
};

}

#endif // JOB_H_INCLUDED
