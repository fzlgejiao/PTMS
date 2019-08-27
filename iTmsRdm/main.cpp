#include <QtWidgets/QApplication>
#include <QTextCodec>
#include "iview.h"
#include "irdm.h"

int main(int argc, char *argv[])
{
	QTextCodec *tc = QTextCodec::codecForName("GBK");
	//QTextCodec::setCodecForCStrings(tc);														//这个函数主要用在用字符常量或者QByteArray构造QString对象时使用的默认编码方式
	QTextCodec::setCodecForLocale(tc);															//这个函数主要用于设置和对本地文件系统读写时候的默认编码格式。比如通过流读取一个文件时内容时的编码格式。或者通过qDebug()输出打印信息时的编码
	//QTextCodec::setCodecForTr(tc);																//这个函数的作用是设置传给tr函数时的默认字串编码

	QApplication a(argc, argv);
	a.setApplicationVersion("V0.0.5");
	a.setApplicationName("RDM - PTMS");

	iRDM oRDM;
	iView view(&oRDM);
	view.show();
	view.resize(800, 480);
	return a.exec();
}
