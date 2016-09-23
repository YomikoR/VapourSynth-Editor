#ifndef ENCODE_DIALOG_H_INCLUDED
#define ENCODE_DIALOG_H_INCLUDED

#include <QProcess>
#include <vector>
#include <deque>
#include <functional>

#include <ui_encode_dialog.h>

#include "../vapoursynth/vs_script_processor_dialog.h"
#include "../common/chrono.h"
#include "../settings/settingsmanager.h"

struct VSFrameRef;

struct NumberedFrameRef
{
	int number;
	const VSFrameRef * cpFrameRef;

	NumberedFrameRef(int a_number, const VSFrameRef * a_cpFrameRef);
	bool operator<(const NumberedFrameRef & a_other) const;
};

struct VariableToken
{
	QString token;
	QString description;
	std::function<QString()> evaluate;
};

class EncodeDialog : public VSScriptProcessorDialog
{
	Q_OBJECT

	public:

		EncodeDialog(SettingsManager * a_pSettingsManager,
			QWidget * a_pParent = nullptr);
		virtual ~EncodeDialog();

		enum class State
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

	public slots:

		void call();

	protected slots:

		virtual void slotWriteLogMessage(int a_messageType,
			const QString & a_message) override;

		void slotWholeVideoButtonPressed();

		void slotStartStopEncodeButtonPressed();

		void slotExecutableBrowseButtonPressed();

		void slotArgumentsHelpButtonPressed();

		void slotEncodingPresetSaveButtonPressed();
		void slotEncodingPresetDeleteButtonPressed();
		void slotEncodingPresetComboBoxActivated(const QString & a_text);

		void slotReceiveFrame(int a_frameNumber,
			const VSFrameRef * a_cpFrameRef);

		void slotEncoderStarted();
		void slotEncoderFinished(int a_exitCode,
			QProcess::ExitStatus a_exitStatus);
		void slotEncoderError(QProcess::ProcessError a_error);
		void slotEncoderReadChannelFinished();
		void slotEncoderBytesWritten(qint64 a_bytes);
		void slotEncoderReadyReadStandardError();

	protected:

		virtual void stopAndCleanUp() override;

		void stopProcessing();

		void processFramesQueue();

		QString decodeArguments(const QString & a_arguments);

		void clearFramesQueue();

		void outputStandardError();

		void fillVariables();

		void setUpEncodingPresets();

		Ui::EncodeDialog m_ui;

		int m_firstFrame;
		int m_lastFrame;
		int m_framesTotal;
		int m_framesProcessed;

		hr_time_point m_encodeStartTime;

		QProcess m_encoder;

		std::vector<char> m_framebuffer;

		std::deque<NumberedFrameRef> m_framesQueue;
		size_t m_cachedFramesLimit;

		int m_lastFrameProcessed;
		int m_lastFrameRequested;

		std::vector<VariableToken> m_variables;

		State m_state;

		size_t m_bytesToWrite;

		std::vector<EncodingPreset> m_encodingPresets;
};

#endif // ENCODE_DIALOG_H_INCLUDED
