/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright 2012 The MITRE Corporation                                      *
 *                                                                           *
 * Licensed under the Apache License, Version 2.0 (the "License");           *
 * you may not use this file except in compliance with the License.          *
 * You may obtain a copy of the License at                                   *
 *                                                                           *
 *     http://www.apache.org/licenses/LICENSE-2.0                            *
 *                                                                           *
 * Unless required by applicable law or agreed to in writing, software       *
 * distributed under the License is distributed on an "AS IS" BASIS,         *
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  *
 * See the License for the specific language governing permissions and       *
 * limitations under the License.                                            *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <QCryptographicHash>
#include <QDebug>
#ifndef BR_EMBEDDED
#include <QDesktopServices>
#endif // BR_EMBEDDED
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QProcessEnvironment>
#include <QRegExp>
#include <QRegularExpression>
#include <QStack>
#include <QUrl>
#include <openbr/openbr_plugin.h>

#include "alphanum.hpp"
#include "qtutils.h"

using namespace br;

namespace QtUtils
{

QStringList getFiles(QDir dir, bool recursive)
{
    dir = QDir(dir.canonicalPath());

    QStringList files;
    foreach (const QString &file, naturalSort(dir.entryList(QDir::Files)))
        files.append(dir.absoluteFilePath(file));

    if (!recursive) return files;

    foreach (const QString &folder, naturalSort(dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot))) {
        QDir subdir(dir);
        bool success = subdir.cd(folder); if (!success) qFatal("cd failure.");
        files.append(getFiles(subdir, true));
    }
    return files;
}

QStringList getFiles(const QString &regexp)
{
    QFileInfo fileInfo(regexp);
    QDir dir(fileInfo.dir());
    QRegExp re(fileInfo.fileName());
    re.setPatternSyntax(QRegExp::Wildcard);

    QStringList files;
    foreach (const QString &fileName, dir.entryList(QDir::Files))
        if (re.exactMatch(fileName))
            files.append(dir.filePath(fileName));
    return files;
}

QStringList readLines(const QString &file)
{
    QStringList lines;
    readFile(file, lines);
    return lines;
}

void readFile(const QString &file, QStringList &lines)
{
    QByteArray data;
    readFile(file, data);
    lines = QString(data).split(QRegularExpression("[\n|\r\n|\r]"), QString::SkipEmptyParts);
    for (int i=0; i<lines.size(); i++)
        lines[i] = lines[i].simplified();
}

void readFile(const QString &file, QByteArray &data, bool uncompress)
{
    QFile f(file);
    if (!f.open(QFile::ReadOnly)) {
        if (f.exists()) qFatal("Unable to open %s for reading. Check file permissions.", qPrintable(file));
        else            qFatal("Unable to open %s for reading. File does not exist.", qPrintable(file));
    }
    data = f.readAll();
    if (uncompress) data = qUncompress(data);
    f.close();
}

void writeFile(const QString &file, const QStringList &lines)
{
    if (file.isEmpty()) return;
    const QString baseName = QFileInfo(file).baseName();

    if (baseName == "terminal") {
        printf("%s\n", qPrintable(lines.join("\n")));
    } else if (baseName == "buffer") {
        Globals->buffer = lines.join("\n").toStdString().c_str();
    } else {
        QFile f(file);
        touchDir(f);

        if (!f.open(QFile::WriteOnly))
            qFatal("Failed to open %s for writing.", qPrintable(file));

        foreach (const QString &line, lines)
            f.write((line+"\n").toLocal8Bit());

        f.close();
    }
}

void writeFile(const QString &file, const QString &data)
{
    writeFile(file, data.toLocal8Bit());
}

void writeFile(const QString &file, const QByteArray &data, int compression)
{
    if (file.isEmpty()) return;
    const QString baseName = QFileInfo(file).baseName();
    const QByteArray contents = (compression == 0) ? data : qCompress(data, compression);
    if (baseName == "terminal") {
        printf("%s\n", qPrintable(contents));
    } else if (baseName == "buffer") {
        Globals->buffer = data;
    } else {
        QFile f(file);
        touchDir(f);
        if (!f.open(QFile::WriteOnly))
            qFatal("Failed to open %s for writing.", qPrintable(file));
        f.write(contents);
        f.close();
    }
}

void copyFile(const QString &src, const QString &dst)
{
    touchDir(QFileInfo(dst));
    if (!QFile::copy(src, dst)) {
        if (QFileInfo(src).exists()) qFatal("Unable to copy %s to %s. Check file permissions.", qPrintable(src), qPrintable(dst));
        else                         qFatal("Unable to copy %s to %s. File does not exist.", qPrintable(src), qPrintable(dst));
    }
}

void touchDir(const QDir &dir)
{
    if (dir.exists(".")) return;
    if (!dir.mkpath("."))
        qFatal("Unable to create path to dir %s", qPrintable(dir.absolutePath()));
}

void touchDir(const QFile &file)
{
    touchDir(QFileInfo(file));
}

void touchDir(const QFileInfo &fileInfo)
{
    touchDir(fileInfo.dir());
}

void emptyDir(QDir &dir)
{
    foreach (const QString &folder, dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks)) {
        QDir subdir(dir);
        bool success = subdir.cd(folder); if (!success) qFatal("cd failure.");
        emptyDir(subdir);
    }

    foreach (const QString &file, dir.entryList(QDir::Files))
        dir.remove(file);

    foreach (const QString &folder, dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks))
        dir.rmdir(folder);

    foreach (const QString &symlink, dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot))
        dir.remove(symlink);
}

void deleteDir(QDir &dir)
{
    emptyDir(dir);
    dir.rmdir(".");
}

QString find(const QString &file, const QString &alt)
{
    if (QFileInfo(file).exists()) return file;
    if (QFileInfo(alt).exists()) return alt;
    qFatal("Can't find file %s or alt %s\n", qPrintable(file), qPrintable(alt));
    return "";
}

bool toBool(const QString &string)
{
    bool ok;
    bool result = (string.toFloat(&ok) != 0.f);
    if (ok) return result;
    else    return (string != "FALSE") && (string != "false") && (string != "F") && (string != "f");
}

int toInt(const QString &string)
{
    bool ok;
    int result = string.toInt(&ok); if (!ok) qFatal("Expected integer value, got %s.", qPrintable(string));
    return result;
}

float toFloat(const QString &string)
{
    bool ok;
    float result = string.toFloat(&ok); if (!ok) qFatal("Expected floating point value, got %s.", qPrintable(string));
    return result;
}

QList<float> toFloats(const QStringList &strings)
{
    QList<float> floats;
    bool ok;
    foreach (const QString &string, strings) {
        floats.append(string.toFloat(&ok));
        if (!ok) qFatal("Failed to convert %s to floating point format.", qPrintable(string));
    }
    return floats;
}

QStringList toStringList(const QList<float> &values)
{
    QStringList result; result.reserve(values.size());
    foreach (float value, values)
        result.append(QString::number(value));
    return result;
}

QStringList toStringList(const std::vector<std::string> &string_list)
{
    QStringList result;
    foreach (const std::string &string, string_list)
        result.append(QString::fromStdString(string));
    return result;
}

QStringList toStringList(int num_strings, const char *strings[])
{
    QStringList result;
    for (int i=0; i<num_strings; i++)
        result.append(strings[i]);
    return result;
}

QString shortTextHash(QString string)
{
    string.remove(QRegExp("[{}<>&]"));
    return QString(QCryptographicHash::hash(qPrintable(string), QCryptographicHash::Md5).toBase64()).remove(QRegExp("[^a-zA-Z1-9]")).left(6);
}

QStringList parse(QString args, char split, bool *ok)
{
    if (args.isEmpty()) return QStringList();

    QStringList words;
    int start = 0;
    bool inQuote = false;
    QStack<QChar> subexpressions;
    for (int i=0; i<args.size(); i++) {
        if (inQuote) {
            if (args[i] == '\'')
                inQuote = false;
        } else {
            if (args[i] == '\'') {
                inQuote = true;
            } else if ((args[i] == '(') || (args[i] == '[') || (args[i] == '<') || (args[i] == '{')) {
                subexpressions.push(args[i]);
            } else if (args[i] == ')') {
                if (subexpressions.isEmpty() || (subexpressions.pop() != '(')) {
                    if (ok) *ok = false;
                    else    qFatal("Unexpected ')'.");
                    return words;
                }
            } else if (args[i] == ']') {
                if (subexpressions.isEmpty() || (subexpressions.pop() != '[')) {
                    if (ok) *ok = false;
                    else    qFatal("Unexpected ']'.");
                    return words;
                }
            } else if (args[i] == '>') {
                if (subexpressions.isEmpty() || (subexpressions.pop() != '<')) {
                    if (ok) *ok = false;
                    else    qFatal("Unexpected '>'.");
                    return words;
                }
            } else if (args[i] == '}') {
                if (subexpressions.isEmpty() || (subexpressions.pop() != '{')) {
                    if (ok) *ok = false;
                    else    qFatal("Unexpected '}'.");
                    return words;
                }
            } else if (subexpressions.isEmpty() && (args[i] == split)) {
                words.append(args.mid(start, i-start).trimmed());
                start = i+1;
            }
        }
    }

    if (ok) *ok = true;
    words.append(args.mid(start).trimmed());
    return words;
}

void checkArgsSize(const QString &name, const QStringList &args, int min, int max)
{
    if (max == -1) max = std::numeric_limits<int>::max();
    if (max == 0) max = min;
    if (args.size() < min) qFatal("%s expects at least %d arguments, got %d", qPrintable(name), min, args.size());
    if (args.size() > max) qFatal("%s expects no more than %d arguments, got %d", qPrintable(name), max, args.size());
}

QPointF toPoint(const QString &string, bool *ok)
{
    if (string.startsWith('(') && string.endsWith(')')) {
        bool okParse;
        const QStringList words = parse(string.mid(1, string.size()-2), ',', &okParse);
        if (okParse && (words.size() == 2)) {
            float x, y;
            bool okX, okY;
            x = words[0].toFloat(&okX);
            y = words[1].toFloat(&okY);
            if (okX && okY) {
                if (ok) *ok = true;
                return QPointF(x, y);
            }
        }
    }

    if (ok) *ok = false;
    return QPointF();
}

QRectF toRect(const QString &string, bool *ok)
{
    if (string.startsWith('(') && string.endsWith(')')) {
        bool okParse;
        const QStringList words = parse(string.mid(1, string.size()-2), ',', &okParse);
        if (okParse && (words.size() == 4)) {
            float x, y, width, height;
            bool okX, okY, okWidth, okHeight;
            x = words[0].toFloat(&okX);
            y = words[1].toFloat(&okY);
            width = words[2].toFloat(&okWidth);
            height = words[3].toFloat(&okHeight);
            if (okX && okY && okWidth && okHeight) {
                if (ok) *ok = true;
                return QRectF(x, y, width, height);
            }
        }
    }

    if (ok) *ok = false;
    return QRectF();
}

QStringList naturalSort(const QStringList &strings)
{
    QList<std::string> stdStrings; stdStrings.reserve(strings.size());
    foreach (const QString &string, strings)
        stdStrings.append(string.toStdString());

    std::sort(stdStrings.begin(), stdStrings.end(), doj::alphanum_less<std::string>());

    QStringList result; result.reserve(strings.size());
    foreach (const std::string &stdString, stdStrings)
        result.append(QString::fromStdString(stdString));

    return result;
}

bool runRScript(const QString &file)
{
    QProcess RScript;
    RScript.start("Rscript", QStringList() << file);
    RScript.waitForFinished(-1);
    bool result = ((RScript.exitCode() == 0) && (RScript.error() == QProcess::UnknownError));
    if (!result) qDebug("Failed to run 'Rscript', did you forget to install R?  "
                        "See online documentation of 'br_plot' for required R packages.  "
                        "Otherwise, try running Rscript on %s to get the exact error.", qPrintable(file));
    return result;
}

bool runDot(const QString &file)
{
    QProcess dot;
    dot.start("dot -Tpdf -O " + file);
    dot.waitForFinished(-1);
    return ((dot.exitCode() == 0) && (dot.error() == QProcess::UnknownError));
}

void showFile(const QString &file)
{
#ifndef BR_EMBEDDED
    (void) file;
    // A bug in Qt5 currently prevents us from doing this:
    // QDesktopServices::openUrl(QUrl::fromLocalFile(file));
#else // BR_EMBEDDED
    (void) file;
#endif // BR_EMBEDDED
}

QString toString(const QVariant &variant)
{
    if (variant.canConvert(QVariant::List)) return toString(qvariant_cast<QVariantList>(variant));
    else if (variant.canConvert(QVariant::String)) return variant.toString();
    else if (variant.canConvert(QVariant::PointF)) {
        QPointF point = qvariant_cast<QPointF>(variant);
        return QString("(%1,%2)").arg(QString::number(point.x()),QString::number(point.y()));
    } else if (variant.canConvert(QVariant::RectF)) {
        QRectF rect = qvariant_cast<QRectF>(variant);
        return QString("(%1,%2,%3,%4)").arg(QString::number(rect.x()),
                                            QString::number(rect.y()),
                                            QString::number(rect.width()),
                                            QString::number(rect.height()));
    }

    return QString();
}

QString toString(const QVariantList &variantList)
{
    QStringList variants;

    foreach (const QVariant &variant, variantList)
        variants.append(toString(variant));

    if (!variants.isEmpty()) return "[" + variants.join(", ") + "]";

    return QString();
}

QString toTime(int s)
{
    int h = s / (60*60);
    int m = (s - h*60*60) / 60;
    s = (s - h*60*60 - m*60);

    const QChar fillChar = QLatin1Char('0');

    return QString("%1:%2:%3").arg(h,2,10,fillChar).arg(m,2,10,fillChar).arg(s,2,10,fillChar);
}

float euclideanLength(const QPointF &point)
{
    return sqrt(pow(point.x(), 2) + pow(point.y(), 2));
}

}  // namespace QtUtils

