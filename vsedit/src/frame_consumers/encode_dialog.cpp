#include "encode_dialog.h"

#include "../../../common-src/helpers.h"
#include "../../../common-src/settings/settings_manager.h"

#include <vapoursynth/VapourSynth4.h>
#include <vapoursynth/VSHelper4.h>

#include <QMessageBox>
#include <QFileDialog>
#include <algorithm>

//==============================================================================

EncodeDialog::EncodeDialog(SettingsManager * a_pSettingsManager,
	VSScriptLibrary * a_pVSScriptLibrary, QWidget * a_pParent) :
	QDialog(a_pParent,
		  Qt::Window
		| Qt::CustomizeWindowHint
		| Qt::WindowMinimizeButtonHint
		| Qt::WindowCloseButtonHint
		)
	, m_pSettingsManager(a_pSettingsManager)
	, m_pJob(nullptr)
{
	vsedit::disableFontKerning(this);
	m_ui.setupUi(this);

	m_pJob = new vsedit::Job(JobProperties(), a_pSettingsManager,
		a_pVSScriptLibrary, this);

	setUpEncodingPresets();

	m_ui.feedbackTextEdit->setName("encode_log");
	m_ui.feedbackTextEdit->setSettingsManager(m_pSettingsManager);
	m_ui.feedbackTextEdit->loadSettings();

	connect(m_ui.wholeVideoButton, SIGNAL(clicked()),
		this, SLOT(slotWholeVideoButtonPressed()));
	connect(m_ui.startEncodeButton, SIGNAL(clicked()),
		this, SLOT(slotStartEncodeButtonPressed()));
	connect(m_ui.pauseEncodeButton, SIGNAL(clicked()),
		m_pJob, SLOT(pause()));
	connect(m_ui.abortEncodeButton, SIGNAL(clicked()),
		m_pJob, SLOT(abort()));
	connect(m_ui.executableBrowseButton, SIGNAL(clicked()),
		this, SLOT(slotExecutableBrowseButtonPressed()));
	connect(m_ui.argumentsHelpButton, SIGNAL(clicked()),
		this, SLOT(slotArgumentsHelpButtonPressed()));

	connect(m_pJob, &vsedit::Job::signalStateChanged,
		this, &EncodeDialog::slotJobStateChanged);
	connect(m_pJob, &vsedit::Job::signalProgressChanged,
		this, &EncodeDialog::slotJobProgressChanged);
	connect(m_pJob, &vsedit::Job::signalPropertiesChanged,
		this, &EncodeDialog::slotJobPropertiesChanged);
	connect(m_pJob, SIGNAL(signalLogMessage(const QString &, const QString &)),
		this, SLOT(slotWriteLogMessage(const QString &, const QString &)));
}

// END OF EncodeDialog::EncodeDialog(SettingsManager * a_pSettingsManager,
//		VSScriptLibrary * a_pVSScriptLibrary, QWidget * a_pParent)
//==============================================================================

EncodeDialog::~EncodeDialog()
{
}

// END OF EncodeDialog::~EncodeDialog()
//==============================================================================

bool EncodeDialog::initialize(const QString & a_script,
	const QString & a_scriptName)
{
	if(m_pJob->isActive())
		return false;

	JobProperties properties;
	properties.scriptName = a_scriptName;
	properties.scriptText = a_script;
	m_pJob->setProperties(properties);
	bool initialized = m_pJob->initialize();
	if(!initialized)
		return false;

	properties = m_pJob->properties();
	const VSVideoInfo * cpVideoInfo = m_pJob->videoInfo();
	Q_ASSERT(cpVideoInfo);

	m_ui.feedbackTextEdit->clear();
	setWindowTitle(tr("Encode: %1").arg(a_scriptName));
	QString text = tr("Ready to encode script %1").arg(a_scriptName);
	m_ui.feedbackTextEdit->addEntry(text);
	m_ui.metricsEdit->clear();
	int lastFrame = cpVideoInfo->numFrames - 1;
	m_ui.fromFrameSpinBox->setMaximum(lastFrame);
	m_ui.toFrameSpinBox->setMaximum(lastFrame);
	m_ui.fromFrameSpinBox->setValue(properties.firstFrameReal);
	m_ui.toFrameSpinBox->setValue(properties.lastFrameReal);
	m_ui.processingProgressBar->setMaximum(lastFrame);
	m_ui.processingProgressBar->setValue(0);

	setUiEnabled();

	return true;
}

// END OF bool EncodeDialog::initialize(const QString & a_script,
//		const QString & a_scriptName)
//==============================================================================

bool EncodeDialog::busy() const
{
	return m_pJob->isActive();
}

// END OF bool EncodeDialog::busy() const
//==============================================================================

void EncodeDialog::showActive()
{
	if(isMinimized())
		showNormal();
	else
		show();
	QApplication::setActiveWindow(this);
}

// END OF void EncodeDialog::showActive()
//==============================================================================

void EncodeDialog::showEvent(QShowEvent * a_pEvent)
{
	QDialog::showEvent(a_pEvent);
}

// END OF void EncodeDialog::showEvent(QShowEvent * a_pEvent)
//==============================================================================

void EncodeDialog::closeEvent(QCloseEvent * a_pEvent)
{
	if(m_pJob->isActive())
	{
		a_pEvent->ignore();
		return;
	}

	m_pJob->cleanUpEncoding();

	QDialog::closeEvent(a_pEvent);
}

// END OF void EncodeDialog::closeEvent(QCloseEvent * a_pEvent)
//==============================================================================

void EncodeDialog::slotWholeVideoButtonPressed()
{
	const VSVideoInfo * cpVideoInfo = m_pJob->videoInfo();
	Q_ASSERT(cpVideoInfo);
	int lastFrame = cpVideoInfo->numFrames - 1;
	m_ui.fromFrameSpinBox->setValue(0);
	m_ui.toFrameSpinBox->setValue(lastFrame);
}

// END OF void EncodeDialog::slotWholeVideoButtonPressed()
//==============================================================================

void EncodeDialog::slotStartEncodeButtonPressed()
{
	if(m_pJob->state() == JobState::Paused)
	{
		m_pJob->start();
		return;
	}

	if(m_pJob->isActive())
		return;

	int firstFrame = m_ui.fromFrameSpinBox->value();
	int lastFrame = m_ui.toFrameSpinBox->value();

	if(firstFrame > lastFrame)
	{
		m_ui.feedbackTextEdit->addEntry(tr("First frame number is "
			"larger than the last frame number."), LOG_STYLE_WARNING);
		return;
	}

	m_pJob->setFirstFrame(firstFrame);
	m_pJob->setLastFrame(lastFrame);
	m_pJob->setExecutablePath(vsedit::resolvePathFromApplication(
		m_ui.executablePathEdit->text()));
	m_pJob->setArguments(m_ui.argumentsTextEdit->toPlainText());
	m_pJob->setEncodingHeaderType((EncodingHeaderType)
		m_ui.headerTypeComboBox->currentData().toInt());
	m_pJob->start();
}

// END OF void EncodeDialog::slotStartEncodeButtonPressed()
//==============================================================================

void EncodeDialog::slotExecutableBrowseButtonPressed()
{
	QString applicationPath = QCoreApplication::applicationDirPath();
	QFileDialog fileDialog;
	fileDialog.setWindowTitle(tr("Choose encoder executable"));
	fileDialog.setDirectory(applicationPath);

	if(!fileDialog.exec())
		return;

	QStringList filesList = fileDialog.selectedFiles();
	m_ui.executablePathEdit->setText(filesList[0]);
}

// END OF void EncodeDialog::slotExecutableBrowseButtonPressed()
//==============================================================================

void EncodeDialog::slotArgumentsHelpButtonPressed()
{
	JobVariables variables;
	QString argumentsHelpString = tr("Use following placeholders:");
	for(const vsedit::VariableToken & variable : variables.variables())
	{
		argumentsHelpString += QString("\n%1 - %2")
			.arg(variable.token).arg(variable.description);
	}
	QString title = tr("Encoder arguments");
	QMessageBox msgBox(this);
	msgBox.setWindowTitle(title);
	msgBox.setText(argumentsHelpString);
	vsedit::disableFontKerning(&msgBox);
	msgBox.exec();
}

// END OF void EncodeDialog::slotArgumentsHelpButtonPressed()
//==============================================================================

void EncodeDialog::slotEncodingPresetSaveButtonPressed()
{
	EncodingPreset preset(m_ui.encodingPresetComboBox->currentText());
	if(preset.name.isEmpty())
	{
		m_ui.feedbackTextEdit->addEntry(
			tr("Preset name must not be empty."), LOG_STYLE_WARNING);
		return;
	}

	if(preset.type == EncodingType::CLI)
	{
		preset.executablePath = m_ui.executablePathEdit->text();
		if(preset.executablePath.isEmpty())
		{
			m_ui.feedbackTextEdit->addEntry(
				tr("Executable path must not be empty."),
				LOG_STYLE_WARNING);
			return;
		}

		preset.arguments = m_ui.argumentsTextEdit->toPlainText();
	}

	preset.headerType = (EncodingHeaderType)
		m_ui.headerTypeComboBox->currentData().toInt();

	bool success = m_pSettingsManager->saveEncodingPreset(preset);
	if(!success)
	{
		m_ui.feedbackTextEdit->addEntry(tr("Error saving preset."),
			LOG_STYLE_ERROR);
		return;
	}

	std::vector<EncodingPreset>::iterator it = std::find(
		m_encodingPresets.begin(), m_encodingPresets.end(), preset);
	if(it == m_encodingPresets.end())
	{
		Q_ASSERT(m_ui.encodingPresetComboBox->findText(preset.name) == -1);
		m_encodingPresets.push_back(preset);
		m_ui.encodingPresetComboBox->addItem(preset.name);
		m_ui.encodingPresetComboBox->model()->sort(0);
	}
	else
	{
		Q_ASSERT(m_ui.encodingPresetComboBox->findText(preset.name) != -1);
		*it = preset;
	}

	m_ui.feedbackTextEdit->addEntry(tr("Preset \'%1\' saved.")
		.arg(preset.name), LOG_STYLE_POSITIVE);
}

// END OF void EncodeDialog::slotEncodingPresetSaveButtonPressed()
//==============================================================================

void EncodeDialog::slotEncodingPresetDeleteButtonPressed()
{
	EncodingPreset preset(m_ui.encodingPresetComboBox->currentText());
	if(preset.name.isEmpty())
		return;

	QMessageBox quesBox(this);
	vsedit::disableFontKerning(&quesBox);
	quesBox.setWindowTitle(tr("Delete preset"));
	quesBox.setText(tr("Do you really want to delete "
		"preset \'%1\'?").arg(preset.name));
	quesBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
	quesBox.setDefaultButton(QMessageBox::No);
	int result = quesBox.exec();
	if(result == QMessageBox::No)
		return;

	std::vector<EncodingPreset>::iterator it = std::find(
		m_encodingPresets.begin(), m_encodingPresets.end(), preset);
	if(it == m_encodingPresets.end())
	{
		Q_ASSERT(m_ui.encodingPresetComboBox->findText(preset.name) == -1);
		m_ui.feedbackTextEdit->addEntry(tr("Error deleting preset. "
			"Preset was never saved."), LOG_STYLE_ERROR);
		return;
	}

	int index = m_ui.encodingPresetComboBox->findText(preset.name);
	Q_ASSERT(index != -1);
	m_ui.encodingPresetComboBox->removeItem(index);
	m_encodingPresets.erase(it);
	m_ui.encodingPresetComboBox->setCurrentIndex(0);
	slotEncodingPresetComboBoxActivated(
		m_ui.encodingPresetComboBox->currentText());

	bool success = m_pSettingsManager->deleteEncodingPreset(preset.name);
	if(!success)
	{
		m_ui.feedbackTextEdit->addEntry(tr("Error deleting "
			"preset \'%1\'.").arg(preset.name), LOG_STYLE_ERROR);
		return;
	}

	m_ui.feedbackTextEdit->addEntry(tr("Preset \'%1\' deleted.")
		.arg(preset.name), LOG_STYLE_POSITIVE);
}

// END OF void EncodeDialog::slotEncodingPresetDeleteButtonPressed()
//==============================================================================

void EncodeDialog::slotEncodingPresetComboBoxActivated(const QString & a_text)
{
	if(a_text.isEmpty())
	{
		m_ui.executablePathEdit->clear();
		m_ui.argumentsTextEdit->clear();
		return;
	}

	EncodingPreset preset(a_text);

	std::vector<EncodingPreset>::iterator it = std::find(
		m_encodingPresets.begin(), m_encodingPresets.end(), preset);
	if(it == m_encodingPresets.end())
	{
		m_ui.feedbackTextEdit->addEntry(tr("Error. There is no preset "
			"named \'%1\'.").arg(preset.name), LOG_STYLE_ERROR);
		return;
	}

	preset = *it;

	m_ui.executablePathEdit->setText(preset.executablePath);
	m_ui.argumentsTextEdit->setPlainText(preset.arguments);

	int headerTypeIndex =
		m_ui.headerTypeComboBox->findData((int)preset.headerType);
	if(headerTypeIndex < 0)
	{
		m_ui.feedbackTextEdit->addEntry(tr("Error. Preset \'%1\' "
			"has unknown header type.").arg(preset.name), LOG_STYLE_ERROR);
		headerTypeIndex = 0;
	}
	m_ui.headerTypeComboBox->setCurrentIndex(headerTypeIndex);
}

// END OF void EncodeDialog::slotEncodingPresetComboBoxActivated(
//		const QString & a_text)
//==============================================================================

void EncodeDialog::slotJobStateChanged(JobState a_newState, JobState a_oldState)
{
	setUiEnabled();

	JobProperties properties = m_pJob->properties();

	JobState pauseStates[] = {JobState::Pausing, JobState::Paused};

	JobState failStates[] = {JobState::FailedCleanUp, JobState::Failed,
		JobState::Aborted, JobState::Aborting};

	if(a_newState == JobState::Running)
	{
		JobState idleStates[] = {JobState::Waiting, JobState::Failed,
			JobState::Aborted, JobState::Completed};
		if(!vsedit::contains(idleStates, a_oldState))
			return;

		setWindowTitle(tr("0% Encode: %1").arg(properties.scriptName));

		m_ui.processingProgressBar->setMaximum(properties.framesTotal());
		m_ui.processingProgressBar->setValue(0);
	}
}

// END OF void EncodeDialog::slotJobStateChanged(JobState a_newState,
//		JobState a_oldState)
//==============================================================================

void EncodeDialog::slotJobProgressChanged()
{
	JobProperties properties = m_pJob->properties();

	m_ui.processingProgressBar->setValue(properties.framesProcessed);

	QDateTime now = QDateTime::currentDateTimeUtc();
	double passed = ((double)properties.timeStarted.msecsTo(now)) / 1000.0;
	QString passedString = vsedit::timeToString(passed);

	QString text = tr("Time elapsed: %1 - %2 FPS")
		.arg(passedString).arg(QString::number(properties.fps, 'f', 20));

	if((properties.framesProcessed > 0) &&
		(properties.framesProcessed < properties.framesTotal()))
	{
		Q_ASSERT(properties.fps > 0.0);
		double estimated = (properties.framesTotal() -
			properties.framesProcessed) / properties.fps;
		QString estimatedString = vsedit::timeToString(estimated);
		text += tr("; estimated time to finish: %1")
			.arg(estimatedString);
	}

	m_ui.metricsEdit->setText(text);

	int percentage = (int)((double)properties.framesProcessed * 100.0 /
		(double)properties.framesTotal());
	setWindowTitle(tr("%1% Encode: %2")
		.arg(percentage).arg(properties.scriptName));
}

// END OF void EncodeDialog::slotJobProgressChanged()
//==============================================================================

void EncodeDialog::slotJobPropertiesChanged()
{
	JobProperties properties = m_pJob->properties();
	m_ui.processingProgressBar->setMaximum(properties.framesTotal());
}

// END OF void EncodeDialog::slotJobPropertiesChanged()
//==============================================================================

void EncodeDialog::slotWriteLogMessage(const QString & a_message,
	const QString & a_style)
{
	if(!isVisible())
		emit signalWriteLogMessage(a_message, a_style);
	else
		m_ui.feedbackTextEdit->addEntry(a_message, a_style);
}

// END OF void EncodeDialog::slotWriteLogMessage(const QString & a_message,
//		const QString & a_style)
//==============================================================================

void EncodeDialog::setUpEncodingPresets()
{
	m_encodingPresets = m_pSettingsManager->getAllEncodingPresets();
	for(const EncodingPreset & preset : m_encodingPresets)
		m_ui.encodingPresetComboBox->addItem(preset.name);

	m_ui.headerTypeComboBox->addItem(tr("No header"),
		(int)EncodingHeaderType::NoHeader);
	m_ui.headerTypeComboBox->addItem(tr("Y4M"),
		(int)EncodingHeaderType::Y4M);

	connect(m_ui.encodingPresetSaveButton, SIGNAL(clicked()),
		this, SLOT(slotEncodingPresetSaveButtonPressed()));
	connect(m_ui.encodingPresetDeleteButton, SIGNAL(clicked()),
		this, SLOT(slotEncodingPresetDeleteButtonPressed()));
	connect(m_ui.encodingPresetComboBox, SIGNAL(textActivated(const QString &)),
		this, SLOT(slotEncodingPresetComboBoxActivated(const QString &)));

	m_ui.encodingPresetComboBox->setCurrentIndex(0);
	slotEncodingPresetComboBoxActivated(
		m_ui.encodingPresetComboBox->currentText());
}

// END OF void EncodeDialog::setUpEncodingPresets()
//==============================================================================

void EncodeDialog::setUiEnabled()
{
}

// END OF void EncodeDialog::setUiEnabled()
//==============================================================================
