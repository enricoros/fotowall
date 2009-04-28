/*
    PosteRazor - Make your own poster!
    Copyright (C) 2005-2009 by Alessandro Portale
    http://posterazor.sourceforge.net/

    This file is part of PosteRazor

    PosteRazor is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    PosteRazor is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with PosteRazor; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
*/

#include "mainwindow.h"
#include "controller.h"
#if defined (FREEIMAGE_LIB)
#   include "imageloaderfreeimage.h"
#else
#   include "imageloaderqt.h"
#endif

#include <QtGui>

#if !defined(QT_SHARED) && !defined(QT_DLL) && !defined(FREEIMAGE_LIB)
Q_IMPORT_PLUGIN(qgif)
Q_IMPORT_PLUGIN(qjpeg)
Q_IMPORT_PLUGIN(qtiff)
#endif

int main (int argc, char **argv)
{
    QApplication a(argc, argv);

#if 0
    Q_ASSERT(argc == 3);
    PosteRazorCore posteRazorCore;
    QString loadErrorMessage;
    const bool loaded = posteRazorCore.loadInputImage(argv[1], loadErrorMessage);
    Q_ASSERT(loaded);
    if (loaded) {
        const int err = posteRazorCore.savePoster(argv[2]);
        Q_ASSERT(err == 0);
    }
    return 0;
#elif 0
    QImage image(512, 512, QImage::Format_ARGB32);
    image.fill(Qt::white);
    QRadialGradient gradient(image.rect().center(), image.width());
    gradient.setColorAt(0, Qt::black);
    gradient.setColorAt(1, Qt::red);
    QPainter p(&image);
    p.fillRect(image.rect(), gradient);

    ImageLoaderQt imageLoader;
    imageLoader.setQImage(image);
    PosteRazorCore posteRazorCore(&imageLoader);
    Wizard wizard;

    Controller controller(&posteRazorCore, &wizard);
    controller.setImageLoadingAvailable(false);
    wizard.show();
    return a.exec();
#else

    QCoreApplication::setApplicationName("PosteRazor");
#if QT_VERSION >= 0x040400
    QCoreApplication::setApplicationVersion("1.9.5");
#endif
    QCoreApplication::setOrganizationName("CasaPortale");
    QCoreApplication::setOrganizationDomain("de.casaportale");

    MainWindow dialog;
#if defined (FREEIMAGE_LIB)
    ImageLoaderFreeImage
#else
    ImageLoaderQt
#endif
        imageLoader;
    PosteRazorCore posteRazorCore(&imageLoader);
    Controller controller(&posteRazorCore, &dialog);

#ifdef Q_OS_WIN
    // PosteRazor 1 also used just an .ini file. This lets
    // PosteRazor easily become "portable app" (e.g. on USB stick)
    QSettings::setDefaultFormat(QSettings::IniFormat);
#endif
    QSettings settings;
    dialog.readSettings(&settings);
    controller.readSettings(&settings);

    dialog.show();
    if (argc == 2)
        controller.loadInputImage(argv[1]);

    const int appReturn = a.exec();
    dialog.writeSettings(&settings);
    controller.writeSettings(&settings);

    return appReturn;
#endif
}
