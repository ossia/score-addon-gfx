#include "renderer.hpp"

#include "mesh.hpp"

void Renderer::init(QRhi& rhi)
{
  ready = false;
  m_vertexBuffer = rhi.newBuffer(
      QRhiBuffer::Immutable,
      QRhiBuffer::VertexBuffer,
      sizeof(Mesh::vertexArray));
  m_vertexBuffer->build();

  m_rendererUBO = rhi.newBuffer(
      #if defined(_WIN32)
      QRhiBuffer::Dynamic,
      #else
        QRhiBuffer::Immutable,
      #endif
        QRhiBuffer::UniformBuffer, sizeof(ScreenUBO));
  m_rendererUBO->build();

  m_emptyTexture = rhi.newTexture(
      QRhiTexture::RGBA8, QSize{1, 1}, 1, QRhiTexture::Flag{});
  m_emptyTexture->build();
}

void Renderer::release()
{
  for (int i = 0; i < renderedNodes.size(); i++)
    renderedNodes[i]->release();

  delete m_vertexBuffer;
  m_vertexBuffer = nullptr;

  delete m_rendererUBO;
  m_rendererUBO = nullptr;

  delete m_emptyTexture;
  m_emptyTexture = nullptr;

  ready = false;
}

void Renderer::maybeRebuild()
{
  const QSize outputSize = state.swapChain->currentPixelSize();
  if (outputSize != lastSize)
  {
    release();

    // Now we have the nodes in the order in which they are going to
    // be processed

    // For each, we create a render target
    for (int i = 0; i < renderedNodes.size() - 1; i++)
    {
      auto node = renderedNodes[i];
      node->createRenderTarget(state);
    }

    // Except the last one which is going to render to screen
    renderedNodes.back()->setScreenRenderTarget(state);

    init(*state.rhi);
    for (int i = 0; i < renderedNodes.size(); i++)
      renderedNodes[i]->init(*this);

    lastSize = outputSize;
  }
}

void Renderer::render()
{
  if (renderedNodes.size() <= 1)
    return;
  const auto commands = state.swapChain->currentFrameCommandBuffer();

  // Check if the viewport has changed
  maybeRebuild();

  auto updateBatch = state.rhi->nextResourceUpdateBatch();
  update(*updateBatch);

  for (int i = 0; i < renderedNodes.size(); i++)
  {
    auto node = renderedNodes[i];

    node->update(*this, *updateBatch);

    commands->beginPass(
        node->m_renderTarget, Qt::black, {1.0f, 0}, updateBatch);
    draw(*node, *commands);
    commands->endPass();

    if (i < renderedNodes.size() - 1)
      updateBatch = state.rhi->nextResourceUpdateBatch();
  }
}

void Renderer::update(QRhiResourceUpdateBatch& res)
{
  if (!ready)
  {
    ready = true;

    res.uploadStaticBuffer(
        m_vertexBuffer, 0, m_vertexBuffer->size(), Mesh::vertexArray);

    const auto proj = state.rhi->clipSpaceCorrMatrix();

    if (!state.rhi->isYUpInFramebuffer())
    {
      screenUBO.texcoordAdjust[0] = 1.f;
      screenUBO.texcoordAdjust[1] = 0.f;
    }
    else
    {
      screenUBO.texcoordAdjust[0] = -1.f;
      screenUBO.texcoordAdjust[1] = 1.f;
    }
    memcpy(&screenUBO.mvp[0], proj.data(), sizeof(float) * 16);

    screenUBO.renderSize[0] = this->lastSize.width();
    screenUBO.renderSize[1] = this->lastSize.height();
#if defined(_WIN32)
    res.updateDynamicBuffer(m_rendererUBO, 0, sizeof(ScreenUBO), &screenUBO);
#else
    res.uploadStaticBuffer(m_rendererUBO, 0, sizeof(ScreenUBO), &screenUBO);
#endif
  }
}

void Renderer::draw(RenderedNode& node, QRhiCommandBuffer& cb)
{
  const auto sz = state.swapChain->currentPixelSize();
  cb.setGraphicsPipeline(node.pipeline());
  cb.setShaderResources(node.resources());
  cb.setViewport(QRhiViewport(0, 0, sz.width(), sz.height()));

  const QRhiCommandBuffer::VertexInput bindings[]
      = {{m_vertexBuffer, 0}, {m_vertexBuffer, 3 * 2 * sizeof(float)}};

  cb.setVertexInput(0, 2, bindings);
  cb.draw(3);
}
