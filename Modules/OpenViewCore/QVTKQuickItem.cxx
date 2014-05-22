/*========================================================================
  OpenView -- http://openview.kitware.com

  Copyright 2012 Kitware, Inc.

  Licensed under the BSD license. See LICENSE file for details.
 ========================================================================*/
#include "QVTKQuickItem.h"

#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QOpenGLShaderProgram>
#include <QQuickWindow>
#include <QThread>
#include <QSGSimpleRectNode>

#include "QVTKInteractor.h"
#include "QVTKInteractorAdapter.h"
#include "vtkGenericOpenGLRenderWindow.h"
#include "vtkEventQtSlotConnect.h"
#include "vtkgl.h"
#include "vtkOpenGLExtensionManager.h"
#include "vtkRenderer.h"
#include "vtkRendererCollection.h"

#include "vtkCubeSource.h"
#include "vtkPolyDataMapper.h"
#include "vtkProperty.h"


#include <iostream>

#include <QQuickFramebufferObject>
#include <QOpenGLFramebufferObject>

#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkObjectFactory.h>

#include <vtkRendererCollection.h>
#include <vtkCamera.h>

class QVTKFramebufferObjectRenderer;

class vtkInternalOpenGLRenderWindow : public vtkGenericOpenGLRenderWindow
{
public:
  static vtkInternalOpenGLRenderWindow* New();
  vtkTypeMacro(vtkInternalOpenGLRenderWindow, vtkGenericOpenGLRenderWindow)

  virtual void OpenGLInitState()
  {
     Superclass::OpenGLInitState();
     vtkgl::UseProgram(0); // Shouldn't Superclass::OpenGLInitState() handle this?

     // We get depth buffer fighting between the VTK-drawn scene and the
     // QSG-drawn background if we don't disable the depth test here.
     glDisable(GL_DEPTH_TEST);

     // Disabling blending doesn't seem crucial but when analyzing the OpenGL
     // state at the start of VTK rendering this was a differnce between the
     // FBO approach and the old draw-over-QSG approach
     glDisable(GL_BLEND);
  }

  // Override to use deferred rendering - Tell the QSG that we need to
  // be rendered which will then, at the appropriate time, call
  // InternalRender to do the actual OpenGL rendering.
  virtual void Render();

  // Do the actual OpenGL rendering
  void InternalRender()
  {
     Superclass::Render();
  }

  // Provides a convenient way to set the protected FBO ivars from an existing
  // FBO that was created and owned by Qt's FBO abstraction class
  // QOpenGLFramebufferObject
  void SetFramebufferObject(QOpenGLFramebufferObject *fbo);

  QVTKFramebufferObjectRenderer *QtParentRenderer;

protected:
  vtkInternalOpenGLRenderWindow() :
     QtParentRenderer(0)
  {
  }

  ~vtkInternalOpenGLRenderWindow()
  {
     // Prevent superclass destructors from destroying the framebuffer object.
     // QOpenGLFramebufferObject owns the FBO and manages it's lifecyle.
     this->OffScreenRendering = 0;
  }
};
vtkStandardNewMacro(vtkInternalOpenGLRenderWindow);


class QVTKFramebufferObjectRenderer : public QQuickFramebufferObject::Renderer
{
public:
   QVTKFramebufferObjectRenderer(vtkInternalOpenGLRenderWindow *rw) :
      m_vtkRenderWindow(rw),
      m_neverRendered(true),
      m_readyToRender(false)
   {
      m_vtkRenderWindow->Register(NULL);
      m_vtkRenderWindow->QtParentRenderer = this;
   }

   ~QVTKFramebufferObjectRenderer()
   {
      m_vtkRenderWindow->QtParentRenderer = 0;
      m_vtkRenderWindow->Delete();
   }

   // Called from the GUI thread
   virtual void synchronize(QQuickFramebufferObject * item)
   {
     m_vtkQuickItem = static_cast<QVTKQuickItem*>(item);

     // the first synchronize call - right before the the framebufferObject
     // is created for the first time
     if (m_neverRendered)
     {
       m_neverRendered = false;
       m_vtkQuickItem->init();
     }
     m_readyToRender = m_vtkQuickItem->prepareForRender();
   }

   virtual void render()
   {
     if (!m_readyToRender)
     {
       return;
     }

     // lock/unlock the m_viewLock *inside* the prepareForRender/
     // cleanupAfterRender to avoid a potential deadlock of prepareForRender/
     // cleanupAfterRender are used to lock/unlock a different mutex
     m_vtkQuickItem->m_viewLock.lock();

     m_vtkRenderWindow->PushState();
     m_vtkRenderWindow->OpenGLInitState();
     m_vtkRenderWindow->InternalRender(); // vtkXOpenGLRenderWindow renders the scene to the FBO
     m_vtkRenderWindow->PopState();

     m_vtkQuickItem->m_viewLock.unlock();

     m_vtkQuickItem->cleanupAfterRender();
   }

   QOpenGLFramebufferObject *createFramebufferObject(const QSize &size)
   {
//      qDebug("QVTKFramebufferObjectRenderer::createFramebufferObject");
      QOpenGLFramebufferObjectFormat format;
      format.setAttachment(QOpenGLFramebufferObject::Depth);
      QOpenGLFramebufferObject *fbo = new QOpenGLFramebufferObject(size, format);
      m_vtkRenderWindow->SetFramebufferObject(fbo);
      return fbo;
   }

   vtkInternalOpenGLRenderWindow *m_vtkRenderWindow;
   QVTKQuickItem *m_vtkQuickItem;
   bool m_neverRendered;
   bool m_readyToRender;

   friend class vtkInternalOpenGLRenderWindow;
};

//
// vtkInternalOpenGLRenderWindow Definitions
//

void vtkInternalOpenGLRenderWindow::Render()
{
   if (this->QtParentRenderer)
   {
      this->QtParentRenderer->update();
   }
}

void vtkInternalOpenGLRenderWindow::SetFramebufferObject(QOpenGLFramebufferObject *fbo)
{
   // QOpenGLFramebufferObject documentation states that "The color render
   // buffer or texture will have the specified internal format, and will
   // be bound to the GL_COLOR_ATTACHMENT0 attachment in the framebuffer
   // object"
   this->BackLeftBuffer = this->FrontLeftBuffer = this->BackBuffer = this->FrontBuffer =
         static_cast<unsigned int>(GL_COLOR_ATTACHMENT0);

   // Save GL objects by static casting to standard C types. GL* types
   // are not allowed in VTK header files.
   QSize fboSize = fbo->size();
   this->SetSize(fboSize.width(), fboSize.height());
   this->NumberOfFrameBuffers = 1;
   this->FrameBufferObject       = static_cast<unsigned int>(fbo->handle());
   this->DepthRenderBufferObject = 0; // static_cast<unsigned int>(depthRenderBufferObject);
   this->TextureObjects[0]       = static_cast<unsigned int>(fbo->texture());
   this->OffScreenRendering = 1;
   this->OffScreenUseFrameBuffer = 1;
   this->Modified();
}

//
// QVTKQuickItem Definitions
//

QVTKQuickItem::QVTKQuickItem(QQuickItem* parent)
  : QQuickFramebufferObject(parent)
{
  setAcceptHoverEvents(true);
  setAcceptedMouseButtons(Qt::LeftButton | Qt::MiddleButton | Qt::RightButton);

  m_interactor = vtkSmartPointer<QVTKInteractor>::New();
  m_interactorAdapter = new QVTKInteractorAdapter(this);
  m_connect = vtkSmartPointer<vtkEventQtSlotConnect>::New();

  // Setup render window and interactor
  m_win = vtkInternalOpenGLRenderWindow::New();
  m_interactor->SetRenderWindow(m_win);
  // Qt::DirectConnection in order to execute callback immediately.
  // This avoids an error when vtkTexture attempts to query driver features and it is unable to determine "IsCurrent"
  m_connect->Connect(m_win, vtkCommand::WindowIsCurrentEvent, this, SLOT(IsCurrent(vtkObject*, unsigned long, void*, void*)), NULL, 0.0, Qt::DirectConnection);
  m_connect->Connect(m_win, vtkCommand::WindowIsDirectEvent, this, SLOT(IsDirect(vtkObject*, unsigned long, void*, void*)), NULL, 0.0, Qt::DirectConnection);
  m_connect->Connect(m_win, vtkCommand::WindowSupportsOpenGLEvent, this, SLOT(SupportsOpenGL(vtkObject*, unsigned long, void*, void*)), NULL, 0.0, Qt::DirectConnection);

  connect(this, SIGNAL(textureFollowsItemSizeChanged(bool)),
          this, SLOT(onTextureFollowsItemSizeChanged(bool)));
}

QVTKQuickItem::~QVTKQuickItem()
{
  if(m_win)
    {
    m_connect->Disconnect(m_win, vtkCommand::WindowIsCurrentEvent, this, SLOT(IsCurrent(vtkObject*, unsigned long, void*, void*)));
    m_connect->Disconnect(m_win, vtkCommand::WindowIsDirectEvent, this, SLOT(IsDirect(vtkObject*, unsigned long, void*, void*)));
    m_connect->Disconnect(m_win, vtkCommand::WindowSupportsOpenGLEvent, this, SLOT(SupportsOpenGL(vtkObject*, unsigned long, void*, void*)));
    m_win->Delete();
    }
}

QQuickFramebufferObject::Renderer *QVTKQuickItem::createRenderer() const
{
  return new QVTKFramebufferObjectRenderer(static_cast<vtkInternalOpenGLRenderWindow*>(m_win));
}

vtkOpenGLRenderWindow* QVTKQuickItem::GetRenderWindow() const
{
  return m_win;
}

QVTKInteractor* QVTKQuickItem::GetInteractor() const
{
  return m_interactor;
}

void QVTKQuickItem::IsCurrent(vtkObject*, unsigned long, void*, void* call_data)
{
  bool* ptr = reinterpret_cast<bool*>(call_data);
  *ptr = QOpenGLContext::currentContext() == this->window()->openglContext();
}

void QVTKQuickItem::IsDirect(vtkObject*, unsigned long, void*, void* call_data)
{
  int* ptr = reinterpret_cast<int*>(call_data);
  *ptr = 1;
}

void QVTKQuickItem::SupportsOpenGL(vtkObject*, unsigned long, void*, void* call_data)
{
  int* ptr = reinterpret_cast<int*>(call_data);
  *ptr = 1;
}

void QVTKQuickItem::onTextureFollowsItemSizeChanged(bool follows)
{
  if (!follows)
  {
    qWarning("QVTKQuickItem: Mouse interaction is not (yet) supported when textureFollowsItemSize==false");
  }
}

void QVTKQuickItem::geometryChanged(const QRectF & newGeometry, const QRectF & oldGeometry)
{
  QQuickFramebufferObject::geometryChanged(newGeometry, oldGeometry);
  QSize oldSize(oldGeometry.width(), oldGeometry.height());
  QSize newSize(newGeometry.width(), newGeometry.height());
  QResizeEvent e(newSize, oldSize);
  if (m_interactorAdapter)
    {
    this->m_viewLock.lock();
    m_interactorAdapter->ProcessEvent(&e, m_interactor);
    this->m_viewLock.unlock();
    }
}

void QVTKQuickItem::keyPressEvent(QKeyEvent* e)
{
  e->accept();
  this->m_viewLock.lock();
  m_interactorAdapter->ProcessEvent(e, m_interactor);
  this->m_viewLock.unlock();
  update();
}

void QVTKQuickItem::keyReleaseEvent(QKeyEvent* e)
{
  e->accept();
  this->m_viewLock.lock();
  m_interactorAdapter->ProcessEvent(e, m_interactor);
  this->m_viewLock.unlock();
  update();
}

void QVTKQuickItem::mousePressEvent(QMouseEvent* e)
{
  e->accept();
  this->m_viewLock.lock();
  m_interactorAdapter->ProcessEvent(e, m_interactor);
  this->m_viewLock.unlock();
  update();
}

void QVTKQuickItem::mouseReleaseEvent(QMouseEvent* e)
{
  e->accept();
  this->m_viewLock.lock();
  m_interactorAdapter->ProcessEvent(e, m_interactor);
  this->m_viewLock.unlock();
  update();
}

void QVTKQuickItem::mouseDoubleClickEvent(QMouseEvent* e)
{
  e->accept();
  this->m_viewLock.lock();
  m_interactorAdapter->ProcessEvent(e, m_interactor);
  this->m_viewLock.unlock();
  update();
}

void QVTKQuickItem::mouseMoveEvent(QMouseEvent* e)
{
  e->accept();
  this->m_viewLock.lock();
  m_interactorAdapter->ProcessEvent(e, m_interactor);
  this->m_viewLock.unlock();
  update();
}

void QVTKQuickItem::wheelEvent(QWheelEvent* e)
{
  e->accept();
  this->m_viewLock.lock();
  m_interactorAdapter->ProcessEvent(e, m_interactor);
  this->m_viewLock.unlock();
  update();
}

void QVTKQuickItem::hoverEnterEvent(QHoverEvent* e)
{
  e->accept();
  QEvent e2(QEvent::Enter);
  this->m_viewLock.lock();
  m_interactorAdapter->ProcessEvent(&e2, m_interactor);
  this->m_viewLock.unlock();
  update();
}

void QVTKQuickItem::hoverLeaveEvent(QHoverEvent* e)
{
  e->accept();
  QEvent e2(QEvent::Leave);
  this->m_viewLock.lock();
  m_interactorAdapter->ProcessEvent(&e2, m_interactor);
  this->m_viewLock.unlock();
  update();
}

void QVTKQuickItem::hoverMoveEvent(QHoverEvent* e)
{
  e->accept();
  QMouseEvent e2(QEvent::MouseMove, e->pos(), Qt::NoButton, Qt::NoButton, e->modifiers());
  this->m_viewLock.lock();
  m_interactorAdapter->ProcessEvent(&e2, m_interactor);
  this->m_viewLock.unlock();
  update();
}

void QVTKQuickItem::init()
{
  m_win->OpenGLInitContext();

  // OpenGLInitContext doesn't do this, but OpenView had it. Why? What's it for?
  m_win->GetExtensionManager()->LoadExtension("GL_VERSION_2_0");
}

bool QVTKQuickItem::prepareForRender()
{
   return true;
}

void QVTKQuickItem::cleanupAfterRender()
{
}
  
