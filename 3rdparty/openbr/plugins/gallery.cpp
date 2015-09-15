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

#include <QtCore>
#include <QtConcurrentRun>
#ifndef BR_EMBEDDED
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QXmlStreamReader>
#endif // BR_EMBEDDED
#include <opencv2/highgui/highgui.hpp>
#include "openbr_internal.h"

#include "openbr/universal_template.h"
#include "openbr/core/bee.h"
#include "openbr/core/common.h"
#include "openbr/core/opencvutils.h"
#include "openbr/core/qtutils.h"

#ifdef CVMATIO
#include "MatlabIO.hpp"
#include "MatlabIOContainer.hpp"
#endif

#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#endif // _WIN32

namespace br
{

/*!
 * \ingroup galleries
 * \brief Weka ARFF file format.
 * \author Josh Klontz \cite jklontz
 * http://weka.wikispaces.com/ARFF+%28stable+version%29
 */
class arffGallery : public Gallery
{
    Q_OBJECT
    QFile arffFile;

    TemplateList readBlock(bool *done)
    {
        (void) done;
        qFatal("Not implemented.");
        return TemplateList();
    }

    void write(const Template &t)
    {
        if (!arffFile.isOpen()) {
            arffFile.setFileName(file.name);
            arffFile.open(QFile::WriteOnly);
            arffFile.write("% OpenBR templates\n"
                           "@RELATION OpenBR\n"
                           "\n");

            const int dimensions = t.m().rows * t.m().cols;
            for (int i=0; i<dimensions; i++)
                arffFile.write(qPrintable("@ATTRIBUTE v" + QString::number(i) + " REAL\n"));
            arffFile.write(qPrintable("@ATTRIBUTE class string\n"));

            arffFile.write("\n@DATA\n");
        }

        arffFile.write(qPrintable(OpenCVUtils::matrixToStringList(t).join(',')));
        arffFile.write(qPrintable(",'" + t.file.get<QString>("Label") + "'\n"));
    }

    void init()
    {
        //
    }
};

BR_REGISTER(Gallery, arffGallery)

class BinaryGallery : public Gallery
{
    Q_OBJECT

    void init()
    {
        const QString baseName = file.baseName();

        if (baseName == "stdin") {
#ifdef _WIN32
            if(_setmode(_fileno(stdin), _O_BINARY) == -1)
                qFatal("Failed to set stdin to binary mode!");
#endif // _WIN32

            gallery.open(stdin, QFile::ReadOnly);
        } else if (baseName == "stdout") {
#ifdef _WIN32
            if(_setmode(_fileno(stdout), _O_BINARY) == -1)
                qFatal("Failed to set stdout to binary mode!");
#endif // _WIN32

            gallery.open(stdout, QFile::WriteOnly);
        } else if (baseName == "stderr") {
#ifdef _WIN32
            if(_setmode(_fileno(stderr), _O_BINARY) == -1)
                qFatal("Failed to set stderr to binary mode!");
#endif // _WIN32

            gallery.open(stderr, QFile::WriteOnly);
        } else {
            gallery.setFileName(file);
            if (file.get<bool>("remove"))
                gallery.remove();
            QtUtils::touchDir(gallery);
            QFile::OpenMode mode = QFile::ReadWrite;

            if (file.get<bool>("append"))
                mode |= QFile::Append;

            if (!gallery.open(mode))
                qFatal("Can't open gallery: %s", qPrintable(gallery.fileName()));
        }
        stream.setDevice(&gallery);
    }

    TemplateList readBlock(bool *done)
    {
        if (gallery.atEnd())
            gallery.seek(0);

        TemplateList templates;
        while ((templates.size() < readBlockSize) && !gallery.atEnd()) {
            const Template t = readTemplate();
            if (!t.isEmpty() || !t.file.isNull()) {
                templates.append(t);
                templates.last().file.set("progress", position());
            }

            // Special case for pipes where we want to process data as soon as it is available
            if (gallery.isSequential())
                break;
        }

        *done = gallery.atEnd();
        return templates;
    }

    void write(const Template &t)
    {
        writeTemplate(t);
        if (gallery.isSequential())
            gallery.flush();
    }

protected:
    QFile gallery;
    QDataStream stream;

    qint64 totalSize()
    {
        return gallery.size();
    }

    qint64 position()
    {
        return gallery.pos();
    }

    virtual Template readTemplate() = 0;
    virtual void writeTemplate(const Template &t) = 0;
};

/*!
 * \ingroup galleries
 * \brief A binary gallery.
 *
 * Designed to be a literal translation of templates to disk.
 * Compatible with TemplateList::fromBuffer.
 * \author Josh Klontz \cite jklontz
 */
class galGallery : public BinaryGallery
{
    Q_OBJECT

    Template readTemplate()
    {
        Template t;
        stream >> t;
        return t;
    }

    void writeTemplate(const Template &t)
    {
        if (t.isEmpty() && t.file.isNull())
            return;
        stream << t;
    }
};

BR_REGISTER(Gallery, galGallery)

/*!
 * \ingroup galleries
 * \brief A contiguous array of br_universal_template.
 * \author Josh Klontz \cite jklontz
 */
class utGallery : public BinaryGallery
{
    Q_OBJECT

    Template readTemplate()
    {
        Template t;
        br_universal_template ut;
        if (gallery.read((char*)&ut, sizeof(br_universal_template)) == sizeof(br_universal_template)) {
            QByteArray data(ut.size, Qt::Uninitialized);
            char *dst = data.data();
            qint64 bytesNeeded = ut.size;
            while (bytesNeeded > 0) {
                qint64 bytesRead = gallery.read(dst, bytesNeeded);
                if (bytesRead <= 0) {
                    qDebug() << gallery.errorString();
                    qFatal("Unexepected EOF while reading universal template data, needed: %d more of: %d bytes.", int(bytesNeeded), int(ut.size));
                }
                bytesNeeded -= bytesRead;
                dst += bytesRead;
            }

            if (QCryptographicHash::hash(data.mid(ut.urlSize), QCryptographicHash::Md5) != QByteArray((const char*)ut.templateID, 16))
                qFatal("MD5 hash check failure!");

            t.file.set("ImageID", QVariant(QByteArray((const char*)ut.imageID, 16).toHex()));
            t.file.set("TemplateID", QVariant(QByteArray((const char*)ut.templateID, 16).toHex()));
            t.file.set("AlgorithmID", ut.algorithmID);
            t.file.set("URL", QString(data.data()));
            char *dataStart = data.data() + ut.urlSize;
            uint32_t dataSize = ut.size - ut.urlSize;
            if (ut.algorithmID == -1 || ut.algorithmID == -2) {
                t.file.set("FrontalFace", QRectF(ut.x, ut.y, ut.width, ut.height));
                uint32_t *rightEyeX = reinterpret_cast<uint32_t*>(dataStart);
                dataStart += sizeof(uint32_t);
                uint32_t *rightEyeY = reinterpret_cast<uint32_t*>(dataStart);
                dataStart += sizeof(uint32_t);
                uint32_t *leftEyeX = reinterpret_cast<uint32_t*>(dataStart);
                dataStart += sizeof(uint32_t);
                uint32_t *leftEyeY = reinterpret_cast<uint32_t*>(dataStart);
                dataStart += sizeof(uint32_t);
                dataSize -= sizeof(uint32_t)*4;
                t.file.set("First_Eye", QPointF(*rightEyeX, *rightEyeY));
                t.file.set("Second_Eye", QPointF(*leftEyeX, *leftEyeY));
            }
            else {
                t.file.set("X", ut.x);
                t.file.set("Y", ut.y);
                t.file.set("Width", ut.width);
                t.file.set("Height", ut.height);
            }
            t.append(cv::Mat(1, dataSize, CV_8UC1, dataStart).clone() /* We don't want a shallow copy! */);
        } else {
            if (!gallery.atEnd())
                qFatal("Failed to read universal template header!");
        }
        return t;
    }

    void writeTemplate(const Template &t)
    {
        const QByteArray imageID = QByteArray::fromHex(t.file.get<QByteArray>("ImageID", QByteArray(32, '0')));
        if (imageID.size() != 16)
            qFatal("Expected 16-byte ImageID, got: %d bytes.", imageID.size());

        const int32_t algorithmID = (t.isEmpty() || t.file.fte) ? 0 : t.file.get<int32_t>("AlgorithmID");
        const QByteArray url = t.file.get<QString>("URL", t.file.name).toLatin1();

        uint32_t x = 0, y = 0, width = 0, height = 0;
        QByteArray header;
        if (algorithmID == -1 || algorithmID == -2) {
            const QRectF frontalFace = t.file.get<QRectF>("FrontalFace");
            x      = frontalFace.x();
            y      = frontalFace.y();
            width  = frontalFace.width();
            height = frontalFace.height();

            const QPointF firstEye   = t.file.get<QPointF>("First_Eye");
            const QPointF secondEye  = t.file.get<QPointF>("Second_Eye");
            const uint32_t rightEyeX = firstEye.x();
            const uint32_t rightEyeY = firstEye.y();
            const uint32_t leftEyeX  = secondEye.x();
            const uint32_t leftEyeY  = secondEye.y();

            header.append((const char*)&rightEyeX, sizeof(uint32_t));
            header.append((const char*)&rightEyeY, sizeof(uint32_t));
            header.append((const char*)&leftEyeX , sizeof(uint32_t));
            header.append((const char*)&leftEyeY , sizeof(uint32_t));
        } else {
            x = t.file.get<uint32_t>("X", 0);
            y = t.file.get<uint32_t>("Y", 0);
            width = t.file.get<uint32_t>("Width", 0);
            height = t.file.get<uint32_t>("Height", 0);
        }

        QCryptographicHash templateID(QCryptographicHash::Md5);
        templateID.addData(header);
        if (algorithmID != 0)
            templateID.addData((const char*) t.m().data, t.m().rows * t.m().cols * t.m().elemSize());

        gallery.write(imageID);
        gallery.write(templateID.result());
        gallery.write((const char*) &algorithmID, sizeof(uint32_t));
        gallery.write((const char*) &x          , sizeof(uint32_t));
        gallery.write((const char*) &y          , sizeof(uint32_t));
        gallery.write((const char*) &width      , sizeof(uint32_t));
        gallery.write((const char*) &height     , sizeof(uint32_t));

        const uint32_t urlSize = url.size() + 1;
        gallery.write((const char*) &urlSize, sizeof(uint32_t));

        const uint32_t size = urlSize + header.size() + (algorithmID == 0 ? 0 : t.m().rows * t.m().cols * t.m().elemSize());
        gallery.write((const char*) &size, sizeof(uint32_t));

        gallery.write((const char*) url.data(), urlSize);
        gallery.write(header);
        if (algorithmID != 0)
            gallery.write((const char*) t.m().data, t.m().rows * t.m().cols * t.m().elemSize());
    }
};

BR_REGISTER(Gallery, utGallery)

/*!
 * \ingroup galleries
 * \brief Newline-separated URLs.
 * \author Josh Klontz \cite jklontz
 */
class urlGallery : public BinaryGallery
{
    Q_OBJECT

    Template readTemplate()
    {
        Template t;
        const QString url = QString::fromLocal8Bit(gallery.readLine()).simplified();
        if (!url.isEmpty())
            t.file.set("URL", url);
        return t;
    }

    void writeTemplate(const Template &t)
    {
        const QString url = t.file.get<QString>("URL", t.file.name);
        if (!url.isEmpty()) {
            gallery.write(qPrintable(url));
            gallery.write("\n");
        }
    }
};

BR_REGISTER(Gallery, urlGallery)

/*!
 * \ingroup galleries
 * \brief Newline-separated JSON objects.
 * \author Josh Klontz \cite jklontz
 */
class jsonGallery : public BinaryGallery
{
    Q_OBJECT

    Template readTemplate()
    {
        QJsonParseError error;
        const QByteArray line = gallery.readLine().simplified();
        if (line.isEmpty())
            return Template();
        File file = QJsonDocument::fromJson(line, &error).object().toVariantMap();
        if (error.error != QJsonParseError::NoError) {
            qWarning("Couldn't parse: %s\n", line.data());
            qFatal("%s\n", qPrintable(error.errorString()));
        }
        return file;
    }

    void writeTemplate(const Template &t)
    {
        const QByteArray json = QJsonDocument(QJsonObject::fromVariantMap(t.file.localMetadata())).toJson().replace('\n', "");
        if (!json.isEmpty()) {
            gallery.write(json);
            gallery.write("\n");
        }
    }
};

BR_REGISTER(Gallery, jsonGallery)

/*!
 * \ingroup galleries
 * \brief Reads/writes templates to/from folders.
 * \author Josh Klontz \cite jklontz
 * \param regexp An optional regular expression to match against the files extension.
 */
class EmptyGallery : public Gallery
{
    Q_OBJECT
    Q_PROPERTY(QString regexp READ get_regexp WRITE set_regexp RESET reset_regexp STORED false)
    BR_PROPERTY(QString, regexp, QString())

    qint64 gallerySize;

    void init()
    {
        QDir dir(file.name);
        QtUtils::touchDir(dir);
        gallerySize = dir.count();
    }

    TemplateList readBlock(bool *done)
    {
        TemplateList templates;
        *done = true;

        // Enrolling a null file is used as an idiom to initialize an algorithm
        if (file.isNull()) return templates;

        // Add immediate subfolders
        QDir dir(file);
        QList< QFuture<TemplateList> > futures;
        foreach (const QString &folder, QtUtils::naturalSort(dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot))) {
            const QDir subdir = dir.absoluteFilePath(folder);
            futures.append(QtConcurrent::run(&EmptyGallery::getTemplates, subdir));
        }
        foreach (const QFuture<TemplateList> &future, futures)
            templates.append(future.result());

        // Add root folder
        foreach (const QString &fileName, QtUtils::getFiles(file.name, false))
            templates.append(File(fileName, dir.dirName()));

        if (!regexp.isEmpty()) {
            QRegExp re(regexp);
            re.setPatternSyntax(QRegExp::Wildcard);
            for (int i=templates.size()-1; i>=0; i--) {
                if (!re.exactMatch(templates[i].file.fileName())) {
                    templates.removeAt(i);
                }
            }
        }

        for (int i = 0; i < templates.size(); i++) templates[i].file.set("progress", i);

        return templates;
    }

    void write(const Template &t)
    {
        static QMutex diskLock;

        // Enrolling a null file is used as an idiom to initialize an algorithm
        if (file.name.isEmpty()) return;

        const QString newFormat = file.get<QString>("newFormat",QString());
        QString destination = file.name + "/" + (file.getBool("preservePath") ? t.file.path()+"/" : QString());
        destination += (newFormat.isEmpty() ? t.file.fileName() : t.file.baseName()+newFormat);

        QMutexLocker diskLocker(&diskLock); // Windows prefers to crash when writing to disk in parallel
        if (t.isNull()) {
            QtUtils::copyFile(t.file.resolved(), destination);
        } else {
            QScopedPointer<Format> format(Factory<Format>::make(destination));
            format->write(t);
        }
    }

    qint64 totalSize()
    {
        return gallerySize;
    }

    static TemplateList getTemplates(const QDir &dir)
    {
        const QStringList files = QtUtils::getFiles(dir, true);
        TemplateList templates; templates.reserve(files.size());
        foreach (const QString &file, files)
            templates.append(File(file, dir.dirName()));
        return templates;
    }
};

BR_REGISTER(Gallery, EmptyGallery)

/*!
 * \ingroup galleries
 * \brief Crawl a root location for image files.
 * \author Josh Klontz \cite jklontz
 */
class crawlGallery : public Gallery
{
    Q_OBJECT
    Q_PROPERTY(bool autoRoot READ get_autoRoot WRITE set_autoRoot RESET reset_autoRoot STORED false)
    Q_PROPERTY(int depth READ get_depth WRITE set_depth RESET reset_depth STORED false)
    Q_PROPERTY(bool depthFirst READ get_depthFirst WRITE set_depthFirst RESET reset_depthFirst STORED false)
    Q_PROPERTY(int images READ get_images WRITE set_images RESET reset_images STORED false)
    Q_PROPERTY(bool json READ get_json WRITE set_json RESET reset_json STORED false)
    Q_PROPERTY(int timeLimit READ get_timeLimit WRITE set_timeLimit RESET reset_timeLimit STORED false)
    BR_PROPERTY(bool, autoRoot, false)
    BR_PROPERTY(int, depth, INT_MAX)
    BR_PROPERTY(bool, depthFirst, false)
    BR_PROPERTY(int, images, INT_MAX)
    BR_PROPERTY(bool, json, false)
    BR_PROPERTY(int, timeLimit, INT_MAX)

    QTime elapsed;
    TemplateList templates;

    void crawl(QFileInfo url, int currentDepth = 0)
    {
        if ((templates.size() >= images) || (currentDepth >= depth) || (elapsed.elapsed()/1000 >= timeLimit))
            return;

        if (url.filePath().startsWith("file://"))
            url = QFileInfo(url.filePath().mid(7));

        if (url.isDir()) {
            const QDir dir(url.absoluteFilePath());
            const QFileInfoList files = dir.entryInfoList(QDir::Files);
            const QFileInfoList subdirs = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
            foreach (const QFileInfo &first, depthFirst ? subdirs : files)
                crawl(first, currentDepth + 1);
            foreach (const QFileInfo &second, depthFirst ? files : subdirs)
                crawl(second, currentDepth + 1);
        } else if (url.isFile()) {
            const QString suffix = url.suffix();
            if ((suffix == "bmp") || (suffix == "jpg") || (suffix == "jpeg") || (suffix == "png") || (suffix == "tiff")) {
                File f;
                if (json) f.set("URL", "file://"+url.canonicalFilePath());
                else      f.name = "file://"+url.canonicalFilePath();
                templates.append(f);
            }
        }
    }

    void init()
    {
        elapsed.start();
        const QString root = file.name.mid(0, file.name.size()-6); // Remove .crawl suffix";
        if (!root.isEmpty()) {
            crawl(root);
        } else {
            if (autoRoot) {
                foreach (const QString &path, QStandardPaths::standardLocations(QStandardPaths::HomeLocation))
                    crawl(path);
            } else {
                QFile file;
                file.open(stdin, QFile::ReadOnly);
                while (!file.atEnd()) {
                    const QString url = QString::fromLocal8Bit(file.readLine()).simplified();
                    if (!url.isEmpty())
                        crawl(url);
                }
            }
        }
    }

    TemplateList readBlock(bool *done)
    {
        *done = true;
        return templates;
    }

    void write(const Template &)
    {
        qFatal("Not supported");
    }
};

BR_REGISTER(Gallery, crawlGallery)

/*!
 * \ingroup galleries
 * \brief Treats the gallery as a br::Format.
 * \author Josh Klontz \cite jklontz
 */
class DefaultGallery : public Gallery
{
    Q_OBJECT

    TemplateList readBlock(bool *done)
    {
        *done = true;
        return TemplateList() << file;
    }

    void write(const Template &t)
    {
        QScopedPointer<Format> format(Factory<Format>::make(file));
        format->write(t);
    }

    qint64 totalSize()
    {
        return 1;
    }
};

BR_REGISTER(Gallery, DefaultGallery)

/*!
 * \ingroup galleries
 * \brief Combine all templates into one large matrix and process it as a br::Format
 * \author Josh Klontz \cite jklontz
 */
class matrixGallery : public Gallery
{
    Q_OBJECT
    Q_PROPERTY(const QString extension READ get_extension WRITE set_extension RESET reset_extension STORED false)
    BR_PROPERTY(QString, extension, "mtx")

    TemplateList templates;

    ~matrixGallery()
    {
        if (templates.isEmpty())
            return;

        QScopedPointer<Format> format(Factory<Format>::make(getFormat()));
        format->write(Template(file, OpenCVUtils::toMat(templates.data())));
    }

    File getFormat() const
    {
        return file.name.left(file.name.size() - file.suffix().size()) + extension;
    }

    TemplateList readBlock(bool *done)
    {
        *done = true;
        return TemplateList() << getFormat();
    }

    void write(const Template &t)
    {
        templates.append(t);
    }
};

BR_REGISTER(Gallery, matrixGallery)

/*!
 * \ingroup initializers
 * \brief Initialization support for memGallery.
 * \author Josh Klontz \cite jklontz
 */
class MemoryGalleries : public Initializer
{
    Q_OBJECT

    void initialize() const {}

    void finalize() const
    {
        galleries.clear();
    }

public:
    static QHash<File, TemplateList> galleries; /*!< TODO */
    static QHash<File, bool> aligned; /*!< TODO */
};

QHash<File, TemplateList> MemoryGalleries::galleries;
QHash<File, bool> MemoryGalleries::aligned;

BR_REGISTER(Initializer, MemoryGalleries)

/*!
 * \ingroup galleries
 * \brief A gallery held in memory.
 * \author Josh Klontz \cite jklontz
 */
class memGallery : public Gallery
{
    Q_OBJECT
    int block;
    qint64 gallerySize;

    void init()
    {
        block = 0;
        File galleryFile = file.name.mid(0, file.name.size()-4);
        if ((galleryFile.suffix() == "gal") && galleryFile.exists() && !MemoryGalleries::galleries.contains(file)) {
            QSharedPointer<Gallery> gallery(Factory<Gallery>::make(galleryFile));
            MemoryGalleries::galleries[file] = gallery->read();
            align(MemoryGalleries::galleries[file]);
            MemoryGalleries::aligned[file] = true;
            gallerySize = MemoryGalleries::galleries[file].size();
        }
    }

    TemplateList readBlock(bool *done)
    {
        if (!MemoryGalleries::aligned[file]) {
            align(MemoryGalleries::galleries[file]);
            MemoryGalleries::aligned[file] = true;
        }

        TemplateList templates = MemoryGalleries::galleries[file].mid(block*readBlockSize, readBlockSize);
        for (qint64 i = 0; i < templates.size();i++) {
            templates[i].file.set("progress", i + block * readBlockSize);
        }

        *done = (templates.size() < readBlockSize);
        block = *done ? 0 : block+1;
        return templates;
    }

    void write(const Template &t)
    {
        MemoryGalleries::galleries[file].append(t);
        MemoryGalleries::aligned[file] = false;
    }

    static void align(TemplateList &templates)
    {
        if (!templates.empty() && templates[0].size() > 1) return;

        bool uniform = true;
        QVector<uchar> alignedData(templates.bytes<size_t>());
        size_t offset = 0;
        for (int i=0; i<templates.size(); i++) {
            Template &t = templates[i];
            if (t.size() > 1) qFatal("Can't handle multi-matrix template %s.", qPrintable(t.file.flat()));

            cv::Mat &m = t;
            if (m.data) {
                const size_t size = m.total() * m.elemSize();
                if (!m.isContinuous()) qFatal("Requires continuous matrix data of size %d for %s.", (int)size, qPrintable(t.file.flat()));
                memcpy(&(alignedData.data()[offset]), m.ptr(), size);
                m = cv::Mat(m.rows, m.cols, m.type(), &(alignedData.data()[offset]));
                offset += size;
            }
            uniform = uniform &&
                      (m.rows == templates.first().m().rows) &&
                      (m.cols == templates.first().m().cols) &&
                      (m.type() == templates.first().m().type());
        }

        templates.uniform = uniform;
        templates.alignedData = alignedData;
    }

    qint64 totalSize()
    {
        return gallerySize;
    }

    qint64 position()
    {
        return block * readBlockSize;
    }

};

BR_REGISTER(Gallery, memGallery)

FileList FileList::fromGallery(const File &file, bool cache)
{
    File targetMeta = file;
    targetMeta.name = targetMeta.path() + targetMeta.baseName() + "_meta" + targetMeta.hash() + ".mem";

    FileList fileData;

    // Did we already read the data?
    if (MemoryGalleries::galleries.contains(targetMeta))
    {
        return MemoryGalleries::galleries[targetMeta].files();
    }

    TemplateList templates;
    // OK we read the data in some form, does the gallery type containing matrices?
    if ((QStringList() << "gal" << "mem" << "template").contains(file.suffix())) {
        // Retrieve it block by block, dropping matrices from read templates.
        QScopedPointer<Gallery> gallery(Gallery::make(file));
        gallery->set_readBlockSize(10);
        bool done = false;
        while (!done)
        {
            TemplateList tList = gallery->readBlock(&done);
            for (int i=0; i < tList.size();i++)
            {
                tList[i].clear();
                templates.append(tList[i].file);
            }
        }
    }
    else {
        // this is a gallery format that doesn't include matrices, so we can just read it
        QScopedPointer<Gallery> gallery(Gallery::make(file));
        templates= gallery->read();
    }

    if (cache)
    {
        QScopedPointer<Gallery> memOutput(Gallery::make(targetMeta));
        memOutput->writeBlock(templates);
    }
    fileData = templates.files();
    return fileData;
}

/*!
 * \ingroup galleries
 * \brief Treats each line as a file.
 * \author Josh Klontz \cite jklontz
 *
 * Columns should be comma separated with first row containing headers.
 * The first column in the file should be the path to the file to enroll.
 * Other columns will be treated as file metadata.
 *
 * \see txtGallery
 */
class csvGallery : public FileGallery
{
    Q_OBJECT
    Q_PROPERTY(int fileIndex READ get_fileIndex WRITE set_fileIndex RESET reset_fileIndex)
    BR_PROPERTY(int, fileIndex, 0)

    FileList files;
    QStringList headers;

    ~csvGallery()
    {
        f.close();

        if (files.isEmpty()) return;

        QMap<QString,QVariant> samples;
        foreach (const File &file, files)
            foreach (const QString &key, file.localKeys())
                if (!samples.contains(key))
                    samples.insert(key, file.value(key));

        // Don't create columns in the CSV for these special fields
        samples.remove("Points");
        samples.remove("Rects");

        QStringList lines;
        lines.reserve(files.size()+1);

        { // Make header
            QStringList words;
            words.append("File");
            foreach (const QString &key, samples.keys())
                words.append(getCSVElement(key, samples[key], true));
            lines.append(words.join(","));
        }

        // Make table
        foreach (const File &file, files) {
            QStringList words;
            words.append(file.name);
            foreach (const QString &key, samples.keys())
                words.append(getCSVElement(key, file.value(key), false));
            lines.append(words.join(","));
        }

        QtUtils::writeFile(file, lines);
    }

    TemplateList readBlock(bool *done)
    {
        *done = false;
        TemplateList templates;
        if (!file.exists()) {
            *done = true;
            return templates;
        }
        QRegExp regexp("\\s*,\\s*");

        if (f.pos() == 0)
        {
            // read a line
            QByteArray lineBytes = f.readLine();
            QString line = QString::fromLocal8Bit(lineBytes).trimmed();
            headers = line.split(regexp);
        }

        for (qint64 i = 0; i < this->readBlockSize && !f.atEnd(); i++){
            QByteArray lineBytes = f.readLine();
            QString line = QString::fromLocal8Bit(lineBytes).trimmed();

            QStringList words = line.split(regexp);
            if (words.size() != headers.size()) continue;
            File fi;
            for (int j=0; j<words.size(); j++) {
                if (j == 0) fi.name = words[j];
                else        fi.set(headers[j], words[j]);
            }
            templates.append(fi);
            templates.last().file.set("progress", f.pos());
        }
        *done = f.atEnd();

        return templates;
    }

    void write(const Template &t)
    {
        files.append(t.file);
    }

    static QString getCSVElement(const QString &key, const QVariant &value, bool header)
    {
        if (value.canConvert<QString>()) {
            if (header) return key;
            else        return value.value<QString>();
        } else if (value.canConvert<QPointF>()) {
            const QPointF point = value.value<QPointF>();
            if (header) return key+"_X,"+key+"_Y";
            else        return QString::number(point.x())+","+QString::number(point.y());
        } else if (value.canConvert<QRectF>()) {
            const QRectF rect = value.value<QRectF>();
            if (header) return key+"_X,"+key+"_Y,"+key+"_Width,"+key+"_Height";
            else        return QString::number(rect.x())+","+QString::number(rect.y())+","+QString::number(rect.width())+","+QString::number(rect.height());
        } else {
            if (header) return key;
            else        return QString::number(std::numeric_limits<float>::quiet_NaN());
        }
    }
};

BR_REGISTER(Gallery, csvGallery)

/*!
 * \ingroup galleries
 * \brief Treats each line as a file.
 * \author Josh Klontz \cite jklontz
 *
 * The entire line is treated as the file path. An optional label may be specified using a space ' ' separator:
 *
\verbatim
<FILE>
<FILE>
...
<FILE>
\endverbatim
 * or
\verbatim
<FILE> <LABEL>
<FILE> <LABEL>
...
<FILE> <LABEL>
\endverbatim
 * \see csvGallery
 */
class txtGallery : public FileGallery
{
    Q_OBJECT
    Q_PROPERTY(QString label READ get_label WRITE set_label RESET reset_label STORED false)
    BR_PROPERTY(QString, label, "")

    TemplateList readBlock(bool *done)
    {
        *done = false;
        if (f.atEnd())
            f.seek(0);

        TemplateList templates;

        for (qint64 i = 0; i < readBlockSize; i++)
        {
            QByteArray lineBytes = f.readLine();
            QString line = QString::fromLocal8Bit(lineBytes).trimmed();

            if (!line.isEmpty()){
                int splitIndex = line.lastIndexOf(' ');
                if (splitIndex == -1) templates.append(File(line));
                else                  templates.append(File(line.mid(0, splitIndex), line.mid(splitIndex+1)));
                templates.last().file.set("progress", this->position());
            }

            if (f.atEnd()) {
                *done=true;
                break;
            }
        }

        return templates;
    }

    void write(const Template &t)
    {
        QString line = t.file.name;
        if (!label.isEmpty())
            line += " " + t.file.get<QString>(label);

        f.write((line+"\n").toLocal8Bit() );
    }
};

BR_REGISTER(Gallery, txtGallery)

/*!
 * \ingroup galleries
 * \brief Treats each line as a call to File::flat()
 * \author Josh Klontz \cite jklontz
 */
class flatGallery : public FileGallery
{
    Q_OBJECT

    TemplateList readBlock(bool *done)
    {
        *done = false;
        if (f.atEnd())
            f.seek(0);

        TemplateList templates;

        for (qint64 i = 0; i < readBlockSize; i++)
        {
            QByteArray line = f.readLine();

            if (!line.isEmpty()) {
                templates.append(File(QString::fromLocal8Bit(line).trimmed()));
                templates.last().file.set("progress", this->position());
            }

            if (f.atEnd()) {
                *done=true;
                break;
            }
        }

        return templates;
    }

    void write(const Template &t)
    {
        f.write((t.file.flat()+"\n").toLocal8Bit() );
    }
};

BR_REGISTER(Gallery, flatGallery)

/*!
 * \ingroup galleries
 * \brief A \ref sigset input.
 * \author Josh Klontz \cite jklontz
 */
class xmlGallery : public FileGallery
{
    Q_OBJECT
    Q_PROPERTY(bool ignoreMetadata READ get_ignoreMetadata WRITE set_ignoreMetadata RESET reset_ignoreMetadata STORED false)
    BR_PROPERTY(bool, ignoreMetadata, false)
    FileList files;

    QXmlStreamReader reader;

    QString currentSignatureName;
    bool signatureActive;

    ~xmlGallery()
    {
        f.close();
        if (!files.isEmpty())
            BEE::writeSigset(file, files, ignoreMetadata);
    }

    TemplateList readBlock(bool *done)
    {
        if (reader.atEnd())
            f.seek(0);

        TemplateList templates;
        qint64 count = 0;

        while (!reader.atEnd())
        {
            // if an identity is active we try to read presentations
            if (signatureActive)
            {
                while (signatureActive)
                {
                    QXmlStreamReader::TokenType signatureToken = reader.readNext();

                    // did the signature end?
                    if (signatureToken == QXmlStreamReader::EndElement && reader.name() == "biometric-signature") {
                        signatureActive = false;
                        break;
                    }

                    // did we reach the end of the document? Theoretically this shoudln't happen without reaching the end of
                    if (signatureToken == QXmlStreamReader::EndDocument)
                        break;

                    // a presentation!
                    if (signatureToken == QXmlStreamReader::StartElement && reader.name() == "presentation") {
                        templates.append(Template(File("",currentSignatureName)));
                        foreach (const QXmlStreamAttribute &attribute, reader.attributes()) {
                            // file-name is stored directly on file, not as a key/value pair
                            if (attribute.name() == "file-name")
                                templates.last().file.name = attribute.value().toString();
                            // other values are directly set as metadata
                            else if (!ignoreMetadata) templates.last().file.set(attribute.name().toString(), attribute.value().toString());
                        }

                        // a presentation can have bounding boxes as child elements
                        QList<QRectF> rects = templates.last().file.rects();
                        while (true)
                        {
                            QXmlStreamReader::TokenType pToken = reader.readNext();
                            if (pToken == QXmlStreamReader::EndElement && reader.name() == "presentation")
                                break;

                            if (pToken == QXmlStreamReader::StartElement)
                            {
                                if (reader.attributes().hasAttribute("x")
                                    && reader.attributes().hasAttribute("y")
                                    && reader.attributes().hasAttribute("width")
                                    && reader.attributes().hasAttribute("height") )
                                {
                                    // get bounding box properties as attributes, just going to assume this all works
                                    qreal x = reader.attributes().value("x").string()->toDouble();
                                    qreal y = reader.attributes().value("y").string()->toDouble();
                                    qreal width =  reader.attributes().value("width").string()->toDouble();
                                    qreal height = reader.attributes().value("height").string()->toDouble();
                                    rects += QRectF(x, y, width, height);
                                }
                            }
                        }
                        templates.last().file.setRects(rects);
                        templates.last().file.set("progress", f.pos());

                        // we read another complete template
                        count++;
                    }
                }
            }
            // otherwise, keep reading elements until the next identity is reacehed
            else
            {
                QXmlStreamReader::TokenType token = reader.readNext();

                // end of file?
                if (token == QXmlStreamReader::EndDocument)
                    break;

                // we are only interested in new elements
                if (token != QXmlStreamReader::StartElement)
                    continue;

                QStringRef elName = reader.name();

                // biometric-signature-set is the root element
                if (elName == "biometric-signature-set")
                    continue;

                // biometric-signature -- an identity
                if (elName == "biometric-signature")
                {
                    // read the name associated with the current signature
                    if (!reader.attributes().hasAttribute("name"))
                    {
                        qDebug() << "Biometric signature missing name";
                        continue;
                    }
                    currentSignatureName = reader.attributes().value("name").toString();
                    signatureActive = true;

                    // If we've already read enough templates for this block, then break here.
                    // We wait untill the start of the next signature to be sure that done should
                    // actually be false (i.e. there are actually items left in this file)
                    if (count >= this->readBlockSize) {
                        *done = false;
                        return templates;
                    }

                }
            }
        }
        *done = true;

        return templates;
    }

    void write(const Template &t)
    {
        files.append(t.file);
    }

    void init()
    {
        FileGallery::init();
        reader.setDevice(&f);
    }
};

BR_REGISTER(Gallery, xmlGallery)

/*!
 * \ingroup galleries
 * \brief Treat the file as a single binary template.
 * \author Josh Klontz \cite jklontz
 */
class templateGallery : public Gallery
{
    Q_OBJECT

    TemplateList readBlock(bool *done)
    {
        *done = true;
        QByteArray data;
        QtUtils::readFile(file.name.left(file.name.size()-QString(".template").size()), data);
        return TemplateList() << Template(file, cv::Mat(1, data.size(), CV_8UC1, data.data()).clone());
    }

    void write(const Template &t)
    {
        (void) t;
        qFatal("No supported.");
    }

    void init()
    {
        //
    }
};

BR_REGISTER(Gallery, templateGallery)

/*!
 * \ingroup galleries
 * \brief Database input.
 * \author Josh Klontz \cite jklontz
 */
class dbGallery : public Gallery
{
    Q_OBJECT

    TemplateList readBlock(bool *done)
    {
        TemplateList templates;
        br::File import = file.get<QString>("import", "");
        QString query = file.get<QString>("query");
        QString subset = file.get<QString>("subset", "");

#ifndef BR_EMBEDDED
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName(file);
        if (!db.open()) qFatal("Failed to open SQLite database %s.", qPrintable(file.name));

        if (!import.isNull()) {
            qDebug("Parsing %s", qPrintable(import.name));
            QStringList lines = QtUtils::readLines(import);
            QList<QStringList> cells; cells.reserve(lines.size());
            const QRegExp re("\\s*,\\s*");
            foreach (const QString &line, lines) {
                cells.append(line.split(re));
                if (cells.last().size() != cells.first().size()) qFatal("Column count mismatch.");
            }

            QStringList columns, qMarks;
            QList<QVariantList> variantLists;
            for (int i=0; i<cells[0].size(); i++) {
                bool isNumeric;
                cells[1][i].toInt(&isNumeric);
                columns.append(cells[0][i] + (isNumeric ? " INTEGER" : " STRING"));
                qMarks.append("?");

                QVariantList variantList; variantList.reserve(lines.size()-1);
                for (int j=1; j<lines.size(); j++) {
                    if (isNumeric) variantList << cells[j][i].toInt();
                    else           variantList << cells[j][i];
                }
                variantLists.append(variantList);
            }

            const QString &table = import.baseName();
            qDebug("Creating table %s", qPrintable(table));
            QSqlQuery q(db);
            if (!q.exec("CREATE TABLE " + table + " (" + columns.join(", ") + ");"))
                qFatal("%s.", qPrintable(q.lastError().text()));
            if (!q.prepare("insert into " + table + " values (" + qMarks.join(", ") + ")"))
                qFatal("%s.", qPrintable(q.lastError().text()));
            foreach (const QVariantList &vl, variantLists)
                q.addBindValue(vl);
            if (!q.execBatch()) qFatal("%s.", qPrintable(q.lastError().text()));
        }

        QSqlQuery q(db);
        if (query.startsWith('\'') && query.endsWith('\''))
            query = query.mid(1, query.size()-2);
        if (!q.exec(query))
            qFatal("%s.", qPrintable(q.lastError().text()));

        if ((q.record().count() == 0) || (q.record().count() > 3))
            qFatal("Query record expected one to three fields, got %d.", q.record().count());
        const bool hasMetadata = (q.record().count() >= 2);
        const bool hasFilter = (q.record().count() >= 3);

        QString labelName = "Label";
        if (q.record().count() >= 2)
            labelName = q.record().fieldName(1);

        // subset = seed:subjectMaxSize:numSubjects:subjectMinSize or
        // subset = seed:{Metadata,...,Metadata}:numSubjects
        int seed = 0, subjectMaxSize = std::numeric_limits<int>::max(), numSubjects = std::numeric_limits<int>::max(), subjectMinSize = 0;
        QList<QRegExp> metadataFields;
        if (!subset.isEmpty()) {
            const QStringList &words = subset.split(":");
            QtUtils::checkArgsSize("Input", words, 2, 4);
            if      (words[0] == "train") seed = 0;
            else if (words[0] == "test" ) seed = 1;
            else                          seed = QtUtils::toInt(words[0]);
            if (words[1].startsWith('{') && words[1].endsWith('}')) {
                foreach (const QString &regexp, words[1].mid(1, words[1].size()-2).split(","))
                    metadataFields.append(QRegExp(regexp));
                subjectMaxSize = metadataFields.size();
            } else {
                subjectMaxSize = QtUtils::toInt(words[1]);
            }
            numSubjects = words.size() >= 3 ? QtUtils::toInt(words[2]) : std::numeric_limits<int>::max();
            subjectMinSize = words.size() >= 4 ? QtUtils::toInt(words[3]) : subjectMaxSize;
        }

        srand(seed);

        typedef QPair<QString,QString> Entry; // QPair<File,Metadata>
        QHash<QString, QList<Entry> > entries; // QHash<Label, QList<Entry> >
        while (q.next()) {
            if (hasFilter && (seed >= 0) && (qHash(q.value(2).toString()) % 2 != (uint)seed % 2)) continue; // Ensures training and testing filters don't overlap

            if (metadataFields.isEmpty())
                entries[hasMetadata ? q.value(1).toString() : ""].append(QPair<QString,QString>(q.value(0).toString(), hasFilter ? q.value(2).toString() : ""));
            else
                entries[hasFilter ? q.value(2).toString() : ""].append(QPair<QString,QString>(q.value(0).toString(), hasMetadata ? q.value(1).toString() : ""));
        }

        QStringList labels = entries.keys();
        qSort(labels);

        if (hasFilter && ((labels.size() > numSubjects) || (numSubjects == std::numeric_limits<int>::max())))
            std::random_shuffle(labels.begin(), labels.end());

        foreach (const QString &label, labels) {
            QList<Entry> entryList = entries[label];
            if ((entryList.size() >= subjectMinSize) && (numSubjects > 0)) {

                if (!metadataFields.isEmpty()) {
                    QList<Entry> subEntryList;
                    foreach (const QRegExp &metadata, metadataFields) {
                        for (int i=0; i<entryList.size(); i++) {
                            if (metadata.exactMatch(entryList[i].second)) {
                                subEntryList.append(entryList.takeAt(i));
                                break;
                            }
                        }
                    }
                    if (subEntryList.size() == metadataFields.size())
                        entryList = subEntryList;
                    else
                        continue;
                }

                if (entryList.size() > subjectMaxSize)
                    std::random_shuffle(entryList.begin(), entryList.end());
                foreach (const Entry &entry, entryList.mid(0, subjectMaxSize)) {
                    templates.append(File(entry.first));
                    templates.last().file.set(labelName, label);
                }
                numSubjects--;
            }
        }

        db.close();
#endif // BR_EMBEDDED

        *done = true;
        return templates;
    }

    void write(const Template &t)
    {
        (void) t;
        qFatal("Not supported.");
    }

    void init()
    {
        //
    }
};

BR_REGISTER(Gallery, dbGallery)

/*!
 * \ingroup inputs
 * \brief Input from a google image search.
 * \author Josh Klontz \cite jklontz
 */
class googleGallery : public Gallery
{
    Q_OBJECT

    TemplateList readBlock(bool *done)
    {
        TemplateList templates;

        static const QString search = "http://images.google.com/images?q=%1&start=%2";
        QString query = file.name.left(file.name.size()-7); // remove ".google"

#ifndef BR_EMBEDDED
        QNetworkAccessManager networkAccessManager;
        for (int i=0; i<100; i+=20) { // Retrieve 100 images
            QNetworkRequest request(search.arg(query, QString::number(i)));
            QNetworkReply *reply = networkAccessManager.get(request);

            while (!reply->isFinished())
                QThread::yieldCurrentThread();

            QString data(reply->readAll());
            delete reply;

            QStringList words = data.split("imgurl=");
            words.takeFirst(); // Remove header
            foreach (const QString &word, words) {
                QString url = word.left(word.indexOf("&amp"));
                url = url.replace("%2520","%20");
                int junk = url.indexOf('%', url.lastIndexOf('.'));
                if (junk != -1) url = url.left(junk);
                templates.append(File(url,query));
            }
        }
#endif // BR_EMBEDDED

        *done = true;
        return templates;
    }

    void write(const Template &)
    {
        qFatal("Not supported.");
    }
};

BR_REGISTER(Gallery, googleGallery)

/*!
 * \ingroup galleries
 * \brief Print template statistics.
 * \author Josh Klontz \cite jklontz
 */
class statGallery : public Gallery
{
    Q_OBJECT
    QSet<QString> subjects;
    QList<int> bytes;

    ~statGallery()
    {
        int emptyTemplates = 0;
        for (int i=bytes.size()-1; i>=0; i--)
            if (bytes[i] == 0) {
                bytes.removeAt(i);
                emptyTemplates++;
            }

        double bytesMean, bytesStdDev;
        Common::MeanStdDev(bytes, &bytesMean, &bytesStdDev);
        printf("Subjects: %d\nEmpty Templates: %d/%d\nBytes/Template: %.4g +/- %.4g\n",
               subjects.size(), emptyTemplates, emptyTemplates+bytes.size(), bytesMean, bytesStdDev);
    }

    TemplateList readBlock(bool *done)
    {
        *done = true;
        return TemplateList() << file;
    }

    void write(const Template &t)
    {
        subjects.insert(t.file.get<QString>("Label"));
        bytes.append(t.bytes());
    }
};

BR_REGISTER(Gallery, statGallery)

/*!
 * \ingroup galleries
 * \brief Implements the FDDB detection format.
 * \author Josh Klontz \cite jklontz
 *
 * http://vis-www.cs.umass.edu/fddb/README.txt
 */
class FDDBGallery : public Gallery
{
    Q_OBJECT

    TemplateList readBlock(bool *done)
    {
        *done = true;
        QStringList lines = QtUtils::readLines(file);
        TemplateList templates;
        while (!lines.empty()) {
            const QString fileName = lines.takeFirst();
            int numDetects = lines.takeFirst().toInt();
            for (int i=0; i<numDetects; i++) {
                const QStringList detect = lines.takeFirst().split(' ');
                Template t(fileName);
                QList<QVariant> faceList; //to be consistent with slidingWindow
                if (detect.size() == 5) { //rectangle
                    faceList.append(QRectF(detect[0].toFloat(), detect[1].toFloat(), detect[2].toFloat(), detect[3].toFloat()));
                    t.file.set("Confidence", detect[4].toFloat());
                } else if (detect.size() == 6) { //ellipse
                    float x = detect[3].toFloat(),  
                          y = detect[4].toFloat(),
                          radius = detect[1].toFloat();
                    faceList.append(QRectF(x - radius,y - radius,radius * 2.0, radius * 2.0));
                    t.file.set("Confidence", detect[5].toFloat());
                } else {
                    qFatal("Unknown FDDB annotation format.");
                }
                t.file.set("Face", faceList);
                t.file.set("Label",QString("face"));
                templates.append(t);
            }
        }
        return templates;
    }

    void write(const Template &t)
    {
        (void) t;
        qFatal("Not implemented.");
    }

    void init()
    {
        //
    }
};

BR_REGISTER(Gallery, FDDBGallery)

/*!
 * \ingroup galleries
 * \brief Text format for associating anonymous landmarks with images.
 * \author Josh Klontz \cite jklontz
 *
 * \code
 * file_name:x1,y1,x2,y2,...,xn,yn
 * file_name:x1,y1,x2,y2,...,xn,yn
 * ...
 * file_name:x1,y1,x2,y2,...,xn,yn
 * \endcode
 */
class landmarksGallery : public Gallery
{
    Q_OBJECT

    TemplateList readBlock(bool *done)
    {
        *done = true;
        TemplateList templates;
        foreach (const QString &line, QtUtils::readLines(file)) {
            const QStringList words = line.split(':');
            if (words.size() != 2) qFatal("Expected exactly one ':' in: %s.", qPrintable(line));
            File file(words[0]);
            const QList<float> vals = QtUtils::toFloats(words[1].split(','));
            if (vals.size() % 2 != 0) qFatal("Expected an even number of comma-separated values.");
            QList<QPointF> points; points.reserve(vals.size()/2);
            for (int i=0; i<vals.size(); i+=2)
                points.append(QPointF(vals[i], vals[i+1]));
            file.setPoints(points);
            templates.append(file);
        }
        return templates;
    }

    void write(const Template &t)
    {
        (void) t;
        qFatal("Not implemented.");
    }

    void init()
    {
        //
    }
};

BR_REGISTER(Gallery, landmarksGallery)

#ifdef CVMATIO

using namespace cv;

class vbbGallery : public Gallery
{
    Q_OBJECT

    void init()
    {
        MatlabIO matio;
        QString filename = (Globals->path.isEmpty() ? "" : Globals->path + "/") + file.name;
        bool ok = matio.open(filename.toStdString(), "r");
        if (!ok) qFatal("Couldn't open the vbb file");

        vector<MatlabIOContainer> variables;
        variables = matio.read();
        matio.close();

        double vers = variables[1].data<Mat>().at<double>(0,0);
        if (vers != 1.4) qFatal("This is an old vbb version, we don't mess with that.");

        A = variables[0].data<vector<vector<MatlabIOContainer> > >().at(0);
        objLists = A.at(1).data<vector<MatlabIOContainer> >();

        // start at the first frame (duh!)
        currFrame = 0;
    }

    TemplateList readBlock(bool *done)
    {
        *done = false;
        Template rects(file);
        if (objLists[currFrame].typeEquals<vector<vector<MatlabIOContainer> > >()) {
            vector<vector<MatlabIOContainer> > bbs = objLists[currFrame].data<vector<vector<MatlabIOContainer> > >();
            for (unsigned int i=0; i<bbs.size(); i++) {
                vector<MatlabIOContainer> bb = bbs[i];
                Mat pos = bb[1].data<Mat>();
                double left = pos.at<double>(0,0);
                double top = pos.at<double>(0,1);
                double width = pos.at<double>(0,2);
                double height = pos.at<double>(0,3);
                rects.file.appendRect(QRectF(left, top, width, height));
            }
        }
        TemplateList tl;
        tl.append(rects);
        if (++currFrame == (int)objLists.size()) *done = true;
        return tl;
    }

    void write(const Template &t)
    {
        (void)t; qFatal("Not implemented");
    }

private:
    // this holds a bunch of stuff, maybe we'll use it all later
    vector<MatlabIOContainer> A;
    // this, a field in A, holds bounding boxes for each frame
    vector<MatlabIOContainer> objLists;
    int currFrame;
};

BR_REGISTER(Gallery, vbbGallery)

#endif

void FileGallery::init()
{
    f.setFileName(file);
    QtUtils::touchDir(f);
    if (!f.open(QFile::ReadWrite))
        qFatal("Failed to open %s for read/write.", qPrintable(file));
    fileSize = f.size();

    Gallery::init();
}

} // namespace br

#include "gallery.moc"
