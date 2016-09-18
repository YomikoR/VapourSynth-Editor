#ifndef BENCHMARK_DIALOG_H_INCLUDED
#define BENCHMARK_DIALOG_H_INCLUDED

#include <ui_benchmark_dialog.h>

#include "../common/chrono.h"

struct VSFrameRef;
class VapourSynthScriptProcessor;

class ScriptBenchmarkDialog : public QDialog
{
	Q_OBJECT

	public:

		ScriptBenchmarkDialog(
			VapourSynthScriptProcessor * a_pVapourSynthScriptProcessor,
			QWidget * a_pParent = nullptr);
		virtual ~ScriptBenchmarkDialog();

	public slots:

		void call();

	private slots:

		void slotWholeVideoButtonPressed();

		void slotStartStopBenchmarkButtonPressed();

		void slotReceiveFrame(int a_frameNumber,
			const VSFrameRef * a_cpFrameRef);

	private:

		void stopProcessing();

		Ui::ScriptBenchmarkDialog m_ui;

		VapourSynthScriptProcessor * m_pVapourSynthScriptProcessor;

		bool m_processing;

		int m_framesTotal;
		int m_framesProcessed;

		hr_time_point m_benchmarkStartTime;
};

#endif // BENCHMARK_DIALOG_H_INCLUDED
