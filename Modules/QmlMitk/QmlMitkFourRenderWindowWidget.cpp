
#include "QmlMitkFourRenderWindowWidget.h"

#include "mitkDisplayInteractor.h"

#include "usGetModuleContext.h"

#include <QtQuick>
#include <QTimer>

#include <stdexcept>

QmlMitkFourRenderWindowWidget::QmlMitkFourRenderWindowWidget(QQuickItem* parent)
: QQuickItem(parent)
, m_DataStorage(NULL)
, m_ChildrenContainer(NULL)
, m_RenderItemAxial(NULL)
, m_RenderItemSagittal(NULL)
, m_RenderItemFrontal(NULL)
, m_RenderItem3D(NULL)
{
  QQmlEngine engine(this);
  m_Component = new QQmlComponent(&engine,
                                  QUrl("qrc:///MITK/Modules/QmlMitk/QmlMitkFourRenderWindowWidget.qml"),
                                  QQmlComponent::PreferSynchronous);

  if (m_Component->status() != QQmlComponent::Ready)
  {
    QObject::connect(m_Component, SIGNAL(statusChanged(QQmlComponent::Status)),
                         this, SLOT(continueLoading()));
    QTimer::singleShot(100,this,SLOT(continueLoading()));
  } else {
    continueLoading();
  }
}

void QmlMitkFourRenderWindowWidget::continueLoading()
{
  if (m_Component->isError()) {
    qWarning() << m_Component->errors();
    return;
  }
  if (m_Component->status() != QQmlComponent::Ready) {
     qWarning() << "Qml component not ready";
     return;
   }
      
  m_ChildrenContainer = qobject_cast<QQuickItem*>( m_Component->create() );

  if (m_ChildrenContainer)
  {
    QQmlProperty::write(m_ChildrenContainer, "parent", QVariant::fromValue<QObject*>(this));
    QQmlEngine::setObjectOwnership(m_ChildrenContainer, QQmlEngine::CppOwnership);

    SetupWidget(m_ChildrenContainer);
  }
  else
  {
    throw std::logic_error("Initialization of QmlMitkFourRenderWindowWidget went dead wrong. Check code...");
  }
}

QmlMitkFourRenderWindowWidget::~QmlMitkFourRenderWindowWidget()
{
  if (m_ChildrenContainer)
  {
    m_ChildrenContainer->deleteLater();
  }
}

void QmlMitkFourRenderWindowWidget::SetupWidget( QQuickItem* parent )
{
  m_RenderItemAxial = parent->findChild<QmlMitkRenderWindowItem*>("mitkRenderItemAxial");
  m_RenderItemSagittal = parent->findChild<QmlMitkRenderWindowItem*>("mitkRenderItemSagittal");
  m_RenderItemFrontal = parent->findChild<QmlMitkRenderWindowItem*>("mitkRenderItemFrontal");
  m_RenderItem3D = parent->findChild<QmlMitkRenderWindowItem*>("mitkRenderItem3D");
  
  if (m_RenderItemAxial && m_RenderItemSagittal && m_RenderItemFrontal && m_RenderItem3D)
  {
    m_RenderItemAxial->GetRenderer()->SetMapperID( mitk::BaseRenderer::Standard2D );
    m_RenderItemAxial->GetRenderer()->GetSliceNavigationController()->SetDefaultViewDirection( mitk::SliceNavigationController::Axial );

    m_RenderItemSagittal->GetRenderer()->SetMapperID( mitk::BaseRenderer::Standard2D );
    m_RenderItemSagittal->GetRenderer()->GetSliceNavigationController()->SetDefaultViewDirection( mitk::SliceNavigationController::Sagittal );

    m_RenderItemFrontal->GetRenderer()->SetMapperID( mitk::BaseRenderer::Standard2D );
    m_RenderItemFrontal->GetRenderer()->GetSliceNavigationController()->SetDefaultViewDirection( mitk::SliceNavigationController::Frontal );
   
    m_RenderItem3D->GetRenderer()->SetMapperID( mitk::BaseRenderer::Standard3D );
    m_RenderItem3D->GetRenderer()->GetSliceNavigationController()->SetDefaultViewDirection( mitk::SliceNavigationController::Original );
  
    InitializeMoveZoomInteraction();

    /// @note -- Make sure to try to complete datastorage setup.
    /// This is dealing with qml component load times.
    this->SetDataStorage(this->m_DataStorage);
  }
}

void QmlMitkFourRenderWindowWidget::InitializeMoveZoomInteraction()
{
  static mitk::DisplayInteractor::Pointer m_ZoomScroller = mitk::DisplayInteractor::New();
  m_ZoomScroller->LoadStateMachine("DisplayInteraction.xml");
  m_ZoomScroller->SetEventConfig("DisplayConfigMITK.xml");

  us::ModuleContext* context = us::GetModuleContext();
  context->RegisterService<mitk::InteractionEventObserver>( m_ZoomScroller.GetPointer() );
}
 
  
void QmlMitkFourRenderWindowWidget::SetDataStorage( mitk::DataStorage::Pointer storage )
{
  if (m_ChildrenContainer == NULL) {
     m_DataStorage = storage;
     return;  /// @note The qml components are not loaded and ready yet.  Assume that SetupWidgets will handle things.
  }

  if (storage.IsNull()) {
     return;
  }

  m_DataStorage = storage;

  // TODO file bug: planes rendering 2D REQUIRES a parent node for all plane geometries! the mapper should just not care if it cannot find others!
  // TODO make this conditional, only add if not yet preset...
  mitk::DataNode::Pointer planesNodeParent = mitk::DataNode::New();
  m_DataStorage->Add( planesNodeParent );

  m_RenderItemAxial->SetPlaneNodeParent( planesNodeParent );
  m_RenderItemAxial->GetRenderer()->SetDataStorage( m_DataStorage );

  m_RenderItemSagittal->SetPlaneNodeParent( planesNodeParent );
  m_RenderItemSagittal->GetRenderer()->SetDataStorage( m_DataStorage );

  m_RenderItemFrontal->SetPlaneNodeParent( planesNodeParent );
  m_RenderItemFrontal->GetRenderer()->SetDataStorage( m_DataStorage );

  m_RenderItem3D->SetPlaneNodeParent( planesNodeParent );
  m_RenderItem3D->GetRenderer()->SetDataStorage( m_DataStorage );
}

QList<mitk::BaseRenderer *> QmlMitkFourRenderWindowWidget::GetRenderers()
{
   if (!m_RenderItemAxial || !m_RenderItemSagittal || !m_RenderItemFrontal)
      return QList<mitk::BaseRenderer*>();

   QList<mitk::BaseRenderer*> renderers;
   renderers.append(m_RenderItemAxial->GetRenderer());
   renderers.append(m_RenderItemSagittal->GetRenderer());
   renderers.append(m_RenderItemFrontal->GetRenderer());
   return renderers;
}

