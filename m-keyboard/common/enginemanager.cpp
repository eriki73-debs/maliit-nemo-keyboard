/*
 * This file is part of meego-keyboard 
 *
 * Copyright (C) 2010-2011 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
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

 */

#include "mabstractinputmethodhost.h"
#include "enginemanager.h"
#include "abstractengine.h"
#include "enginedefault.h"
#include "abstractenginewidgethost.h"
#include "mimcorrectionhost.h"
#include "sharedhandlearea.h"
#include "mvirtualkeyboard.h"
#include "symbolview.h"
#include "keyevent.h"

#include "mkeyboardhost.h"
#include <mimenginefactory.h>
#include <MGConfItem>
#include <MSceneWindow>


#include <QDebug>

namespace
{
    const QString DefaultInputLanguage("default");
    const bool DefaultCorrectionSettingAcceptedWithSpaceOption = false;
    const QString CorrectionSettingWithSpace("/meegotouch/inputmethods/virtualkeyboard/correctwithspace");
    const QString InputMethodCorrectionEngineRoot("/meegotouch/inputmethods/correctionengine");
}

class EngineHandlerDefault : public EngineHandler
{
public:
    EngineHandlerDefault(MKeyboardHost &keyboardHost)
        : EngineHandler(keyboardHost),
          mKeyboardHost(keyboardHost),
          mEngineWidgetHost(new MImCorrectionHost(keyboardHost.sceneWindow, 0))
    {
    }

    virtual ~EngineHandlerDefault() {
        delete mEngineWidgetHost;
        mEngineWidgetHost = 0;
    }
    
    static QStringList supportedLanguages() {
        return QStringList();
    }

    //! \reimp
    virtual void activate()
    {
        connect(mEngineWidgetHost,
                SIGNAL(candidateClicked(const QString &, int)),
                &mKeyboardHost,
                SLOT(handleCandidateClicked(const QString &, int)),
                Qt::UniqueConnection);
        mEngineWidgetHost->finalizeOrientationChange();
    }

    virtual void deactivate()
    {
        disconnect(mEngineWidgetHost, 0,
                   &mKeyboardHost,    0);
    }

    virtual AbstractEngineWidgetHost *engineWidgetHost()
    {
        // return default error correction host
        return mEngineWidgetHost;
    }

    virtual bool cursorCanMoveInsidePreedit() const
    {
        return true;
    }

    virtual bool hasHwKeyboardIndicator() const
    {
        return true;
    }

    virtual bool hasErrorCorrection() const
    {
        return true;
    }

    virtual bool acceptPreeditInjection() const
    {
        return true;
    }

    virtual bool hasAutoCaps() const
    {
        return true;
    }

    virtual bool hasContext() const
    {
        return true;
    }

    virtual bool commitPreeditWhenInterrupted()
    {
        return true;
    }

    virtual bool correctionAcceptedWithSpaceEnabled() const
    {
        return MGConfItem(CorrectionSettingWithSpace).value(DefaultCorrectionSettingAcceptedWithSpaceOption).toBool();
    }

    virtual bool isComposingInputMethod() const
    {
        return false;
    }

    virtual bool supportTouchPointAccuracy() const
    {
        return true;
    }

    virtual bool commitWhenCandidateClicked() const
    {
        return true;
    }

    virtual void clearPreedit(bool commit)
    {
        if (!mKeyboardHost.preedit.isEmpty()) {
            if (commit) {
                // Commit current preedit
                mKeyboardHost.inputMethodHost()->sendCommitString(mKeyboardHost.preedit);
            } else {
                // Clear current preedit
                QList<MInputMethod::PreeditTextFormat> preeditFormats;
                MInputMethod::PreeditTextFormat preeditFormat(0, 0, MInputMethod::PreeditKeyPress);
                preeditFormats << preeditFormat;
                mKeyboardHost.inputMethodHost()->sendPreeditString("", preeditFormats);
            }
            mKeyboardHost.preedit.clear();
        }
    }

    virtual void editingInterrupted()
    {
        clearPreedit(commitPreeditWhenInterrupted());
    }

    virtual void resetHandler()
    {
    }

    virtual void preparePluginSwitching()
    {
    }

    virtual bool handleKeyPress(const KeyEvent &event)
    {
        Q_UNUSED(event);
        return false;
    }

    virtual bool handleKeyRelease(const KeyEvent &event)
    {
        Q_UNUSED(event);
        return false;
    }

    virtual bool handleKeyClick(const KeyEvent &event)
    {
        Q_UNUSED(event);
        return false;
    }

    //! \reimp_end

private:
    MKeyboardHost &mKeyboardHost;
    AbstractEngineWidgetHost *mEngineWidgetHost;
};

EngineManager *EngineManager::Instance = 0;

EngineManager::EngineManager(MKeyboardHost &keyboardHost)
    : QObject(),
      mKeyboardHost(keyboardHost),
      currentEngineHandler(0),
      currentEngine(0)
{
}

EngineManager::~EngineManager()
{
    qDeleteAll(handlerMap.values());
    qDeleteAll(engineMap.values());
}

void EngineManager::createInstance(MKeyboardHost &keyboardHost)
{
    Q_ASSERT(!Instance);
    if (!Instance) {
        Instance = new EngineManager(keyboardHost);
        Instance->init();
    }
}

void EngineManager::destroyInstance()
{
    Q_ASSERT(Instance);
    delete Instance;
    Instance = 0;
}

EngineManager &EngineManager::EngineManager::instance()
{
    Q_ASSERT(Instance);
    return *Instance;
}

EngineHandler *EngineManager::handler() const
{
    return currentEngineHandler;
}

MImEngineWordsInterface *EngineManager::engine() const
{
    if (currentEngine)
        return currentEngine->engine();
    else
        return 0;
}

void EngineManager::init()
{
    // initialize the language properties map
    handlerMap.insert(DefaultInputLanguage,
                                 new EngineHandlerDefault(mKeyboardHost));

    // initialize the engine map
    MGConfItem defaultEngineSetting(InputMethodCorrectionEngineRoot);
    if (!defaultEngineSetting.value().isNull()) {
        engineMap.insert(DefaultInputLanguage,
                         new EngineDefault(*(mKeyboardHost.inputMethodHost()),
                                             defaultEngineSetting.value().toString()));
    }
    mLanguage = DefaultInputLanguage;
}

QString EngineManager::activeLanguage() const
{
    return mLanguage;
}

void EngineManager::updateLanguage(const QString &lang)
{
    qDebug() << __PRETTY_FUNCTION__ << "- used language:" << lang;
    if (lang == mLanguage)
        return;

    const QString language = lang.contains("@") ? lang.split('@').first()
                                   : lang;

    AbstractEngine *matchedEngine = 0;
    MGConfItem settingEngineByLang(InputMethodCorrectionEngineRoot + "/" + language);
    if (!settingEngineByLang.value().isNull()) {
        // find and create engine if it not exists in the map.
        matchedEngine = findOrCreateEngine(language, settingEngineByLang.value().toString());
    } else {
        // use default engine, note: it could be also NULL
        matchedEngine = engineMap.value(DefaultInputLanguage).data();
    }

    if (currentEngine != matchedEngine 
        && currentEngine != 0
        && currentEngineHandler != 0
        && currentEngineHandler->engineWidgetHost()) {
        currentEngineHandler->engineWidgetHost()->hideEngineWidget();
    }
         
    if (currentEngineHandler) {
        if (currentEngineHandler->engineWidgetHost())
            currentEngineHandler->engineWidgetHost()->reset();

        currentEngineHandler->editingInterrupted();
    }

    currentEngine = matchedEngine;
    if (currentEngine) {
        currentEngine->updateEngineLanguage(lang);
    }

    EngineHandler *oldEngineHandler = currentEngineHandler;

    currentEngineHandler = findOrCreateEngineHandler(language);

    if(oldEngineHandler!= currentEngineHandler) {
        if (oldEngineHandler)
            oldEngineHandler->deactivate();

        if (currentEngineHandler)
            currentEngineHandler->activate();
    }

    mLanguage = lang;
}

EngineHandler *EngineManager::findOrCreateEngineHandler(const QString &language)
{
    qDebug() << __PRETTY_FUNCTION__ << "- used language:" << language;
    QMap<QString, QPointer<EngineHandler> >::iterator matchedIt = handlerMap.find(language);
    QPointer<EngineHandler> matchedEngineHandler = 0;

    if (matchedIt != handlerMap.end()) {
        // already in the map
        matchedEngineHandler = matchedIt.value().data();

    } else {
        // use default language properties
        matchedEngineHandler = handlerMap.value(DefaultInputLanguage).data();
    }

    return matchedEngineHandler;
}

AbstractEngine *EngineManager::findOrCreateEngine(const QString &language,
                                                                       const QString &engineName)
{
    qDebug() << __PRETTY_FUNCTION__ << "- used language:" << language << "," << engineName;
    QMap<QString, QPointer<AbstractEngine> >::iterator matchedIt = engineMap.find(language);
    QPointer<AbstractEngine> matchedEngine = 0;

    if (matchedIt != engineMap.end()) {
        // the engine for language is already in the map
        matchedEngine = matchedIt.value().data();

    } else {
        // use default engine
        matchedEngine = engineMap.value(DefaultInputLanguage).data();

    }

    return matchedEngine;
}