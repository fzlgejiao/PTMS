#include "iTmsGui.h"
#include <QtWidgets/QApplication>
#include <QTextCodec>

int main(int argc, char *argv[])
{
	QTextCodec *tc = QTextCodec::codecForName("GBK");
	//QTextCodec::setCodecForCStrings(tc);														//���������Ҫ�������ַ���������QByteArray����QString����ʱʹ�õ�Ĭ�ϱ��뷽ʽ
	QTextCodec::setCodecForLocale(tc);															//���������Ҫ�������úͶԱ����ļ�ϵͳ��дʱ���Ĭ�ϱ����ʽ������ͨ������ȡһ���ļ�ʱ����ʱ�ı����ʽ������ͨ��qDebug()�����ӡ��Ϣʱ�ı���
	//QTextCodec::setCodecForTr(tc);																//������������������ô���tr����ʱ��Ĭ���ִ�����

	QApplication a(argc, argv);
	a.setApplicationVersion(QObject::tr("X00.00.01"));

	iTmsGui w;
	w.show();
	w.resize(1200, 800);
	return a.exec();
}
