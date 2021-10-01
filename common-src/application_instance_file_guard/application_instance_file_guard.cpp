#include "application_instance_file_guard.h"

#include <QObject>
#include <QStandardPaths>

#ifndef Q_OS_WIN
	#include <sys/file.h>
#endif

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

#ifdef Q_OS_WIN
	if(QFile::exists(filePath))
	{
		bool deleted = QFile::remove(filePath);
		if(!deleted)
		{
			m_error = QObject::tr("Could not delete file %1")
				.arg(filePath);
			return false;
		}
	}
#endif

	m_fileName = a_fileName;
	m_file.setFileName(filePath);
	bool opened = m_file.open(QIODevice::ReadWrite);
	if(!opened)
	{
		m_error = QObject::tr("Could not open file %1").arg(filePath);
		return false;
	}

#ifndef Q_OS_WIN
	int result = flock(m_file.handle(), LOCK_EX | LOCK_NB);
	if(result != 0)
	{
		m_error = QObject::tr("Could not lock file %1").arg(filePath);
		m_file.close();
		return false;
	}
#endif

	return true;
}

//==============================================================================

bool ApplicationInstanceFileGuard::unlock()
{
	if(m_file.isOpen())
	{
	#ifndef Q_OS_WIN
		flock(m_file.handle(), LOCK_UN | LOCK_NB);
	#endif
		m_file.close();
	}

	QString filePath = m_file.fileName();

	if(filePath.isEmpty())
	{
		m_error = QObject::tr("File name is empty.");
		return false;
	}

	if(!QFile::exists(filePath))
		return true;

	bool deleted = QFile::remove(filePath);
	if(!deleted)
		m_error = QObject::tr("Could not delete file %1").arg(filePath);
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
