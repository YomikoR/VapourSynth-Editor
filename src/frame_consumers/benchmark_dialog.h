#ifndef BENCHMARK_DIALOG_H_INCLUDED
#define BENCHMARK_DIALOG_H_INCLUDED

#include <ui_benchmark_dialog.h>

class VapourSynthScriptProcessor;

class ScriptBenchmarkDialog : public QDialog
{
	Q_OBJECT

	public:

		ScriptBenchmarkDialog(
			VapourSynthScriptProcessor * a_pVapourSynthScriptProcessor,
			QWidget * a_pParent = nullptr);
		virtual ~ScriptBenchmarkDialog();

	private:

		Ui::ScriptBenchmarkDialog m_ui;

		VapourSynthScriptProcessor * m_pVapourSynthScriptProcessor;
};

#endif // BENCHMARK_DIALOG_H_INCLUDED
