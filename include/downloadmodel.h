#pragma once

#include "download.h"
#include <QAbstractTableModel>
#include <QModelIndex>

//Custom Model
class DownloadModel : public QAbstractTableModel {

public:
	static constexpr int COL_CNT = 7;
	static const QString HEADERS[COL_CNT];
	static const QString DEFAULT_TEXT;

	enum UPDATE_FIELD {
		PROGRESS = 0x0000,
		SPEED = 0x0001,
		SIZE = 0x0002,
		STATE = 0x003,
		ETA = 0x004,
		ALL = 0x0005
	};

	DownloadModel(QObject* parent = 0);

	//overriding virtual methods from QAbstractDownloadModel
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	int columnCount(const QModelIndex& parent = QModelIndex()) const override;
	QVariant data(const QModelIndex& index,
		int role = Qt::DisplayRole) const override;
	QVariant headerData(int section, Qt::Orientation orienttation,
		int role = Qt::DisplayRole) const override;
	bool setData(const QModelIndex& index, const QVariant& value,
		int role = Qt::EditRole) override;
	bool insertRows(int row, int count,
		const QModelIndex& parent = QModelIndex()) override;

	bool removeRows(int row, int count,
		const QModelIndex &parent = QModelIndex()) override;

	//replaces download in the last row
	void replaceDownloadLRow(Download* download);

	Download* getDownload(int i) const {
		if(i < downloads.size()) return downloads.at(i);
		return nullptr;
	}

	int getDownloadIndex(Download* download) const {
		int index = downloads.indexOf(download);
	}

	//Method to directly add a new download
	void addNewDownload(Download* download) {
		insertRow(downloadCount);
		replaceDownloadLRow(download);
	}

private:
	QList<Download*> downloads;
	int downloadCount; //row count
};