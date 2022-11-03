
#include <QApplication>
#include "pub_logfile.h"
#include "dlg_mainwindow.h"
#include "filepath.h"

std::string GetProgramDir()
{
	char exeFullPath[MAX_PATH]; // Full path
	std::string strPath = "";

	GetModuleFileName(NULL, exeFullPath, MAX_PATH);
	strPath = (std::string)exeFullPath;    // Get full path of the file
	int pos = strPath.find_last_of('\\', strPath.length());
	return strPath.substr(0, pos);  // Return the directory without the file name
}

int main(int argc, char** argv)
{
	int ret = 0;

#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

	QApplication app(argc,argv);
	QTextCodec *locale_codec = QTextCodec::codecForLocale();
	if (NULL != locale_codec)
	{

		QTextCodec::setCodecForLocale(locale_codec);
		std::string str = GetProgramDir();
		CFilePath::Instance().SetEnvPath(str);
		std::string str2 = CFilePath::Instance().GetEnvPath();
		str2 = str2 + "\\log\\megsky_agc.log";
		CLogFile::Instance().SetFileName(str2, 1024 * 1024 * 1000);//²âÊÔÈÕÖ¾

		app.connect(&app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()));
		CLogFile::Instance().StartThread();


		CMainWndDlg* dlg = new CMainWndDlg;
		dlg->showFullScreen();
		ret = app.exec();

		CLogFile::Instance().StopThread();
		delete dlg;
		dlg = NULL;

	}
	return ret;
}