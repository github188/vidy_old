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

#include <opencv2/imgproc/imgproc.hpp>
#include "openbr_internal.h"

#include "openbr/core/opencvutils.h"

using namespace cv;

namespace br
{

/*!
 * \ingroup transforms
 * \brief Crops about the specified region of interest.
 * \author Josh Klontz \cite jklontz
 */
class CropTransform : public UntrainableTransform
{
    Q_OBJECT
    Q_PROPERTY(int x READ get_x WRITE set_x RESET reset_x STORED false)
    Q_PROPERTY(int y READ get_y WRITE set_y RESET reset_y STORED false)
    Q_PROPERTY(int width READ get_width WRITE set_width RESET reset_width STORED false)
    Q_PROPERTY(int height READ get_height WRITE set_height RESET reset_height STORED false)
    BR_PROPERTY(int, x, 0)
    BR_PROPERTY(int, y, 0)
    BR_PROPERTY(int, width, -1)
    BR_PROPERTY(int, height, -1)

    void project(const Template &src, Template &dst) const
    {
        dst = Mat(src, Rect(x, y, width < 1 ? src.m().cols-x-abs(width) : width, height < 1 ? src.m().rows-y-abs(height) : height));
    }
};

BR_REGISTER(Transform, CropTransform)

/*!
 * \ingroup transforms
 * \brief Crops the rectangular regions of interest.
 * \author Josh Klontz \cite jklontz
 */
class ROITransform : public UntrainableTransform
{
    Q_OBJECT
    Q_PROPERTY(QString propName READ get_propName WRITE set_propName RESET reset_propName STORED false)
    BR_PROPERTY(QString, propName, "")

    void project(const Template &src, Template &dst) const
    {
        if (!propName.isEmpty()) {
            QRectF rect = src.file.get<QRectF>(propName);
            dst += src.m()(OpenCVUtils::toRect(rect));
        } else if (!src.file.rects().empty()) {
            foreach (const QRectF &rect, src.file.rects())
                dst += src.m()(OpenCVUtils::toRect(rect));
        } else if (src.file.contains(QStringList() << "X" << "Y" << "Width" << "Height")) {
            dst += src.m()(Rect(src.file.get<int>("X"),
                                src.file.get<int>("Y"),
                                src.file.get<int>("Width"),
                                src.file.get<int>("Height")));
        } else {
            dst = src;
            if (Globals->verbose)
                qWarning("No rects present in file.");
        }
    }
};

BR_REGISTER(Transform, ROITransform)

/*!
 * \ingroup transforms
 * \brief Crops the rectangular regions of interest from given points and sizes.
 * \author Austin Blanton \cite imaus10
 */
class ROIFromPtsTransform : public UntrainableTransform
{
    Q_OBJECT
    Q_PROPERTY(int width READ get_width WRITE set_width RESET reset_width STORED false)
    Q_PROPERTY(int height READ get_height WRITE set_height RESET reset_height STORED false)
    BR_PROPERTY(int, width, 1)
    BR_PROPERTY(int, height, 1)

    void project(const Template &src, Template &dst) const
    {
        foreach (const QPointF &pt, src.file.points()) {
            int x = pt.x() - (width/2);
            int y = pt.y() - (height/2);
            dst += src.m()(Rect(x, y, width, height));
        }
    }
};

BR_REGISTER(Transform, ROIFromPtsTransform)

/*!
 * \ingroup transforms
 * \brief Resize the template
 * \author Josh Klontz \cite jklontz
 * \note Method: Area should be used for shrinking an image, Cubic for slow but accurate enlargment, Bilin for fast enlargement.
 * \param preserveAspect If true, the image will be sized per specification, but
 * 	a border will be applied to preserve aspect ratio.
 */
class ResizeTransform : public UntrainableTransform
{
    Q_OBJECT
    Q_ENUMS(Method)

public:
    /*!< */
    enum Method { Near = INTER_NEAREST,
                  Area = INTER_AREA,
                  Bilin = INTER_LINEAR,
                  Cubic = INTER_CUBIC,
                  Lanczo = INTER_LANCZOS4};

private:
    Q_PROPERTY(int rows READ get_rows WRITE set_rows RESET reset_rows STORED false)
    Q_PROPERTY(int columns READ get_columns WRITE set_columns RESET reset_columns STORED false)
    Q_PROPERTY(Method method READ get_method WRITE set_method RESET reset_method STORED false)
    Q_PROPERTY(bool preserveAspect READ get_preserveAspect WRITE set_preserveAspect RESET reset_preserveAspect STORED false)
    BR_PROPERTY(int, rows, -1)
    BR_PROPERTY(int, columns, -1)
    BR_PROPERTY(Method, method, Bilin)
    BR_PROPERTY(bool, preserveAspect, false)

    void project(const Template &src, Template &dst) const
    {
        if (!preserveAspect)
            resize(src, dst, Size((columns == -1) ? src.m().cols*rows/src.m().rows : columns, rows), 0, 0, method);
        else {
            float inRatio = (float) src.m().rows / src.m().cols;
            float outRatio = (float) rows / columns;
            dst = Mat::zeros(rows, columns, src.m().type());
            if (outRatio > inRatio) {
                float heightAR = src.m().rows * inRatio / outRatio;
                Mat buffer;
                resize(src, buffer, Size(columns, heightAR), 0, 0, method);
                buffer.copyTo(dst.m()(Rect(0, (rows - heightAR) / 2, columns, heightAR)));
            } else {
                float widthAR = src.m().cols / inRatio * outRatio;
                Mat buffer;
                resize(src, buffer, Size(widthAR, rows), 0, 0, method);
                buffer.copyTo(dst.m()(Rect((columns - widthAR) / 2, 0, widthAR, rows)));
            }
        }
    }
};

BR_REGISTER(Transform, ResizeTransform)

/*!
 * \ingroup transforms
 * \brief Limit the size of the template
 * \author Josh Klontz \cite jklontz
 */
class LimitSizeTransform : public UntrainableTransform
{
    Q_OBJECT
    Q_PROPERTY(int max READ get_max WRITE set_max RESET reset_max STORED false)
    BR_PROPERTY(int, max, -1)

    void project(const Template &src, Template &dst) const
    {
        const Mat &m = src;
        if (m.rows > m.cols)
            if (m.rows > max) resize(m, dst, Size(std::max(1, m.cols * max / m.rows), max));
            else              dst = m;
        else
            if (m.cols > max) resize(m, dst, Size(max, std::max(1, m.rows * max / m.cols)));
            else              dst = m;
    }
};

BR_REGISTER(Transform, LimitSizeTransform)

/*!
 * \ingroup transforms
 * \brief Enforce a multiple of \em n columns.
 * \author Josh Klontz \cite jklontz
 */
class DivTransform : public UntrainableTransform
{
    Q_OBJECT
    Q_PROPERTY(int n READ get_n WRITE set_n RESET reset_n STORED false)
    BR_PROPERTY(int, n, 1)

    void project(const Template &src, Template &dst) const
    {
        dst = Mat(src, Rect(0,0,n*(src.m().cols/n),src.m().rows));
    }
};

BR_REGISTER(Transform, DivTransform)

/*!
 * \ingroup transforms
 * \brief Crop out black borders
 * \author Josh Klontz \cite jklontz
 */
class CropBlackTransform : public UntrainableTransform
{
    Q_OBJECT

    void project(const Template &src, Template &dst) const
    {
        Mat gray;
        OpenCVUtils::cvtGray(src, gray);

        int xStart = 0;
        while (xStart < gray.cols) {
            if (mean(gray.col(xStart))[0] >= 1) break;
            xStart++;
        }

        int xEnd = gray.cols - 1;
        while (xEnd >= 0) {
            if (mean(gray.col(xEnd))[0] >= 1) break;
            xEnd--;
        }

        int yStart = 0;
        while (yStart < gray.rows) {
            if (mean(gray.col(yStart))[0] >= 1) break;
            yStart++;
        }

        int yEnd = gray.rows - 1;
        while (yEnd >= 0) {
            if (mean(gray.col(yEnd))[0] >= 1) break;
            yEnd--;
        }

        dst = src.m()(Rect(xStart, yStart, xEnd-xStart, yEnd-yStart));
    }
};

BR_REGISTER(Transform, CropBlackTransform)

/*!
 * \ingroup transforms
 * \brief Divide the matrix into 4 smaller matricies of equal size.
 * \author Josh Klontz \cite jklontz
 */
class SubdivideTransform : public UntrainableTransform
{
    Q_OBJECT

    void project(const Template &src, Template &dst) const
    {
        const Mat &m = src;
        const int subrows = m.rows/2;
        const int subcolumns = m.cols/2;
        dst.append(Mat(m,Rect(0,          0, subcolumns, subrows)).clone());
        dst.append(Mat(m,Rect(subcolumns, 0, subcolumns, subrows)).clone());
        dst.append(Mat(m,Rect(0,          subrows, subcolumns, subrows)).clone());
        dst.append(Mat(m,Rect(subcolumns, subrows, subcolumns, subrows)).clone());
    }
};

BR_REGISTER(Transform, SubdivideTransform)

/*!
 * \ingroup transforms
 * \brief Trim the image so the width and the height are the same size.
 * \author Josh Klontz \cite jklontz
 */
class CropSquareTransform : public UntrainableTransform
{
    Q_OBJECT

    void project(const Template &src, Template &dst) const
    {
        const Mat &m = src;
        const int newSize = min(m.rows, m.cols);
        dst = Mat(m, Rect((m.cols-newSize)/2, (m.rows-newSize)/2, newSize, newSize));
    }
};

BR_REGISTER(Transform, CropSquareTransform)

} // namespace br

#include "crop.moc"
