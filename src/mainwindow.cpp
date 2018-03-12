#include <QVBoxLayout>
#include <QPushButton>
#include <QButtonGroup>
#include <QGridLayout>
#include <QToolBar>
#include <QThread>
#include <assert.h>
#include "mainwindow.h"
#include "skeletoneditwidget.h"
#include "meshlite.h"
#include "skeletontomesh.h"

MainWindow::MainWindow() :
    m_skeletonToMesh(NULL),
    m_skeletonDirty(false)
{
    QPushButton *skeletonButton = new QPushButton("Skeleton");
    QPushButton *motionButton = new QPushButton("Motion");
    QPushButton *modelButton = new QPushButton("Model");
    
    QButtonGroup *pageButtonGroup = new QButtonGroup;
    pageButtonGroup->addButton(skeletonButton);
    pageButtonGroup->addButton(motionButton);
    pageButtonGroup->addButton(modelButton);
    
    skeletonButton->setCheckable(true);
    motionButton->setCheckable(true);
    modelButton->setCheckable(true);
    
    pageButtonGroup->setExclusive(true);
    
    skeletonButton->setChecked(true);
    
    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->addStretch();
    topLayout->addWidget(skeletonButton);
    topLayout->addWidget(motionButton);
    topLayout->addWidget(modelButton);
    topLayout->addStretch();
    
    skeletonButton->adjustSize();
    motionButton->adjustSize();
    modelButton->adjustSize();
    
    m_skeletonEditWidget = new SkeletonEditWidget;
    
    m_modelingWidget = new ModelingWidget;
    m_modelingWidget->setFixedSize(128, 128);
    
    QPushButton *changeTurnaroundButton = new QPushButton("Change turnaround..");
    
    QVBoxLayout *rightLayout = new QVBoxLayout;
    rightLayout->addWidget(m_modelingWidget);
    rightLayout->addSpacing(10);
    rightLayout->addWidget(changeTurnaroundButton);
    rightLayout->addStretch();
    
    QToolBar *toolbar = new QToolBar;
    toolbar->setOrientation(Qt::Vertical);
    QAction *addAction = new QAction(tr("Add"), this);
    QAction *selectAction = new QAction(tr("Select"), this);
    toolbar->addAction(addAction);
    toolbar->addAction(selectAction);
    
    QVBoxLayout *leftLayout = new QVBoxLayout;
    leftLayout->addWidget(toolbar);
    leftLayout->addStretch();
    
    QHBoxLayout *middleLayout = new QHBoxLayout;
    middleLayout->addLayout(leftLayout);
    middleLayout->addWidget(m_skeletonEditWidget);
    middleLayout->addLayout(rightLayout);
    
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(topLayout);
    mainLayout->addSpacing(20);
    mainLayout->addLayout(middleLayout);
    
    QWidget *centralWidget = new QWidget;
    centralWidget->setLayout(mainLayout);
    
    setCentralWidget(centralWidget);
    setWindowTitle(tr("Dust 3D"));
    
    bool connectResult;
    
    connectResult = connect(addAction, SIGNAL(triggered(bool)), m_skeletonEditWidget->graphicsView(), SLOT(turnOnAddNodeMode()));
    assert(connectResult);
    
    connectResult = connectResult = connect(selectAction, SIGNAL(triggered(bool)), m_skeletonEditWidget->graphicsView(), SLOT(turnOffAddNodeMode()));
    assert(connectResult);
    
    connectResult = connect(m_skeletonEditWidget->graphicsView(), SIGNAL(nodesChanged()), this, SLOT(skeletonChanged()));
    assert(connectResult);
}

void MainWindow::meshReady()
{
    m_modelingWidget->updateMesh(m_skeletonToMesh->takeResultMesh());
    delete m_skeletonToMesh;
    m_skeletonToMesh = NULL;
    if (m_skeletonDirty) {
        skeletonChanged();
    }
}

void MainWindow::skeletonChanged()
{
    if (m_skeletonToMesh) {
        m_skeletonDirty = true;
        return;
    }
    
    m_skeletonDirty = false;
    
    QThread *thread = new QThread;
    m_skeletonToMesh = new SkeletonToMesh(m_skeletonEditWidget->graphicsView());
    m_skeletonToMesh->moveToThread(thread);
    connect(thread, SIGNAL(started()), m_skeletonToMesh, SLOT(process()));
    connect(m_skeletonToMesh, SIGNAL(finished()), this, SLOT(meshReady()));
    connect(m_skeletonToMesh, SIGNAL(finished()), thread, SLOT(quit()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    thread->start();
}
