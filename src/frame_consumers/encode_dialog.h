#ifndef ENCODE_DIALOG_H_INCLUDED
#define ENCODE_DIALOG_H_INCLUDED

#include <QProcess>
#include <vector>
#include <deque>
#include <functional>

#include <ui_encode_dialog.h>

#include "../vapoursynth/vs_script_processor_dialog.h"
#include "../common/chrono.h"

struct VSFrameRef;
class SettingsManager;

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

	public slots:

		void call();

	protected slots:

		virtual void slotWriteLogMessage(int a_messageType,
			const QString & a_message) override;

		void slotWholeVideoButtonPressed();

		void slotStartStopBenchmarkButtonPressed();

		void slotExecutableBrowseButtonPressed();

		void slotArgumentsHelpButtonPressed();

		void slotReceiveFrame(int a_frameNumber,
			const VSFrameRef * a_cpFrameRef);

	protected:

		virtual void stopAndCleanUp() override;

		void stopProcessing();

		QString decodeArguments(const QString & a_arguments);

		void clearFramesQueue();

		void outputStandardError();

		void fillVariables();

		Ui::EncodeDialog m_ui;

		bool m_processing;

		int m_framesTotal;
		int m_framesProcessed;

		hr_time_point m_encodeStartTime;

		QProcess m_encoder;

		std::vector<char> m_framebuffer;

		std::deque<NumberedFrameRef> m_framesQueue;

		int m_lastFrameProcessed;

		std::vector<VariableToken> m_variables;
};

#endif // ENCODE_DIALOG_H_INCLUDED
