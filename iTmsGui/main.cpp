#include "iTmsGui.h"
#include "irdm.h"
#include <QtWidgets/QApplication>
#include <QTextCodec>
#include <QStyle> 
#include <QDesktopWidget> 

int main(int argc, char *argv[])
{
	QTextCodec *tc = QTextCodec::codecForName("GBK");
	//QTextCodec::setCodecForCStrings(tc);														//���������Ҫ�������ַ���������QByteArray����QString����ʱʹ�õ�Ĭ�ϱ��뷽ʽ
	QTextCodec::setCodecForLocale(tc);															//���������Ҫ�������úͶԱ����ļ�ϵͳ��дʱ���Ĭ�ϱ����ʽ������ͨ������ȡһ���ļ�ʱ����ʱ�ı����ʽ������ͨ��qDebug()�����ӡ��Ϣʱ�ı���
	//QTextCodec::setCodecForTr(tc);																//������������������ô���tr����ʱ��Ĭ���ִ�����

	QApplication a(argc, argv);
	a.setApplicationVersion(QObject::tr("v0.0.3"));

	iSys &oSys = iSys::Instance();

	iTmsGui w;
	w.show();
	w.resize(1200, 800);

	//Center Window on the Screen
	w.setGeometry(
		QStyle::alignedRect(
			Qt::LeftToRight,
			Qt::AlignCenter,
			w.size(),
			a.desktop()->availableGeometry()
		)
	);

	return a.exec();
}
