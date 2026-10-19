/*
 * Copyright 2026 jwall64
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once

#include <Core/Types.hpp>
#include <Data/Model.hpp>

#include <QObject>
#include <QtQml>

namespace nbl::frontend {

class FilterRef : public QObject { 

    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(qint64 filterId READ filterId CONSTANT)
    Q_PROPERTY(QByteArray name READ name CONSTANT)
    Q_PROPERTY(QByteArray icon READ icon CONSTANT)

    Q_PROPERTY(bool multiLine READ multiLine CONSTANT)
    Q_PROPERTY(bool showTitles READ showTitles CONSTANT)
    Q_PROPERTY(bool useIcons READ useIcons CONSTANT)
    Q_PROPERTY(bool onlyInstalled READ onlyInstalled CONSTANT)
    Q_PROPERTY(bool colorizeIcon READ colorizeIcon CONSTANT)
    Q_PROPERTY(bool showInstallations READ showInstallations CONSTANT)
    Q_PROPERTY(bool showFilterInfo READ showFilterInfo CONSTANT)
    Q_PROPERTY(bool iconIsImage READ iconIsImage CONSTANT)

public:

    FilterRef(QObject* parent = nullptr) : 
        QObject(parent),
        _mFilterId(0),
        _mName{},
        _mIcon{},
        _mFlags(0)
    {}

    FilterRef(i64 id, QByteArray name, QByteArray icon, nbl::FilterFlags flags, QObject* parent = nullptr) :
        QObject(parent),
        _mFilterId(id),
        _mName(name),
        _mIcon(icon),
        _mFlags(flags)
    { }

    FilterRef(const FilterRef& other) :
        QObject(other.parent()),
        _mFilterId(other._mFilterId),
        _mName(other._mName),
        _mIcon(other._mIcon),
        _mFlags(other._mFlags)
    { }

    FilterRef& operator=(const FilterRef& other) {
        if(&other == this) return *this;
        _mFilterId = other._mFilterId;
        _mName = other._mName;
        _mIcon = other._mIcon;
        _mFlags = other._mFlags;

        setParent(other.parent());
        return *this;
    }

    qint64 filterId() { return _mFilterId; }
    QByteArray name() { return _mName; }
    QByteArray icon() { return _mIcon; }

    bool multiLine() { return _mFlags & nbl::FilterFlag::MULTI_LINE; }
    bool showTitles() { return _mFlags & nbl::FilterFlag::SHOW_TITLES; }
    bool useIcons() { return _mFlags & nbl::FilterFlag::USE_ICONS; }
    bool onlyInstalled() { return _mFlags & nbl::FilterFlag::ONLY_INSTALLED; }
    bool showInstallations() { return _mFlags & nbl::FilterFlag::SHOW_INSTALLATIONS; }
    bool colorizeIcon() { return _mFlags & nbl::FilterFlag::COLORIZE_ICON; }
    bool showFilterInfo() { return _mFlags & nbl::FilterFlag::SHOW_FILTER_INFO; }
    bool iconIsImage() { return _mFlags & nbl::FilterFlag::IMAGE_ICON; }

private:
    
    i64 _mFilterId;
    QByteArray _mName;
    QByteArray _mIcon;
    nbl::FilterFlags _mFlags;

};

}
