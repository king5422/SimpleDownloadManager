#include "mainwindow.h"
#include "ui_inputdialog.h"
#include <QSplitter> 
#include <QItemDelegate> 
#include <QPainter>
#include <QMessageBox> 
#include <QDebug>
#include <QDesktopServices> 

class TableDelegate : public QItemDelegate {
	public:
		explicit TableDelegate(QObject* parent=0):QItemDelegate(parent) {}

		void paint(QPainter* painter, const QStyleOptionViewItem& option,
			const QModelIndex& index) const override {
			
			QStyleOptionViewItem option2 = option;
			option2.state = option.state & (~QStyle::State_HasFocus);

			if(index.column() != 2) {
				QItemDelegate::paint(painter, option2, index);
				return;
			}

			//Draw ProgressBar
			QStyleOptionProgressBar progressbar;
			progressbar.state = option2.state;
			progressbar.rect = option2.rect;
			progressbar.minimum = 0;
			progressbar.maximum = 100;
			progressbar.textAlignment = Qt::AlignVCenter|Qt::AlignRight;
			progressbar.textVisible = true;
			progressbar.palette = option2.palette;

			//highlight on slection
			if(progressbar.state & QStyle::State_Selected)
				painter->fillRect(progressbar.rect, option2.palette.highlight());
			

			//set progressbar value & text
			progressbar.progress = index.data().toInt();
			progressbar.text = index.data().toString()+"%";

			//draw
			qApp->style()->drawControl(QStyle::CE_ProgressBar, 
				&progressbar, painter);
		}
};

class InputDialog : public QDialog {

public:
	explicit InputDialog(QString title, QString label, QWidget* parent = 0):
		QDialog(parent), ui(new  Ui::Dialog) {
		
		ui->setupUi(this);
		this->setWindowTitle(title);
		this->setModal(true);
		ui->label->setText(label);
	}

	QString getVal() {
		return ui->lineEdit->text();
	}

private:
	Ui::Dialog* ui;

};

MainWindow::MainWindow(QWidget* parent):
	QMainWindow(parent), ui(new Ui::MainWindow) {
	
	QResource::registerResource("resource.rcc");
	ui->setupUi(this);


	model = new DownloadModel(this); 
	TableDelegate *delegate = new TableDelegate(this);
 
	ui->tableView->setModel(model);
	ui->tableView->setItemDelegate(delegate);
	ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui->tableView->horizontalHeader()->setHighlightSections(false); 
	ui->tableView->verticalHeader()->setVisible(false);
	ui->tableView->setShowGrid(false);
	ui->tableView->setSortingEnabled(true);
	ui->tableView->resizeColumnToContents(0);

	connect(ui->actionNew, &QAction::triggered, this, &MainWindow::newDownload);
}          

void MainWindow::newDownload() {

		QUrl url;
		InputDialog* dialog = new InputDialog("New Download", 
			"Enter Download URL: ", this);
		
		dialog->show();
		QEventLoop loop;
		connect(dialog, &QDialog::accepted, this, [&]{
			url = dialog->getVal();
			loop.exit();
		});
		loop.exec();

		//add new download to model
		Download* download = new Download(model->rowCount(), url, 1, model);

		connect(download, &Download::downloadInfoComplete, this, 
			[=](QString message, bool error){
			if(error) {
				QMessageBox::warning(this, "Error!", message);
				download->deleteLater();
				return;
			} 
			
			model->addDownload(download);

			//updating progressbar
			connect(download, &Download::progressUpdate, this, [=]{
				updateModel(download, DownloadModel::PROGRESS);
			});
			//update size if updateDownloadInfo fails
			connect(download, &Download::fileSizeUpdate, this, [=]{
				updateModel(download, DownloadModel::SIZE);
			});
			 
		});

		download->updateDownloadInfo(); 
		download->startDownload();

		ui->tableView->resizeColumnToContents(1);
		ui->tableView->resizeColumnToContents(2);
		ui->tableView->resizeColumnToContents(3);
}
 
bool MainWindow::updateModel(Download* download, 
	DownloadModel::UPDATE_FIELD field) const { 
	
	int row = model->getDownloadIndex(download);
	QModelIndex index = model->index(row, 0);
	model->setData(index, QVariant(field));
} 