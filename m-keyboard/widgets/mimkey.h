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



#ifndef SINGLEWIDGETBUTTON_H
#define SINGLEWIDGETBUTTON_H

#include "mimabstractkey.h"

class MImAbstractKeyAreaStyleContainer;
class QGraphicsItem;

//! Represents a key model with the key's current binding state, and also contains its visible area.
class MImKey
    : public MImAbstractKey
{
public:
    explicit MImKey(const MImKeyModel &mModel,
                    const MImAbstractKeyAreaStyleContainer &style,
                    QGraphicsItem &parent);

    virtual ~MImKey();

    //! \reimp
    virtual const QString label() const;
    virtual const QString secondaryLabel() const;
    virtual const QRectF &buttonRect() const;
    virtual const QRectF &buttonBoundingRect() const;
    virtual void setModifiers(bool shift, QChar accent = QChar());
    virtual void setDownState(bool down);
    virtual void setSelected(bool select);
    virtual ButtonState state() const;
    virtual const MImKeyModel &model() const;
    virtual const MImKeyBinding &binding() const;
    virtual bool isDeadKey() const;
    virtual bool isShiftKey() const;
    virtual bool isNormalKey() const;
    virtual bool increaseTouchPointCount();
    virtual bool decreaseTouchPointCount();
    virtual void resetTouchPointCount();
    virtual int touchPointCount() const;
    //! \reimp_end

    //! Return limit for active touchpoints
    static int touchPointLimit();

    //! \brief Returns the icon of this button, if it has one.
    const QPixmap *icon() const;

    //! \brief Returns icon identifier, if it was loaded.
    QString iconId() const;

    //! \brief Draws the icon of this button, if it has one, to the given rectangle.
    void drawIcon(const QRect &rectangle, QPainter *painter) const;

    //! \brief Calls parent item's QGraphicsItem::update() who actually draws the button.
    void update();

    //! Returns preferred fixed witdth
    int preferredFixedWidth() const;

    //! Returns preferred dynamic width
    qreal preferredWidth(qreal pixelPerSizeUnit, qreal spacing) const;

    //! Cache for the buttons position and size. They can always
    //! be calculated but are faster to access this way.
    QRectF cachedBoundingRect;
    QRectF cachedButtonRect;

    //! The width for this button. Not managed by this class.
    //! It is used by MImKeyArea to store the correct button size.
    qreal width;

private:
    //! Contains information about icon
    struct IconInfo
    {
        //! Actual image
        const QPixmap *pixmap;
        //! Icon identified
        QString id;

        IconInfo();
        ~IconInfo();
    };

    void loadIcon(bool shift);
    const IconInfo &iconInfo() const;

    const MImKeyModel &mModel;

    bool shift;
    QChar accent;

    QString currentLabel;
    ButtonState currentState;
    bool selected;

    IconInfo lowerCaseIcon;
    IconInfo upperCaseIcon;

    const MImAbstractKeyAreaStyleContainer &styleContainer;

    QGraphicsItem &parentItem;

    //! Touchpoint count
    int currentTouchPointCount;
};

#endif // SINGLEWIDGETBUTTON_H

