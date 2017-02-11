#ifndef JOB_H_INCLUDED
#define JOB_H_INCLUDED

#include "../settings/settings_definitions.h"
#include "../frame_consumers/frame_header_writers/frame_header_writer.h"
#include "../common/chrono.h"
#include "../common/helpers.h"

#include <QObject>
#include <QUuid>
#include <QDateTime>
#include <QProcess>
#include <vector>

namespace vsedit
{

class Job : public QObject
{
	Q_OBJECT

public:

	Job(QObject * a_pParent = nullptr);
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

	virtual void start();
	virtual void pause();
	virtual void abort();

	virtual QUuid id();
	virtual bool setId(const QUuid & a_id);

	virtual JobType type();
	virtual bool setType(JobType a_type);

	virtual JobState state();
	virtual bool setState(JobState a_state);

	virtual std::vector<QUuid> dependsOnJobIds();
	virtual bool setDependsOnJobIds(const std::vector<QUuid> & a_ids);

signals:

	void signalStateChanged(JobState a_state);

protected:

	QUuid m_id;

	JobType m_type;
	JobState m_jobState;

	std::vector<QUuid> m_dependsOnJobIds;

	QDateTime m_timeStarted;
	QDateTime m_timeEnded;

	QString m_scriptName;

	EncodingType m_encodingType;
	EncodingHeaderType m_encodingHeaderType;
	QString m_executablePath;
	QString m_arguments;
	QString m_shellCommand;

	int m_firstFrame;
	int m_lastFrame;
	int m_framesTotal;
	int m_framesProcessed;

	hr_time_point m_encodeStartTime;

	QProcess m_encoder;

	std::vector<char> m_framebuffer;

	int m_lastFrameProcessed;
	int m_lastFrameRequested;

	std::vector<vsedit::VariableToken> m_variables;

	EncodingState m_encodingState;

	size_t m_bytesToWrite;
	size_t m_bytesWritten;

	FrameHeaderWriter * m_pFrameHeaderWriter;
};

}

#endif // JOB_H_INCLUDED
