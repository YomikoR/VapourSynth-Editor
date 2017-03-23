#include "job_dependencies_delegate.h"

#include "jobs_model.h"

#include <QListWidget>

//==============================================================================

JobDependenciesDelegate::JobDependenciesDelegate(QObject * a_pParent) :
	QStyledItemDelegate(a_pParent)
{
}

// END OF JobDependenciesDelegate::JobDependenciesDelegate(QObject * a_pParent)
//==============================================================================

JobDependenciesDelegate::~JobDependenciesDelegate()
{
}

// END OF JobDependenciesDelegate::~JobDependenciesDelegate()
//==============================================================================

QWidget * JobDependenciesDelegate::createEditor(QWidget * a_pParent,
	const QStyleOptionViewItem & a_option,
	const QModelIndex & a_index) const
{
	(void)a_option;
	(void)a_index;
	return new QListWidget(a_pParent);
}

// END OF QWidget * JobDependenciesDelegate::createEditor(QWidget * a_pParent,
//		const QStyleOptionViewItem & a_option,
//		const QModelIndex & a_index) const
//==============================================================================

void JobDependenciesDelegate::setEditorData(QWidget * a_pEditor,
	const QModelIndex & a_index) const
{
	QListWidget * pListWidget = qobject_cast<QListWidget *>(a_pEditor);
	if(!pListWidget)
		return;

	const JobsModel * cpModel =
		qobject_cast<const JobsModel *>(a_index.model());
	if(!cpModel)
		return;

	const vsedit::Job * cpJob = cpModel->job(a_index.row());
	if(!cpJob)
		return;

	std::vector<QUuid> dependencyIds = cpJob->dependsOnJobIds();

	pListWidget->clear();
	for(int i = 0; i < a_index.row(); ++i)
	{
		QString name = trUtf8("Job %1").arg(i + 1);
		QListWidgetItem * pItem = new QListWidgetItem(name, pListWidget);
		QUuid id = cpModel->job(i)->id();
		pItem->setData(Qt::UserRole, id);
		pItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable |
			Qt::ItemIsUserCheckable);
		bool depends = vsedit::contains(dependencyIds, id);
		pItem->setCheckState(depends ? Qt::Checked : Qt::Unchecked);
	}
}

// END OF void JobDependenciesDelegate::setEditorData(QWidget * a_pEditor,
//		const QModelIndex & a_index) const
//==============================================================================

void JobDependenciesDelegate::setModelData(QWidget * a_pEditor,
	QAbstractItemModel * a_pModel, const QModelIndex & a_index) const
{
	QListWidget * pListWidget = qobject_cast<QListWidget *>(a_pEditor);
	if(!pListWidget)
		return;

	JobsModel * pModel = qobject_cast<JobsModel *>(a_pModel);
	if(!pModel)
		return;

	std::vector<QUuid> dependencyIds;

	for(int i = 0; i < pListWidget->count(); ++i)
	{
		QListWidgetItem * pItem = pListWidget->item(i);
		if(pItem->checkState() != Qt::Checked)
			continue;
		QUuid id = pItem->data(Qt::UserRole).toUuid();
		dependencyIds.push_back(id);
	}

	pModel->setJobDependsOnIds(a_index.row(), dependencyIds);
}

// END OF void JobDependenciesDelegate::setModelData(QWidget * a_pEditor,
//		QAbstractItemModel * a_pModel, const QModelIndex & a_index) const
//==============================================================================

void JobDependenciesDelegate::updateEditorGeometry(QWidget * a_pEditor,
	const QStyleOptionViewItem & a_option, const QModelIndex & a_index) const
{
	(void)a_index;
	QListWidget * pListWidget = qobject_cast<QListWidget *>(a_pEditor);
	if(!pListWidget)
		return;
	pListWidget->move(a_option.rect.topLeft());
}

// END OF void JobDependenciesDelegate::updateEditorGeometry(
//		QWidget * a_pEditor, const QStyleOptionViewItem & a_option,
//		const QModelIndex & a_index) const
//==============================================================================
