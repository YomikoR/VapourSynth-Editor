#include "job_edit_dialog.h"

#include "../../../common-src/settings/settings_manager.h"
#include "../../../common-src/vapoursynth/vapoursynth_script_processor.h"
#include "../../../common-src/jobs/job_variables.h"
#include "../../../common-src/helpers.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <map>
#include <limits>

//==============================================================================

JobEditDialog::JobEditDialog(SettingsManager * a_pSettingsManager,
	VSScriptLibrary * a_pVSScriptLibrary, QWidget * a_pParent) :
	  QDialog(a_pParent, (Qt::WindowFlags)0
		| Qt::Dialog
		| Qt::CustomizeWindowHint
		| Qt::WindowTitleHint
		| Qt::WindowMinimizeButtonHint
		| Qt::WindowMaximizeButtonHint
		| Qt::WindowCloseButtonHint)
	, m_pSettingsManager(a_pSettingsManager)
	, m_pVSScriptLibrary(a_pVSScriptLibrary)
{
	m_ui.setupUi(this);

	JobType jobTypes[] = {JobType::EncodeScriptCLI, JobType::RunProcess,
		JobType::RunShellCommand};
	for(const JobType & jobType : jobTypes)
		m_ui.jobTypeComboBox->addItem(JobProperties::typeName(jobType),
			(int)jobType);
	m_ui.jobTypeComboBox->setCurrentIndex(0);
	slotJobTypeChanged(m_ui.jobTypeComboBox->currentIndex());

	m_ui.encodingHeaderTypeComboBox->addItem(tr("No header"),
		(int)EncodingHeaderType::NoHeader);
	m_ui.encodingHeaderTypeComboBox->addItem(tr("Y4M"),
		(int)EncodingHeaderType::Y4M);

	m_ui.encodingFirstFrameSpinBox->setMaximum(std::numeric_limits<int>::max());
	m_ui.encodingLastFrameSpinBox->setMaximum(std::numeric_limits<int>::max());

	setUpEncodingPresets();

	connect(m_ui.jobTypeComboBox, SIGNAL(currentIndexChanged(int)),
		this, SLOT(slotJobTypeChanged(int)));
	connect(m_ui.encodingScriptBrowseButton, SIGNAL(clicked()),
		this, SLOT(slotEncodingScriptBrowseButtonClicked()));
	connect(m_ui.encodingPresetComboBox, SIGNAL(activated(const QString &)),
		this, SLOT(slotEncodingPresetComboBoxActivated(const QString &)));
	connect(m_ui.encodingPresetSaveButton, SIGNAL(clicked()),
		this, SLOT(slotEncodingPresetSaveButtonClicked()));
	connect(m_ui.encodingPresetDeleteButton, SIGNAL(clicked()),
		this, SLOT(slotEncodingPresetDeleteButton()));
	connect(m_ui.encodingExecutableBrowseButton, SIGNAL(clicked()),
		this, SLOT(slotEncodingExecutableBrowseButtonClicked()));
	connect(m_ui.encodingArgumentsHelpButton, SIGNAL(clicked()),
		this, SLOT(slotEncodingArgumentsHelpButtonClicked()));
	connect(m_ui.encodingFramesFromVideoButton, SIGNAL(clicked()),
		this, SLOT(slotEncodingFramesFromVideoButtonClicked()));
	connect(m_ui.processExecutableBrowseButton, SIGNAL(clicked()),
		this, SLOT(slotProcessExecutableBrowseButtonClicked()));
	connect(m_ui.jobSaveButton, SIGNAL(clicked()), this, SLOT(accept()));
	connect(m_ui.cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

// END OF JobEditDialog::JobEditDialog(SettingsManager * a_pSettingsManager,
//		QWidget * a_pParent)
//==============================================================================

JobEditDialog::~JobEditDialog()
{
}

// END OF JobEditDialog::~JobEditDialog()
//==============================================================================

JobProperties JobEditDialog::jobProperties() const
{
	JobProperties newProperties;
	newProperties.type = (JobType)m_ui.jobTypeComboBox->currentData().toInt();
	newProperties.scriptName = m_ui.encodingScriptPathEdit->text();
	newProperties.encodingHeaderType = (EncodingHeaderType)m_ui
		.encodingHeaderTypeComboBox->currentData().toInt();
	if(newProperties.type == JobType::EncodeScriptCLI)
	{
		newProperties.executablePath = m_ui.encodingExecutablePathEdit->text();
		newProperties.arguments = m_ui.encodingArgumentsTextEdit->toPlainText();
	}
	else if(newProperties.type == JobType::RunProcess)
	{
		newProperties.executablePath = m_ui.processExecutablePathEdit->text();
		newProperties.arguments = m_ui.processArgumentsTextEdit->toPlainText();
	}
	newProperties.shellCommand = m_ui.shellCommandTextEdit->toPlainText();
	newProperties.firstFrame = m_ui.encodingFirstFrameSpinBox->value();
	newProperties.lastFrame = m_ui.encodingLastFrameSpinBox->value();
	return newProperties;
}

// END OF JobProperties JobEditDialog::jobProperties() const
//==============================================================================

int JobEditDialog::call(const QString & a_title,
	const JobProperties & a_jobProperties)
{
	setUpEncodingPresets();
	setWindowTitle(a_title);

	int index = m_ui.jobTypeComboBox->findData((int)a_jobProperties.type);
	m_ui.jobTypeComboBox->setCurrentIndex(index);
	m_ui.encodingScriptPathEdit->setText(a_jobProperties.scriptName);
	m_ui.encodingPresetComboBox->clearEditText();
	index = m_ui.encodingHeaderTypeComboBox->findData(
		(int)a_jobProperties.encodingHeaderType);
	m_ui.encodingHeaderTypeComboBox->setCurrentIndex(index);
	m_ui.encodingExecutablePathEdit->setText(a_jobProperties.executablePath);
	m_ui.encodingArgumentsTextEdit->setPlainText(a_jobProperties.arguments);
	m_ui.processExecutablePathEdit->setText(a_jobProperties.executablePath);
	m_ui.processArgumentsTextEdit->setPlainText(a_jobProperties.arguments);
	m_ui.shellCommandTextEdit->setPlainText(a_jobProperties.shellCommand);

	m_ui.encodingFirstFrameSpinBox->setValue(a_jobProperties.firstFrame);
	m_ui.encodingLastFrameSpinBox->setValue(a_jobProperties.lastFrame);

	return exec();
}

// END OF int JobEditDialog::call(const QString & a_title,
//		const JobProperties & a_jobProperties)
//==============================================================================

void JobEditDialog::slotJobTypeChanged(int a_index)
{
	std::map<JobType, QWidget *> panels =
	{
		{JobType::EncodeScriptCLI, m_ui.encodingPanel},
		{JobType::RunProcess, m_ui.processPanel},
		{JobType::RunShellCommand, m_ui.shellCommandPanel},
	};

	JobType jobType = (JobType)m_ui.jobTypeComboBox->itemData(a_index).toInt();

	for(const std::pair<JobType, QWidget *> & pair : panels)
	{
		if(pair.first == jobType)
			pair.second->setVisible(true);
		else
			pair.second->setVisible(false);
	}
}

// END OF void JobEditDialog::slotJobTypeChanged(int a_index)
//==============================================================================

void JobEditDialog::slotEncodingScriptBrowseButtonClicked()
{
	QFileDialog fileDialog(this);
	fileDialog.setWindowTitle(tr("Open VapourSynth script"));
	fileDialog.setNameFilter(
		tr("VapourSynth script (*.vpy);;All files (*)"));

	QString path = m_ui.encodingScriptPathEdit->text();
	if(path.isEmpty())
		path = m_pSettingsManager->getLastUsedPath();
	QFileInfo fileInfo(path);
	QString dirPath = fileInfo.absoluteDir().path();
	fileDialog.setDirectory(dirPath);
	fileDialog.selectFile(fileInfo.fileName());

	if(!fileDialog.exec())
		return;

	QStringList filesList = fileDialog.selectedFiles();
	m_ui.encodingScriptPathEdit->setText(filesList[0]);
	m_pSettingsManager->setLastUsedPath(filesList[0]);
}

// END OF void JobEditDialog::slotEncodingScriptBrowseButtonClicked()
//==============================================================================

void JobEditDialog::slotEncodingPresetComboBoxActivated(const QString & a_text)
{
	if(a_text.isEmpty())
	{
		m_ui.encodingExecutablePathEdit->clear();
		m_ui.encodingArgumentsTextEdit->clear();
		return;
	}

	EncodingPreset preset(a_text);

	std::vector<EncodingPreset>::iterator it = std::find(
		m_encodingPresets.begin(), m_encodingPresets.end(), preset);
	if(it == m_encodingPresets.end())
		return;

	preset = *it;

	m_ui.encodingExecutablePathEdit->setText(preset.executablePath);
	m_ui.encodingArgumentsTextEdit->setPlainText(preset.arguments);

	int headerTypeIndex =
		m_ui.encodingHeaderTypeComboBox->findData((int)preset.headerType);
	if(headerTypeIndex < 0)
		headerTypeIndex = 0;
	m_ui.encodingHeaderTypeComboBox->setCurrentIndex(headerTypeIndex);
}

// END OF void JobEditDialog::slotEncodingPresetComboBoxActivated(
//		const QString & a_text)
//==============================================================================

void JobEditDialog::slotEncodingPresetSaveButtonClicked()
{
	EncodingPreset preset(m_ui.encodingPresetComboBox->currentText());
	if(preset.name.isEmpty())
	{
		QMessageBox::warning(this, tr("Preset save error."),
			tr("Preset name must not be empty."));
		return;
	}

	if(preset.type == EncodingType::CLI)
	{
		preset.executablePath = m_ui.encodingExecutablePathEdit->text();
		if(preset.executablePath.isEmpty())
		{
			QMessageBox::warning(this, tr("Preset save error."),
				tr("Executable path must not be empty."));
			return;
		}

		preset.arguments = m_ui.encodingArgumentsTextEdit->toPlainText();
	}

	preset.headerType = (EncodingHeaderType)
		m_ui.encodingHeaderTypeComboBox->currentData().toInt();

	bool success = m_pSettingsManager->saveEncodingPreset(preset);
	if(!success)
	{
		QMessageBox::critical(this, tr("Preset save error."),
			tr("Error saving encoding preset."));
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
}

// END OF void JobEditDialog::slotEncodingPresetSaveButtonClicked()
//==============================================================================

void JobEditDialog::slotEncodingPresetDeleteButton()
{
	EncodingPreset preset(m_ui.encodingPresetComboBox->currentText());
	if(preset.name.isEmpty())
		return;

	QMessageBox::StandardButton result = QMessageBox::question(this,
		tr("Delete preset"), tr("Do you really want to delete "
		"preset \'%1\'?").arg(preset.name),
		QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No),
		QMessageBox::No);
	if(result == QMessageBox::No)
		return;

	std::vector<EncodingPreset>::iterator it = std::find(
		m_encodingPresets.begin(), m_encodingPresets.end(), preset);
	if(it == m_encodingPresets.end())
	{
		Q_ASSERT(m_ui.encodingPresetComboBox->findText(preset.name) == -1);
		QMessageBox::critical(this, tr("Preset delete error."),
			tr("Error deleting preset. Preset was never saved."));
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
		QMessageBox::critical(this, tr("Preset delete error."),
			tr("Error deleting preset \'%1\'.").arg(preset.name));
		return;
	}
}

// END OF void JobEditDialog::slotEncodingPresetDeleteButton()
//==============================================================================

void JobEditDialog::slotEncodingExecutableBrowseButtonClicked()
{
	QString executablePath = chooseExecutable(
		tr("Choose encoder executable"),
		m_ui.encodingExecutablePathEdit->text());

	if(!executablePath.isEmpty())
		m_ui.encodingExecutablePathEdit->setText(executablePath);
}

// END OF void JobEditDialog::slotEncodingExecutableBrowseButtonClicked()
//==============================================================================

void JobEditDialog::slotEncodingFramesFromVideoButtonClicked()
{
	QString scriptName = m_ui.encodingScriptPathEdit->text();
	QString absoluteScriptPath =
		vsedit::resolvePathFromApplication(scriptName);
	QFile scriptFile(absoluteScriptPath);
	bool opened = scriptFile.open(QIODevice::ReadOnly);
	if(!opened)
		return;

	QString script = QString::fromUtf8(scriptFile.readAll());
	scriptFile.close();

	VapourSynthScriptProcessor processor(m_pSettingsManager,
		m_pVSScriptLibrary);
	bool initialized = processor.initialize(script, scriptName);
	if(!initialized)
		return;

	const VSVideoInfo * cpVideoInfo = processor.videoInfo();
	m_ui.encodingFirstFrameSpinBox->setValue(0);
	m_ui.encodingLastFrameSpinBox->setValue(cpVideoInfo->numFrames - 1);
}

// END OF void JobEditDialog::slotEncodingFramesFromVideoButtonClicked()
//==============================================================================

void JobEditDialog::slotEncodingArgumentsHelpButtonClicked()
{
	JobVariables variables;
	QString argumentsHelpString = tr("Use following placeholders:");
	for(const vsedit::VariableToken & variable : variables.variables())
	{
		argumentsHelpString += QString("\n%1 - %2")
			.arg(variable.token).arg(variable.description);
	}
	QString title = tr("Encoder arguments");
	QMessageBox::information(this, title, argumentsHelpString);
}

// END OF void JobEditDialog::slotEncodingArgumentsHelpButtonClicked()
//==============================================================================

void JobEditDialog::slotProcessExecutableBrowseButtonClicked()
{
	QString executablePath = chooseExecutable(
		tr("Choose process executable"),
		m_ui.processExecutablePathEdit->text());

	if(!executablePath.isEmpty())
		m_ui.processExecutablePathEdit->setText(executablePath);
}

// END OF void JobEditDialog::slotProcessExecutableBrowseButtonClicked()
//==============================================================================

void JobEditDialog::setUpEncodingPresets()
{
	m_ui.encodingPresetComboBox->clear();

	m_encodingPresets = m_pSettingsManager->getAllEncodingPresets();
	for(const EncodingPreset & preset : m_encodingPresets)
		m_ui.encodingPresetComboBox->addItem(preset.name);

	m_ui.encodingPresetComboBox->setCurrentIndex(0);
	slotEncodingPresetComboBoxActivated(
		m_ui.encodingPresetComboBox->currentText());
}

// END OF void JobEditDialog::setUpEncodingPresets()
//==============================================================================

QString JobEditDialog::chooseExecutable(const QString & a_dialogTitle,
	const QString & a_initialPath)
{
	QString applicationPath = QCoreApplication::applicationDirPath();

	QFileDialog fileDialog;
	fileDialog.setWindowTitle(a_dialogTitle);
	if(a_initialPath.isEmpty())
		fileDialog.setDirectory(applicationPath);
	else
		fileDialog.selectFile(a_initialPath);

#ifdef Q_OS_WIN
	fileDialog.setNameFilter("*.exe");
#endif

	if(!fileDialog.exec())
		return QString();

	QStringList filesList = fileDialog.selectedFiles();
	return filesList[0];
}

// END OF QString JobEditDialog::chooseExecutable(const QString & a_dialogTitle,
//		const QString & a_initialPath)
//==============================================================================
