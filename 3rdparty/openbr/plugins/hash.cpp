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
#include "openbr_internal.h"
#include "openbr/core/qtutils.h"

using namespace cv;

namespace br
{

/*!
 * \ingroup transforms
 * \brief Wraps QCryptographicHash
 * \author Josh Klontz \cite jklontz
 */
class CryptographicHashTransform : public UntrainableTransform
{
    Q_OBJECT
    Q_ENUMS(Algorithm)
    Q_PROPERTY(Algorithm algorithm READ get_algorithm WRITE set_algorithm RESET reset_algorithm STORED false)

public:
    /*!< */
    enum Algorithm { Md4 = QCryptographicHash::Md4,
                     Md5 = QCryptographicHash::Md5,
                     Sha1 = QCryptographicHash::Sha1 };

private:
    BR_PROPERTY(Algorithm, algorithm, Md5)

    void project(const Template &src, Template &dst) const
    {
        const cv::Mat &m = src;
        QByteArray data((const char *)m.data, int(m.total()*m.elemSize()));
        QByteArray hash = QCryptographicHash::hash(data, (QCryptographicHash::Algorithm)algorithm);
        cv::Mat n(1, hash.size(), CV_8UC1);
        memcpy(n.data, hash.data(), hash.size());
        dst = Template(src.file, n);
    }
};

BR_REGISTER(Transform, CryptographicHashTransform)

/*!
 * \ingroup transforms
 * \brief Kernel hash
 * \author Josh Klontz \cite jklontz
 */
class KernelHashTransform : public UntrainableTransform
{
    Q_OBJECT
    Q_PROPERTY(uchar dimsIn READ get_dimsIn WRITE set_dimsIn RESET reset_dimsIn STORED false)
    Q_PROPERTY(uchar dimsOut READ get_dimsOut WRITE set_dimsOut RESET reset_dimsOut STORED false)
    BR_PROPERTY(uchar, dimsIn, 8)
    BR_PROPERTY(uchar, dimsOut, 7)

    void project(const Template &src, Template &dst) const
    {
        if (src.m().type() != CV_8UC1)
            qFatal("Expected 8UC1 input.");

        dst = Mat::zeros(src.m().rows, src.m().cols, CV_8UC1);
        const uchar *srcData = src.m().data;
        uchar *dstData = dst.m().data;
        const int step = src.m().cols;
        for (int i=0; i<src.m().rows-1; i++)
            for (int j=0; j<src.m().cols-1; j++) {
                dstData[i*step+j] = (uint(pow(float(dimsIn),1.f))*srcData[i    *step+j]
                                   + uint(pow(float(dimsIn),2.f))*srcData[(i+1)*step+j]
                                   + uint(pow(float(dimsIn),0.f))*srcData[i    *step+(j+1)]
                                   /*+ uint(pow(float(dimsIn),0.f))*srcData[(i+1)*step+(j+1)]*/) % dimsOut;
            }
    }
};

BR_REGISTER(Transform, KernelHashTransform)

} // namespace br

#include "hash.moc"
