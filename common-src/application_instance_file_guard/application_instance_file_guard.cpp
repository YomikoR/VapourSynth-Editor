#include "application_instance_file_guard.h"

#include <QObject>
#include <QStandardPaths>

//==============================================================================

ApplicationInstanceFileGuard::ApplicationInstanceFileGuard(
	const QString & a_fileName)
{
	m_tempDir =
		QStandardPaths::writableLocation(QStandardPaths::TempLocation);
	if(!a_fileName.isEmpty())
		lock(a_fileName);
}

//==============================================================================

ApplicationInstanceFileGuard::~ApplicationInstanceFileGuard()
{
	if(m_file.isOpen())
		unlock();
}

//==============================================================================

bool ApplicationInstanceFileGuard::lock(const QString & a_fileName)
{
	if(m_file.isOpen())
	{
		if(a_fileName == m_fileName)
			return true;

		bool unlocked = unlock();
		if(!unlocked)
			return false;
	}

	QString filePath = m_tempDir + "/" + a_fileName;

	if(QFile::exists(filePath))
	{
		bool deleted = QFile::remove(filePath);
		if(!deleted)
		{
			m_error = QObject::trUtf8("Could not delete file %1")
				.arg(filePath);
			return false;
		}
	}

	m_fileName = a_fileName;
	m_file.setFileName(filePath);
	bool opened = m_file.open(QIODevice::ReadWrite);
	if(!opened)
		m_error = QObject::trUtf8("Could not open file %1").arg(filePath);
	return opened;
}

//==============================================================================

bool ApplicationInstanceFileGuard::unlock()
{
	if(m_file.isOpen())
		m_file.close();

	QString filePath = m_file.fileName();

	if(filePath.isEmpty())
	{
		m_error = QObject::trUtf8("File name is empty.");
		return false;
	}

	if(!QFile::exists(filePath))
		return true;

	bool deleted = QFile::remove(filePath);
	if(!deleted)
		m_error = QObject::trUtf8("Could not delete file %1").arg(filePath);
	return deleted;
}

//==============================================================================

bool ApplicationInstanceFileGuard::isLocked() const
{
	return m_file.isOpen();
}

//==============================================================================

QString ApplicationInstanceFileGuard::error() const
{
	return m_error;
}

//==============================================================================
