#ifndef BR_CLASSIFIER_H
#define BR_CLASSIFIER_H

#include <QLabel>
#include <QWidget>
#include <QString>
#include <openbr/openbr_plugin.h>

namespace br
{

class BR_EXPORT Classifier : public QLabel
{
    Q_OBJECT
    QString algorithm;

public:
    explicit Classifier(QWidget *parent = 0);
    void setAlgorithm(const QString &algorithm);

public slots:
    void classify(const br::File &file);

private slots:
    void setClassification(const QString &key, const QString &value);

private:
    void _classify(br::File file);

signals:
    void newClassification(QString key, QString value);
};

} // namespace br

#endif // BR_CLASSIFIER_H
