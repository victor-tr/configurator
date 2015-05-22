/*! \file */

#ifndef HEADERVIEWMODEL_H
#define HEADERVIEWMODEL_H

#include <QStandardItemModel>
#include "HierarchicalHeaderView.h"


class RotatedHeaderViewModel : public QStandardItemModel
{
	QStandardItemModel _horizontalHeaderModel;
	QStandardItemModel _verticalHeaderModel;

    void fillHHeaderModel(QStandardItemModel& headerModel, const QStringList &cols)
	{
        headerModel.insertColumns(0, cols.count());
        for (int i = 0; i < headerModel.columnCount(); ++i) {
            headerModel.setItem(0, i, new QStandardItem(cols.at(i)));
            headerModel.item(0, i)->setData(1, Qt::UserRole);
        }
	}

    void fillVHeaderModel(QStandardItemModel& headerModel, const QStringList &rows)
	{
        headerModel.insertColumns(0, rows.count());
        for (int i = 0; i < headerModel.columnCount(); ++i) {
            headerModel.setItem(0, i, new QStandardItem(rows.at(i)));
            //headerModel.item(0, i)->setData(1, Qt::UserRole);
        }
	}

public:

    RotatedHeaderViewModel(const QStringList &cols, const QStringList &rows, QObject* parent=0)
        : QStandardItemModel(rows.count(), cols.count(), parent)
	{
        fillHHeaderModel(_horizontalHeaderModel, cols);
        fillVHeaderModel(_verticalHeaderModel, rows);
	}

	QVariant data(const QModelIndex& index, int role) const
	{
        switch (role) {
        case HierarchicalHeaderView::HorizontalHeaderDataRole:
            return QVariant::fromValue((QObject*)&_horizontalHeaderModel);

        case HierarchicalHeaderView::VerticalHeaderDataRole:
            return QVariant::fromValue((QObject*)&_verticalHeaderModel);

        case Qt::DisplayRole:
        case Qt::BackgroundRole:
            if (index.isValid())
                return QStandardItemModel::data(index, role);
            break;

        case Qt::TextAlignmentRole:
            if (index.column())
                return Qt::AlignCenter;
            break;

        default:
            break;
        }

        return QVariant();
	}

/*	Qt::ItemFlags flags ( const QModelIndex & index ) const
	{
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	}*/

};

#endif // HEADERVIEWMODEL_H
