#include "benchmark_dialog.h"

#include "../common/helpers.h"
#include "../vapoursynth/vapoursynthscriptprocessor.h"

#include <vapoursynth/VapourSynth.h>

//==============================================================================

ScriptBenchmarkDialog::ScriptBenchmarkDialog(
	VapourSynthScriptProcessor * a_pVapourSynthScriptProcessor,
	QWidget * a_pParent) :
	QDialog(a_pParent, (Qt::WindowFlags)0
		| Qt::Window
		| Qt::CustomizeWindowHint
		| Qt::WindowMinimizeButtonHint
		| Qt::WindowCloseButtonHint
		)
	, m_pVapourSynthScriptProcessor(a_pVapourSynthScriptProcessor)
{
	m_ui.setupUi(this);
	setWindowIcon(QIcon(":time.png"));
}

// END OF ScriptBenchmarkDialog::ScriptBenchmarkDialog(
//		VapourSynthScriptProcessor * a_pVapourSynthScriptProcessor,
//		QWidget * a_pParent
//==============================================================================

ScriptBenchmarkDialog::~ScriptBenchmarkDialog()
{
}

// END OF ScriptBenchmarkDialog::~ScriptBenchmarkDialog()
//==============================================================================
