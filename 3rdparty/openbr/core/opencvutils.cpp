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

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgproc/imgproc_c.h>
#include <openbr/openbr_plugin.h>

#include "opencvutils.h"
#include "qtutils.h"

using namespace cv;
using namespace std;

int OpenCVUtils::getFourcc()
{
    int fourcc = CV_FOURCC('x','2','6','4');
    QVariant recovered_variant = br::Globals->property("fourcc");

    if (!recovered_variant.isNull()) {
        QString recovered_string = recovered_variant.toString();
        if (recovered_string.length() == 4) {
            fourcc = CV_FOURCC(recovered_string[0].toLatin1(),
                               recovered_string[1].toLatin1(),
                               recovered_string[2].toLatin1(),
                               recovered_string[3].toLatin1());
        }
        else if (recovered_string.compare("-1")) fourcc = -1;
    }
    return fourcc;
}

void OpenCVUtils::saveImage(const Mat &src, const QString &file)
{
    if (file.isEmpty()) return;

    if (!src.data) {
        qWarning("OpenCVUtils::saveImage null image.");
        return;
    }

    QtUtils::touchDir(QFileInfo(file).dir());

    Mat draw;
    cvtUChar(src, draw);
    bool success = imwrite(file.toStdString(), draw); if (!success) qFatal("Failed to save %s", qPrintable(file));
}

void OpenCVUtils::showImage(const Mat &src, const QString &window, bool waitKey)
{
    if (!src.data) {
        qWarning("OpenCVUtils::showImage null image.");
        return;
    }

    Mat draw;
    cvtUChar(src, draw);
    imshow(window.toStdString(), draw);
    cv::waitKey(waitKey ? -1 : 1);
}

void OpenCVUtils::cvtGray(const Mat &src, Mat &dst)
{
    if      (src.channels() == 3) cvtColor(src, dst, CV_BGR2GRAY);
    else if (src.channels() == 1) dst = src;
    else                          qFatal("Invalid channel count");
}

void OpenCVUtils::cvtUChar(const Mat &src, Mat &dst)
{
    if (src.depth() == CV_8U) {
        dst = src;
        return;
    }

    double globalMin = std::numeric_limits<double>::max();
    double globalMax = -std::numeric_limits<double>::max();

    vector<Mat> mv;
    split(src, mv);
    for (size_t i=0; i<mv.size(); i++) {
        double min, max;
        minMaxLoc(mv[i], &min, &max);
        globalMin = std::min(globalMin, min);
        globalMax = std::max(globalMax, max);
    }
    assert(globalMax >= globalMin);

    double range = globalMax - globalMin;
    if (range != 0) {
        double scale = 255 / range;
        convertScaleAbs(src, dst, scale, -(globalMin * scale));
    } else {
        // Monochromatic
        dst = Mat(src.size(), CV_8UC1, Scalar((globalMin+globalMax)/2));
    }
}

Mat OpenCVUtils::toMat(const QList<float> &src, int rows)
{
    if (rows == -1) rows = src.size();
    int columns = src.isEmpty() ? 0 : src.size() / rows;
    if (rows*columns != src.size()) qFatal("Invalid matrix size.");
    Mat dst(rows, columns, CV_32FC1);
    for (int i=0; i<src.size(); i++)
        dst.at<float>(i/columns,i%columns) = src[i];
    return dst;
}

Mat OpenCVUtils::toMat(const QList<QList<float> > &srcs, int rows)
{
    QList<float> flat;
    foreach (const QList<float> &src, srcs)
        flat.append(src);
    return toMat(flat, rows);
}

Mat OpenCVUtils::toMat(const QList<int> &src, int rows)
{
    if (rows == -1) rows = src.size();
    int columns = src.isEmpty() ? 0 : src.size() / rows;
    if (rows*columns != src.size()) qFatal("Invalid matrix size.");
    Mat dst(rows, columns, CV_32FC1);
    for (int i=0; i<src.size(); i++)
        dst.at<float>(i/columns,i%columns) = src[i];
    return dst;
}

Mat OpenCVUtils::toMat(const QList<Mat> &src)
{
    if (src.isEmpty()) return Mat();

    int rows = src.size();
    size_t total = src.first().total();
    int type = src.first().type();
    Mat dst(rows, total, type);

    for (int i=0; i<rows; i++) {
        const Mat &m = src[i];
        if ((m.total() != total) || (m.type() != type) || !m.isContinuous())
            qFatal("Invalid matrix.");
        memcpy(dst.ptr(i), m.ptr(), total * src.first().elemSize());
    }
    return dst;
}

Mat OpenCVUtils::toMatByRow(const QList<Mat> &src)
{
    if (src.isEmpty()) return Mat();

    int rows = 0; foreach (const Mat &m, src) rows += m.rows;
    int cols = src.first().cols;
    if (cols == 0) qFatal("Columnless matrix!");
    int type = src.first().type();
    Mat dst(rows, cols, type);

    int row = 0;
    foreach (const Mat &m, src) {
        if ((m.cols != cols) || (m.type() != type) || (!m.isContinuous()))
            qFatal("Invalid matrix.");
        memcpy(dst.ptr(row), m.ptr(), m.rows*m.cols*m.elemSize());
        row += m.rows;
    }
    return dst;
}

QString OpenCVUtils::depthToString(const Mat &m)
{
    switch (m.depth()) {
      case CV_8U:  return "8U";
      case CV_8S:  return "8S";
      case CV_16U: return "16U";
      case CV_16S: return "16S";
      case CV_32S: return "32S";
      case CV_32F: return "32F";
      case CV_64F: return "64F";
      default:     qFatal("Unknown matrix depth!");
    }
    return "?";
}

QString OpenCVUtils::typeToString(const cv::Mat &m)
{
    return depthToString(m) + "C" + QString::number(m.channels());
}

QString OpenCVUtils::elemToString(const Mat &m, int r, int c)
{
    assert(m.channels() == 1);
    switch (m.depth()) {
      case CV_8U:  return QString::number(m.at<quint8>(r,c));
      case CV_8S:  return QString::number(m.at<qint8>(r,c));
      case CV_16U: return QString::number(m.at<quint16>(r,c));
      case CV_16S: return QString::number(m.at<qint16>(r,c));
      case CV_32S: return QString::number(m.at<qint32>(r,c));
      case CV_32F: return QString::number(m.at<float>(r,c));
      case CV_64F: return QString::number(m.at<double>(r,c));
      default:     qFatal("Unknown matrix depth");
    }
    return "?";
}

QString OpenCVUtils::matrixToString(const Mat &m)
{
    QString result;
    vector<Mat> mv;
    split(m, mv);
    if (m.rows > 1) result += "{ ";
    for (int r=0; r<m.rows; r++) {
        if ((m.rows > 1) && (r > 0)) result += "  ";
        if (m.cols > 1) result += "[";
        for (int c=0; c<m.cols; c++) {
            if (mv.size() > 1) result += "(";
            for (unsigned int i=0; i<mv.size()-1; i++)
                result += OpenCVUtils::elemToString(mv[i], r, c) + ", ";
            result += OpenCVUtils::elemToString(mv[mv.size()-1], r, c);
            if (mv.size() > 1) result += ")";
            if (c < m.cols - 1) result += ", ";
        }
        if (m.cols > 1) result += "]";
        if (r < m.rows-1) result += "\n";
    }
    if (m.rows > 1) result += " }";
    return result;
}

QStringList OpenCVUtils::matrixToStringList(const Mat &m)
{
    QStringList results;
    vector<Mat> mv;
    split(m, mv);
    foreach (const Mat &mc, mv)
        for (int i=0; i<mc.rows; i++)
            for (int j=0; j<mc.cols; j++)
                results.append(elemToString(mc, i, j));
    return results;
}

Point2f OpenCVUtils::toPoint(const QPointF &qPoint)
{
    return Point2f(qPoint.x(), qPoint.y());
}

QPointF OpenCVUtils::fromPoint(const Point2f &cvPoint)
{
    return QPointF(cvPoint.x, cvPoint.y);
}

QList<Point2f> OpenCVUtils::toPoints(const QList<QPointF> &qPoints)
{
    QList<Point2f> cvPoints; cvPoints.reserve(qPoints.size());
    foreach (const QPointF &qPoint, qPoints)
        cvPoints.append(toPoint(qPoint));
    return cvPoints;
}

QList<QPointF> OpenCVUtils::fromPoints(const QList<Point2f> &cvPoints)
{
    QList<QPointF> qPoints; qPoints.reserve(cvPoints.size());
    foreach (const Point2f &cvPoint, cvPoints)
        qPoints.append(fromPoint(cvPoint));
    return qPoints;
}

Rect OpenCVUtils::toRect(const QRectF &qRect)
{
    return Rect(qRect.x(), qRect.y(), qRect.width(), qRect.height());
}

QRectF OpenCVUtils::fromRect(const Rect &cvRect)
{
    return QRectF(cvRect.x, cvRect.y, cvRect.width, cvRect.height);
}

QList<Rect> OpenCVUtils::toRects(const QList<QRectF> &qRects)
{
    QList<Rect> cvRects; cvRects.reserve(qRects.size());
    foreach (const QRectF &qRect, qRects)
        cvRects.append(toRect(qRect));
    return cvRects;
}

QList<QRectF> OpenCVUtils::fromRects(const QList<Rect> &cvRects)
{
    QList<QRectF> qRects; qRects.reserve(cvRects.size());
    foreach (const Rect &cvRect, cvRects)
        qRects.append(fromRect(cvRect));
    return qRects;
}

float OpenCVUtils::overlap(const Rect &rect1, const Rect &rect2) {
    float left = max(rect1.x, rect2.x);
    float top = max(rect1.y, rect2.y);
    float right = min(rect1.x + rect1.width, rect2.x + rect2.width);
    float bottom = min(rect1.y + rect1.height, rect2.y + rect2.height);

    float overlap = (right - left + 1) * (top - bottom + 1) / max(rect1.width * rect1.height, rect2.width * rect2.height);
    if (overlap < 0)
        return 0;
    return overlap;
}

float OpenCVUtils::overlap(const QRectF &rect1, const QRectF &rect2) {
    float left = max(rect1.x(), rect2.x());
    float top = max(rect1.y(), rect2.y());
    float right = min(rect1.x() + rect1.width(), rect2.x() + rect2.width());
    float bottom = min(rect1.y() + rect1.height(), rect2.y() + rect2.height());

    float overlap = (right - left + 1) * (top - bottom + 1) / max(rect1.width() * rect1.height(), rect2.width() * rect2.height());
    if (overlap < 0)
        return 0;
    return overlap;
}

bool OpenCVUtils::overlaps(const QList<Rect> &posRects, const Rect &negRect, double overlap)
{
    foreach (const Rect &posRect, posRects) {
        Rect intersect = negRect & posRect;
        if (intersect.area() > overlap*posRect.area())
            return true;
    }
    return false;
}

QDataStream &operator<<(QDataStream &stream, const Mat &m)
{
    // Write header
    int rows = m.rows;
    int cols = m.cols;
    int type = m.type();
    stream << rows << cols << type;

    // Write data
    int len = rows * cols * m.elemSize();
    stream << len;
    if (len > 0) {
        if (!m.isContinuous()) qFatal("Can't serialize non-continuous matrices.");
        int written = stream.writeRawData((const char*)m.data, len);
        if (written != len) qFatal("Mat serialization failure, expected: %d bytes, wrote: %d bytes.", len, written);
    }
    return stream;
}

QDataStream &operator>>(QDataStream &stream, Mat &m)
{
    // Read header
    int rows, cols, type;
    stream >> rows >> cols >> type;
    m.create(rows, cols, type);

    int len;
    stream >> len;
    char *data = (char*) m.data;

    // In certain circumstances, like reading from stdin or sockets, we may not
    // be given all the data we need at once because it isn't available yet.
    // So we loop until it we get it.
    while (len > 0) {
        const int read = stream.readRawData(data, len);
        if (read == -1) qFatal("Mat deserialization failure, exptected %d more bytes.", len);
        data += read;
        len -= read;
    }
    return stream;
}

QDebug operator<<(QDebug dbg, const Mat &m)
{
    dbg.nospace() << OpenCVUtils::matrixToString(m);
    return dbg.space();
}

QDebug operator<<(QDebug dbg, const Point &p)
{
    dbg.nospace() << "(" << p.x << ", " << p.y << ")";
    return dbg.space();
}

QDebug operator<<(QDebug dbg, const Rect &r)
{
    dbg.nospace() << "(" << r.x << ", " << r.y << "," << r.width << "," << r.height << ")";
    return dbg.space();
}

QDataStream &operator<<(QDataStream &stream, const Rect &r)
{
    return stream << r.x << r.y << r.width << r.height;
}

QDataStream &operator>>(QDataStream &stream, Rect &r)
{
    return stream >> r.x >> r.y >> r.width >> r.height;
}

QDataStream &operator<<(QDataStream &stream, const Size &s)
{
    return stream << s.width << s.height;
}

QDataStream &operator>>(QDataStream &stream, Size &s)
{
    return stream >> s.width >> s.height;
}
