#include "pageflipwidget.h"

using namespace br;

PageFlipWidget::PageFlipWidget(QWidget *parent) :
    QWidget(parent)
{
    firstPage = new QPushButton(this);
    firstPage->setIcon(QIcon(":/arrow-first.png"));
    firstPage->setMaximumWidth(30);
    firstPage->setToolTip("Go to first page");

    connect(firstPage, SIGNAL(clicked()), this, SIGNAL(first()));

    previousPage = new QPushButton(this);
    previousPage->setIcon(QIcon(":/arrow-left.png"));
    previousPage->setMaximumWidth(30);
    previousPage->setToolTip("Go to previous page");

    connect(previousPage, SIGNAL(clicked()), this, SIGNAL(previous()));

    nextPage = new QPushButton(this);
    nextPage->setIcon(QIcon(":/arrow-right.png"));
    nextPage->setMaximumWidth(30);
    nextPage->setToolTip("Go to next page");

    connect(nextPage, SIGNAL(clicked()), this, SIGNAL(next()));

    lastPage = new QPushButton(this);
    lastPage->setIcon(QIcon(":/arrow-last.png"));
    lastPage->setMaximumWidth(30);
    lastPage->setToolTip("Go to last page");

    connect(lastPage, SIGNAL(clicked()), this, SIGNAL(last()));

    boxLayout.addWidget(firstPage);
    boxLayout.addWidget(previousPage);
    boxLayout.addWidget(nextPage);
    boxLayout.addWidget(lastPage);

    setLayout(&boxLayout);
}
