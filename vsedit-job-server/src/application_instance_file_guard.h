#ifndef APPLICATION_INSTANCE_FILE_GUARD_H_INCLUDED
#define APPLICATION_INSTANCE_FILE_GUARD_H_INCLUDED

#include <QString>
#include <QFile>

class ApplicationInstanceFileGuard
{
public:

	ApplicationInstanceFileGuard(const QString & a_fileName = QString());
	virtual ~ApplicationInstanceFileGuard();
	bool lock(const QString & a_fileName);
	bool unlock();
	bool isLocked() const;
	QString error() const;

private:

	QFile m_file;
	QString m_error;
	QString m_fileName;
	QString m_tempDir;
};

#endif // APPLICATION_INSTANCE_FILE_GUARD_H_INCLUDED
