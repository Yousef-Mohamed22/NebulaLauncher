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

/**
 * \file TagFunctions.cpp
 * \author jwall64
 * \brief Partial implementation of ApplicationContext.hpp, containing functions related to managing tags
 */

#include "ApplicationContext.hpp"
#include <Data/TagModel.hpp>

qint64 ApplicationContext::addTag(QString name, QColor color) {
    QByteArray arr = name.toUtf8();
    nbl::StringView nblName = nbl::StringView(arr.data(), arr.length());
    i64 id = 0;
    if(nbl::Tag::add(databaseConnection(), nblName, color.rgba(), &id).error()) {
        return 0;
    }
    return static_cast<qint64>(id);
}

qint64 ApplicationContext::findTagId(QString name) {

    qint64 out;
    
    QByteArray utf = name.toUtf8();
    nbl::Result rc = nbl::Tag::find(databaseConnection(), nbl::StringView(utf.data(), utf.length()), [&](const nbl::Tag& meta) {
        out = meta.id;
        return nbl::Result::SUCCESS;
    });

    if(rc == nbl::Result::NOT_FOUND) return 0;
    return out;
}

int ApplicationContext::removeTag(qint64 tag) {
    return nbl::Tag::remove(databaseConnection(), tag);
}

int ApplicationContext::renameTag(qint64 tag, QStringView newName) {
    QByteArray newArr = newName.toUtf8();
    nbl::StringView nblNewName = nbl::StringView(newArr.data(), newArr.length());
    return nbl::Tag::rename(databaseConnection(), tag, nblNewName);
}

int ApplicationContext::recolorTag(qint64 tag, QColor color) {
    return nbl::Tag::setColor(databaseConnection(), tag, color.rgba());
}

int ApplicationContext::addTagParent(qint64 tag, qint64 parent) {
    return nbl::Tag::addParent(databaseConnection(), parent, tag);
}

int ApplicationContext::tagGame(qint64 gameId, qint64 tag) {
    return nbl::Tag::tagGame(databaseConnection(), gameId, tag);
}

int ApplicationContext::untagGame(qint64 gameId, qint64 tag) {
    return nbl::Tag::untagGame(databaseConnection(), gameId, tag);
}

int ApplicationContext::tagInstallation(qint64 installationId, qint64 tag) {
    return nbl::Tag::tagInstallation(databaseConnection(), installationId, tag);
}

int ApplicationContext::untagInstallation(qint64 installationId, qint64 tag) {
    return nbl::Tag::untagInstallation(databaseConnection(), installationId, tag);
}

int ApplicationContext::updateTags(QVariantList modifiedTags) {
    for(QVariant tag : modifiedTags) {

        QVariantMap map = tag.toMap();

        qint64 id = map["id"].toLongLong();

        QString originalName = map["originalName"].toString();
        QString name = map["name"].toString();
        QColor color = map["color"].value<QColor>();
        QVariantList newParents = map["addedParents"].toList();
        QVariantList removedParents = map["removedParents"].toList();

        NBL_CHECK(databaseConnection()->beginTransaction());

        if(originalName != name) {
            renameTag(id, name);
        }

        recolorTag(id, color);

        for(QVariant parent : newParents) {
            nbl::Tag::addParent(databaseConnection(), parent.toLongLong(), id);
        }
        for(QVariant parent : removedParents) {
            nbl::Tag::removeParent(databaseConnection(), parent.toLongLong(), id);
        }

        NBL_CHECK(databaseConnection()->commitTransaction());
    }

    return 0;
}


QVariantList ApplicationContext::getTagsFromIds(QVariantList ids) {
    QList<QVariant> out = {};

    QByteArray namesJson = QJsonDocument(QJsonArray::fromVariantList(ids)).toJson(QJsonDocument::JsonFormat::Compact);

    nbl::Tag::lookupByIds(databaseConnection(), nbl::StringView(namesJson.data(), namesJson.length()), [&](const nbl::Tag& meta) {
        QMap<QString, QVariant> tag = {};
        tag["id"] = static_cast<qint64>(meta.id);
        tag["name"] = nbl::frontend::fromNbl(meta.name);
        tag["color"] = QColor(meta.color);

        out.append(tag);
        return nbl::Result::SUCCESS;
    });
    return out;
}

QList<QVariant> ApplicationContext::getGameTags(qint64 gameId) {
    QList<QVariant> out = {};
    nbl::Tag::enumerateGameTags(databaseConnection(), gameId, [&](const nbl::Tag& meta) {
        QMap<QString, QVariant> tag = {};
        tag["id"] = static_cast<qint64>(meta.id);
        tag["name"] = nbl::frontend::fromNbl(meta.name);
        tag["color"] = QColor(meta.color);

        out.append(tag);
        return nbl::Result::SUCCESS;
    });
    return out;
}

QList<QVariant> ApplicationContext::getInstallationTags(qint64 installationId) {
    QList<QVariant> out = {};
    nbl::Tag::enumerateDirectInstallationTags(databaseConnection(), installationId, [&](const nbl::Tag& meta) {
        QMap<QString, QVariant> tag = {};
        tag["id"] = static_cast<qint64>(meta.id);
        tag["name"] = nbl::frontend::fromNbl(meta.name);
        tag["color"] = QColor(meta.color);

        out.append(tag);
        return nbl::Result::SUCCESS;
    });

    return out;
}

QList<QVariant> ApplicationContext::getTagParents(qint64 tag) {
    QList<QVariant> out = {};

    nbl::Tag::parents(databaseConnection(), tag, [&](const nbl::Tag& meta) {
        QMap<QString, QVariant> tag = {};
        tag["id"] = static_cast<qint64>(meta.id);
        tag["name"] = nbl::frontend::fromNbl(meta.name);
        tag["color"] = QColor(meta.color);

        out.append(tag);
        return nbl::Result::SUCCESS;
    });

    return out;
}


