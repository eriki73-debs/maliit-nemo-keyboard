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

#include "renderer.h"
#include "keyareaitem.h"
#include "keyitem.h"
#include "graphicsview.h"
#include "models/keyarea.h"

#ifdef MALIIT_KEYBOARD_HAVE_GL
#include <QGLWidget>
#endif

namespace MaliitKeyboard {

namespace {

QGraphicsView * createView(QWidget *widget,
                           AbstractBackgroundBuffer *buffer)
{
    GraphicsView *view = new GraphicsView(widget);
    view->setBackgroundBuffer(buffer);
    view->resize(widget->size());
    view->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    view->setOptimizationFlags(QGraphicsView::DontClipPainter | QGraphicsView::DontSavePainterState);
    QGraphicsScene *scene = new QGraphicsScene(view);
    view->setScene(scene);
    view->setFrameShape(QFrame::NoFrame);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

#ifdef MALIIT_KEYBOARD_HAVE_GL
    view->setViewport(new QGLWidget);
#endif

    return view;
}

QGraphicsItem * rootItem(QGraphicsView *view)
{
    if (not view || not view->scene()) {
        return 0;
    }

    QList<QGraphicsItem *> items = view->scene()->items(Qt::DescendingOrder);
    if (not items.isEmpty()) {
        return items.at(0);
    }

    return 0;
}

} // namespace

class RendererPrivate
{
public:
    typedef QVector<KeyAreaItem *> Pool;

    QWidget *window;
    QScopedPointer<QGraphicsView> view;
    AbstractBackgroundBuffer *buffer;
    QRegion region;
    QVector<KeyItem *> key_items;
    QHash<SharedLayout, Pool> pools;

    explicit RendererPrivate()
        : window(0)
        , view(0)
        , buffer(0)
        , region()
        , key_items()
        , pools()
    {}
};

Renderer::Renderer(QObject *parent)
    : QObject(parent)
    , d_ptr(new RendererPrivate)
{}

Renderer::~Renderer()
{}

void Renderer::setWindow(QWidget *window)
{
    Q_D(Renderer);
    d->window = window;

    d->view.reset(createView(d->window, d->buffer));
    d->view->show();

    QGraphicsRectItem *root = new QGraphicsRectItem;
    root->setRect(d->view->rect());
    root->show();
    d->view->scene()->addItem(root);
}

void Renderer::setBackgroundBuffer(AbstractBackgroundBuffer *buffer)
{
    Q_D(Renderer);
    d->buffer = buffer;
}

QRegion Renderer::region() const
{
    Q_D(const Renderer);
    return d->region;
}

QWidget * Renderer::viewport() const
{
    Q_D(const Renderer);
    return d->view->viewport();
}

void Renderer::show(const SharedLayout &layout)
{
    Q_D(Renderer);

    if (layout.isNull() || not d->window || d->view.isNull()) {
        qCritical() << __PRETTY_FUNCTION__
                    << "Invalid layout or no main window specified,"
                    << "don't know where to render to.";
        return;
    }

    // Allows for multiple key areas being shown at the same time:
    // TODO: Animate fade-in, from nearest window border.
    KeyAreaItem *item = 0;
    const RendererPrivate::Pool &pool(d->pools.value(layout));
    if (pool.isEmpty()) {
        item = new KeyAreaItem(rootItem(d->view.data()));
        RendererPrivate::Pool p(Layout::NumPanels, 0);
        p.replace(layout->activePanel(), item);
        d->pools.insert(layout, p);
    } else if (KeyAreaItem *found = pool.at(layout->activePanel())) {
        item = found;
    }

    // TODO: Construct region from polygon path so that split key areas dont
    // consume space in between them.
    d->region -= QRegion(item->boundingRect().toRect());
    item->setKeyArea(layout->activeKeyArea());

    d->region |= QRegion(item->boundingRect().toRect());
    item->show();
}

void Renderer::hide(const SharedLayout &layout)
{
    if (layout.isNull()) {
        qCritical() << __PRETTY_FUNCTION__
                    << "Cannot hide non-existant layout.";
        return;
    }

    Q_D(Renderer);

    // TODO: Animate fade-out, towards nearest window border.
    const RendererPrivate::Pool &pool(d->pools.value(layout));
    if (pool.isEmpty()) {
        return;
    } else if (KeyAreaItem *found = pool.at(Layout::CenterPanel)) {
        d->region -= QRegion(found->boundingRect().toRect());
        found->hide();
    }
}

void Renderer::hideAll()
{
    Q_D(Renderer);

    d->region = QRegion();
    foreach (RendererPrivate::Pool p, d->pools) {
        for (int index = 0; index < p.count(); ++index) {
            if (KeyAreaItem *found = p.at(index)) {
                found->hide();
            }
        }
    }
}

void Renderer::onLayoutChanged(const SharedLayout &layout)
{
    show(layout);
}

void Renderer::onKeysChanged(const SharedLayout &layout)
{
    if (layout.isNull()) {
        qCritical() << __PRETTY_FUNCTION__
                    << "Invalid layout.";
        return;
     }

    Q_D(Renderer);

    const RendererPrivate::Pool &pool(d->pools.value(layout));
    if (pool.isEmpty()) {
        qCritical() << __PRETTY_FUNCTION__
                    << "Unknown layout. Forgot to show it?";
        return;
    }

    if (KeyAreaItem *const ka_item = pool.at(layout->activePanel())) {
        const QVector<Key> &active_keys(layout->activeKeys());
        int index = 0;

        for (; index < active_keys.count(); ++index) {
            KeyItem *item = 0;

            if (index >= d->key_items.count()) {
                item = new KeyItem;
                d->key_items.append(item);
            } else {
                item = d->key_items.at(index);
            }

            item->setParentItem(ka_item);
            item->setKey(active_keys.at(index));
            item->show();
        }

        if (layout->magnifierKey().valid()) {
            KeyItem *magnifier = 0;

            if (index >= d->key_items.count()) {
                magnifier = new KeyItem;
                d->key_items.append(magnifier);
            } else {
                magnifier = d->key_items.at(index);
            }

            magnifier->setParentItem(ka_item);
            magnifier->setKey(layout->magnifierKey());
            magnifier->show();
            ++index;
        }

        // Hide remaining, currently unneeded key items:
        for (; index < d->key_items.count(); ++index) {
            d->key_items.at(index)->hide();
        }
    }
}

} // namespace MaliitKeyboard
