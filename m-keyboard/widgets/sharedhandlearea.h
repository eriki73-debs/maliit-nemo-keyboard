/* * This file is part of meego-keyboard *
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 * All rights reserved.
 * Contact: Nokia Corporation (directui@nokia.com)
 *
 * If you have questions regarding the use of this file, please contact
 * Nokia at directui@nokia.com.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * and appearing in the file LICENSE.LGPL included in the packaging
 * of this file.
 */

#ifndef SHAREDHANDLEAREA_H
#define SHAREDHANDLEAREA_H

#include <MWidget>
#include <MNamespace>

#include <QList>
#include <QPointer>

class QGraphicsLinearLayout;
class FlickGesture;
class MImToolbar;
class Handle;

/*!
  \brief SharedHandleArea represents the handle area shared between the vkb and the symbol
  view

  The purpose of shared handle area is to contain an invisible handle (non-zero size only
  in direct mode without close button) and the toolbar, in that order, and to stay on top
  of either vkb or the symbol view, whichever is in a higher position.  The invisible
  handle is currently disabled, though.
*/
class SharedHandleArea : public MWidget
{
    Q_OBJECT

public:
    /*!
     * \brief Constructor
     * \param toolbar toolbar widget
     * \param parent Parent object.
     */
    explicit SharedHandleArea(MImToolbar &toolbar, QGraphicsWidget *parent = 0);

    //! Destructor
    virtual ~SharedHandleArea();

    //! Set input method mode
    void setInputMethodMode(M::InputMethodMode mode);

    /*!
     * \brief Returns region including this object.
     * \param region Region occupied by other widgets.
     * \param includeExtraInteractiveAreas Result includes transparent interactive area
     * if this param is true.
     *
     * This method will return unchanged \a region is all watched widgets
     * are invisible. See also watchOnWidget.
     */
    QRegion addRegion(const QRegion &region,
                      bool includeExtraInteractiveAreas) const;

    /*!
     * \brief Ask handle area to watch on position and visibility of given \a widget.
     */
    void watchOnWidget(QGraphicsWidget *widget);

    //! Update position and geometry when orientation is changed
    void finalizeOrientationChange();

signals:
    void flickUp(const FlickGesture &gesture);
    void flickDown(const FlickGesture &gesture);
    void flickLeft(const FlickGesture &gesture);
    void flickRight(const FlickGesture &gesture);

    void regionUpdated();
    void inputMethodAreaUpdated();

private:
    /*!
     * This enumeration defines whether regionUpdated and inputMethodAreaUpdated
     * should be emitted
     */
    enum SignalsMode {
        SignalsAuto, //!< Signals are emitted when position is changed
        SignalsEnforce, //!< Signals will be emitted.
        SignalsBlock, //!< Signals are blocked.
    };

    //! Connect signals from a \a handle widget
    void connectHandle(const Handle &handle);

    //! Toggles visibility based on handleToolbarTypeChange and setInputMethodMode calls
    void updateInvisibleHandleVisibility();

private slots:
    //! Update widget position and notify about region update.
    //! \param sendSignals If this parameter contains true then signals regionUpdated and
    //! inputMethodAreaUpdated will be emitted even if position was not changed
    void updatePositionAndRegion(SignalsMode sendSignals = SignalsAuto);

    /*!
     * \brief Move toolbar when other widgets are moved.
     *
     * This slot DOES NOT emit regionUpdated and inputMethodAreaUpdated.
     */
    void updatePosition();

private:
    enum LayoutIndex {
        InvisibleHandleIndex,
        ToolbarIndex,
    };

    QGraphicsLinearLayout &mainLayout;

    //! Invisible gesture handle used only in direct mode
    Handle &invisibleHandle;

    //! Dummy widget we use in place of the invisible gesture handle when it's
    //! not ... err, visible (in its usual invisible way)
    QGraphicsWidget &zeroSizeInvisibleHandle;

    //! Widgets define position of SharedHandleArea: this object
    //! should be placed above other widgets.
    QList<QPointer<QGraphicsWidget> > watchedWidgets;

    MImToolbar &toolbar;

    M::InputMethodMode inputMethodMode;
};

#endif
