#ifndef ENCODE_DIALOG_H_INCLUDED
#define ENCODE_DIALOG_H_INCLUDED

#include <ui_encode_dialog.h>

#include "../../../common-src/jobs/job.h"

class SettingsManager;
class VSScriptLibrary;

class EncodeDialog : public QDialog
{
	Q_OBJECT

public:

	EncodeDialog(SettingsManager * a_pSettingsManager,
		VSScriptLibrary * a_pVSScriptLibrary,
		QWidget * a_pParent = nullptr);
	virtual ~EncodeDialog();

	bool initialize(const QString & a_script, const QString & a_scriptName);

	bool busy() const;

public slots:

	void showActive();

signals:

	void signalWriteLogMessage(const QString & a_style,
		const QString & a_message);

protected:

	virtual void showEvent(QShowEvent * a_pEvent) override;
	virtual void closeEvent(QCloseEvent * a_pEvent) override;

private slots:

	void slotWholeVideoButtonPressed();

	void slotStartEncodeButtonPressed();

	void slotExecutableBrowseButtonPressed();

	void slotArgumentsHelpButtonPressed();

	void slotEncodingPresetSaveButtonPressed();
	void slotEncodingPresetDeleteButtonPressed();
	void slotEncodingPresetComboBoxActivated(const QString & a_text);

	void slotJobStateChanged(JobState a_newState, JobState a_oldState);
	void slotJobProgressChanged();
	void slotJobPropertiesChanged();

	void slotWriteLogMessage(const QString & a_message,
		const QString & a_style);

private:

	void setUpEncodingPresets();

	void setUiEnabled();

	Ui::EncodeDialog m_ui;

	SettingsManager * m_pSettingsManager;

	vsedit::Job * m_pJob;

	std::vector<EncodingPreset> m_encodingPresets;

};

#endif // ENCODE_DIALOG_H_INCLUDED
