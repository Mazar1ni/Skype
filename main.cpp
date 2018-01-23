#include "skype.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Skype* skype = new Skype;
    Q_UNUSED(skype); // чтоб не ругалс€ на неиспользуемую переменную

    return a.exec();
}
