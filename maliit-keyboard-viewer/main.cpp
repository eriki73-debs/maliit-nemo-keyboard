/*
 * This file is part of Maliit Plugins
 *
 * Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: Mohammad Anwari <Mohammad.Anwari@nokia.com>
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this list
 * of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 * Neither the name of Nokia Corporation nor the names of its contributors may be
 * used to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "renderer/renderer.h"
#include "glass/glass.h"
#include "logic/layoutupdater.h"

#include "models/keyarea.h"
#include "models/key.h"
#include "models/keylabel.h"
#include "models/layout.h"

#include <QtGui>

namespace {
MaliitKeyboard::Key createKey(const QPixmap &pm,
                              const MaliitKeyboard::SharedFont &f,
                              const QRect &kr,
                              const QRect &lr,
                              const QByteArray &t,
                              const QColor &c)
{
    MaliitKeyboard::KeyLabel l;
    l.setRect(lr);
    l.setText(t);
    l.setColor(c);
    l.setFont(f);

    MaliitKeyboard::Key k;
    k.setRect(kr);
    k.setBackground(pm);
    k.setLabel(l);

    return k;
}

MaliitKeyboard::KeyArea createPrimaryKeyArea()
{
    typedef QByteArray QBA;

    QPixmap pm(8, 8);
    pm.fill(Qt::lightGray);

    MaliitKeyboard::SharedFont font(new QFont);
    font->setBold(true);
    font->setPointSize(16);

    MaliitKeyboard::KeyArea ka;
    ka.setRect(QRectF(0, 554, 480, 300));
    ka.appendKey(createKey(pm, font, QRect(10, 10, 40, 60),
                           QRect(5, 5, 20, 40), QBA("Q"), Qt::darkBlue));
    ka.appendKey(createKey(pm, font, QRect(60, 10, 80, 120),
                           QRect(5, 5, 20, 40), QBA("W"), Qt::darkMagenta));
    ka.appendKey(createKey(pm, font, QRect(10, 80, 40, 50),
                           QRect(5, 5, 20, 40), QBA("A"), Qt::black));
    ka.appendKey(createKey(pm, font, QRect(10, 140, 130, 60),
                           QRect(5, 5, 20, 40), QBA("X"), Qt::darkCyan));

    return ka;
}

MaliitKeyboard::KeyArea createSecondaryKeyArea()
{
    typedef QByteArray QBA;

    QPixmap pm(8, 8);
    pm.fill(Qt::lightGray);

    MaliitKeyboard::SharedFont font(new QFont);
    font->setBold(true);
    font->setPointSize(16);

    MaliitKeyboard::KeyArea ka;
    ka.setRect(QRectF(0, 0, 480, 100));
    ka.appendKey(createKey(pm, font, QRect(10, 10, 40, 60),
                           QRect(5, 5, 20, 40), QBA("T"), Qt::darkBlue));
    ka.appendKey(createKey(pm, font, QRect(60, 10, 80, 80),
                           QRect(5, 5, 20, 40), QBA("O"), Qt::darkMagenta));

    return ka;
}


}

int main(int argc,
         char ** argv)
{
    QApplication app(argc, argv);

    QWidget *window = new QWidget;
    window->resize(480, 854);
    window->showFullScreen();

    MaliitKeyboard::SharedLayout l0(new MaliitKeyboard::Layout);
    l0->setCenterPanel(createPrimaryKeyArea());

    MaliitKeyboard::SharedLayout l1(new MaliitKeyboard::Layout);
    l1->setCenterPanel(createSecondaryKeyArea());

    MaliitKeyboard::Renderer renderer;
    renderer.setWindow(window);
    renderer.show(l0);
    renderer.show(l1);

    MaliitKeyboard::Glass glass;
    glass.setWindow(renderer.viewport());
    glass.activate(l0);
    glass.activate(l1);

    MaliitKeyboard::LayoutUpdater updater;
    updater.setLayout(l0);

    QObject::connect(&glass,    SIGNAL(activeKeysChanged(SharedLayout,Layout::Panel)),
                     &renderer, SLOT(onActiveKeysChanged(SharedLayout,Layout::Panel)));

    return app.exec();
}
