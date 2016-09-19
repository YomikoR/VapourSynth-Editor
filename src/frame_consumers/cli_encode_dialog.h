#ifndef CLI_ENCODE_DIALOG_H_INCLUDED
#define CLI_ENCODE_DIALOG_H_INCLUDED

#include <QProcess>
#include <vector>
#include <deque>

#include <ui_cli_encode_dialog.h>

#include "../common/chrono.h"

struct VSFrameRef;
class VapourSynthScriptProcessor;

struct NumberedFrameRef
{
	int number;
	const VSFrameRef * cpFrameRef;

	NumberedFrameRef(int a_number, const VSFrameRef * a_cpFrameRef);
	bool operator<(const NumberedFrameRef & a_other) const;
};

class CLIEncodeDialog : public QDialog
{
	Q_OBJECT

	public:

		CLIEncodeDialog(
			VapourSynthScriptProcessor * a_pVapourSynthScriptProcessor,
			QWidget * a_pParent = nullptr);
		virtual ~CLIEncodeDialog();

	public slots:

		void call();

	protected:

		void closeEvent(QCloseEvent * a_pEvent) override;

	private slots:

		void slotWholeVideoButtonPressed();

		void slotStartStopBenchmarkButtonPressed();

		void slotReceiveFrame(int a_frameNumber,
			const VSFrameRef * a_cpFrameRef);

	private:

		void stopProcessing();

		QString decodeArguments(const QString & a_arguments);

		void clearFramesQueue();

		void outputStandardError();

		Ui::CLIEncodeDialog m_ui;

		VapourSynthScriptProcessor * m_pVapourSynthScriptProcessor;

		bool m_processing;

		int m_framesTotal;
		int m_framesProcessed;

		hr_time_point m_encodeStartTime;

		QProcess m_encoder;

		std::vector<char> m_framebuffer;

		std::deque<NumberedFrameRef> m_framesQueue;

		int m_lastFrameProcessed;
};

#endif // CLI_ENCODE_DIALOG_H_INCLUDED
