#ifndef BENCHMARK_DIALOG_H_INCLUDED
#define BENCHMARK_DIALOG_H_INCLUDED

#include <ui_benchmark_dialog.h>

#include "../vapoursynth/vs_script_processor_dialog.h"
#include "../common/chrono.h"

struct VSFrameRef;
class SettingsManager;

class ScriptBenchmarkDialog : public VSScriptProcessorDialog
{
	Q_OBJECT

	public:

		ScriptBenchmarkDialog(SettingsManager * a_pSettingsManager,
			QWidget * a_pParent = nullptr);
		virtual ~ScriptBenchmarkDialog();

	public slots:

		void call();

	protected slots:

		virtual void slotWriteLogMessage(int a_messageType,
			const QString & a_message) override;

		virtual void slotReceiveFrame(int a_frameNumber, int a_outputIndex,
			const VSFrameRef * a_cpOutputFrameRef,
			const VSFrameRef * a_cpPreviewFrameRef) override;

		void slotWholeVideoButtonPressed();

		void slotStartStopBenchmarkButtonPressed();

	protected:

		virtual void stopAndCleanUp() override;

		void stopProcessing();

		Ui::ScriptBenchmarkDialog m_ui;

		bool m_processing;

		int m_framesTotal;
		int m_framesProcessed;

		hr_time_point m_benchmarkStartTime;
};

#endif // BENCHMARK_DIALOG_H_INCLUDED
