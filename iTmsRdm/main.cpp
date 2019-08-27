#include <QtWidgets/QApplication>
#include <QTextCodec>
#include "iview.h"
#include "irdm.h"

int main(int argc, char *argv[])
{
	QTextCodec *tc = QTextCodec::codecForName("GBK");
	//QTextCodec::setCodecForCStrings(tc);														//���������Ҫ�������ַ���������QByteArray����QString����ʱʹ�õ�Ĭ�ϱ��뷽ʽ
	QTextCodec::setCodecForLocale(tc);															//���������Ҫ�������úͶԱ����ļ�ϵͳ��дʱ���Ĭ�ϱ����ʽ������ͨ������ȡһ���ļ�ʱ����ʱ�ı����ʽ������ͨ��qDebug()�����ӡ��Ϣʱ�ı���
	//QTextCodec::setCodecForTr(tc);																//������������������ô���tr����ʱ��Ĭ���ִ�����

	QApplication a(argc, argv);
	a.setApplicationVersion("V0.0.5");
	a.setApplicationName("RDM - PTMS");

	iRDM oRDM;
	iView view(&oRDM);
	view.show();
	view.resize(800, 480);
	return a.exec();
}
