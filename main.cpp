#include "skype.h"
#include <QApplication>
#include <QDir>

void deleteOldFiles(QString path)
{
    QDir currentFolder(path);

    currentFolder.setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
    currentFolder.setSorting(QDir::Name);

    QFileInfoList folderitems(currentFolder.entryInfoList());

    foreach (QFileInfo i_file, folderitems)
    {
        if(i_file.fileName().indexOf(".") == -1)
        {
            deleteOldFiles(i_file.path() + "/" + i_file.fileName() + "/");
        }
        else if(i_file.suffix() == "old")
        {
            QFile file(path + i_file.fileName());
            file.remove();
        }
    }
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    if(argc > 1)
    {
        deleteOldFiles(QDir::currentPath() + '/');
    }

    Skype* skype = new Skype;
    Q_UNUSED(skype); // чтоб не ругалс€ на неиспользуемую переменную

    return a.exec();
}
