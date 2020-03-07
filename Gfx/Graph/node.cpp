#include "node.hpp"

#include "graph.hpp"
#include "mesh.hpp"
#include "renderer.hpp"

NodeModel::NodeModel() {}

NodeModel::NodeModel(QString frag)
{
  setShaders(Mesh::vertexShader, frag);
}

NodeModel::NodeModel(QString vert, QString frag)
{
  setShaders(vert, frag);
}

void RenderedNode::createRenderTarget(const RenderState& state)
{
  auto sz = state.swapChain->surfacePixelSize();
  if(auto true_sz = renderTargetSize())
  {
    sz = *true_sz;
  }

  m_texture = state.rhi->newTexture(
      QRhiTexture::RGBA8, sz, 1, QRhiTexture::RenderTarget);
  m_texture->build();

  QRhiColorAttachment color0{m_texture};

  auto renderTarget = state.rhi->newTextureRenderTarget({color0});
  m_renderPass = renderTarget->newCompatibleRenderPassDescriptor();
  ensure(m_renderPass);
  renderTarget->setRenderPassDescriptor(m_renderPass);
  ensure(renderTarget->build());

  this->m_renderTarget = renderTarget;
}

void RenderedNode::setScreenRenderTarget(const RenderState& state)
{
  m_renderTarget = state.swapChain->currentFrameRenderTarget();
  m_renderPass = state.renderPassDescriptor;
}

std::optional<QSize> RenderedNode::renderTargetSize() const noexcept
{
  return {};
}

void RenderedNode::customInit(Renderer& renderer) {}

void NodeModel::setShaders(QString vert, QString frag)
{
  QShaderBaker b;

  b.setGeneratedShaders({
      {QShader::SpirvShader, 100},
      {QShader::GlslShader, 120},
      {QShader::HlslShader, QShaderVersion(50)},
      {QShader::MslShader, QShaderVersion(12)},
  });
  b.setGeneratedShaderVariants({QShader::Variant{},
                                QShader::Variant{},
                                QShader::Variant{},
                                QShader::Variant{}});

  b.setSourceString(vert.toLatin1(), QShader::VertexStage);
  m_vertexS = b.bake();
  qDebug() << b.errorMessage();

  b.setSourceString(frag.toLatin1(), QShader::FragmentStage);
  m_fragmentS = b.bake();
  qDebug() << b.errorMessage();
  if(!b.errorMessage().isEmpty())
  {
    qDebug() << frag.toStdString().data();
  }

  Q_ASSERT(m_vertexS.isValid());
  Q_ASSERT(m_fragmentS.isValid());
}

void RenderedNode::init(Renderer& renderer)
{
  auto& rhi = *renderer.state.rhi;

  auto& input = node.input;
  m_processUBO = rhi.newBuffer(
      QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, sizeof(ProcessUBO));
  m_processUBO->build();

  // Set up shader inputs
  {
    m_materialSize = 0;
    for (auto in : input)
    {
      switch (in->type)
      {
        case Types::Empty:
          break;
        case Types::Int:
        case Types::Float:
          m_materialSize += 4;
          break;
        case Types::Vec2:
          m_materialSize += 8;
          break;
        case Types::Vec3:
          m_materialSize += 12;
          break;
        case Types::Vec4:
          m_materialSize += 16;
          break;
        case Types::Image:
        {
          auto sampler = rhi.newSampler(
              QRhiSampler::Linear,
              QRhiSampler::Linear,
              QRhiSampler::None,
              QRhiSampler::ClampToEdge,
              QRhiSampler::ClampToEdge);
          ensure(sampler->build());

          QRhiTexture* texture = renderer.m_emptyTexture;
          if (!in->edges.empty())
            if (auto source_node = in->edges[0]->source->node)
              if (auto source_rd = source_node->renderedNodes[&renderer])
                if (auto tex = source_rd->m_texture)
                  texture = tex;

          m_samplers.push_back({sampler, texture});
          break;
        }
        case Types::Audio:
          break;
      }
    }

    if (m_materialSize > 0)
    {
      m_materialUBO = rhi.newBuffer(
          QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, m_materialSize);
      ensure(m_materialUBO->build());
    }
  }

  customInit(renderer);
  // Build the pipeline
  {
    m_ps = rhi.newGraphicsPipeline();
    ensure(m_ps);

    QRhiGraphicsPipeline::TargetBlend premulAlphaBlend;
    premulAlphaBlend.enable = true;
    m_ps->setTargetBlends({premulAlphaBlend});

    m_ps->setSampleCount(1);

    m_ps->setDepthTest(false);
    m_ps->setDepthWrite(false);

    m_ps->setShaderStages({{QRhiShaderStage::Vertex, node.m_vertexS},
                           {QRhiShaderStage::Fragment, node.m_fragmentS}});

    QRhiVertexInputLayout inputLayout;
    inputLayout.setBindings(
        {Mesh::m_vertexInputBindings[0], Mesh::m_vertexInputBindings[1]});
    inputLayout.setAttributes({Mesh::m_vertexAttributeBindings[0],
                               Mesh::m_vertexAttributeBindings[1]});
    m_ps->setVertexInputLayout(inputLayout);

    // Shader resource bindings
    m_srb = rhi.newShaderResourceBindings();
    ensure(m_srb);

    QVector<QRhiShaderResourceBinding> bindings;

    const auto bindingStages = QRhiShaderResourceBinding::VertexStage
                               | QRhiShaderResourceBinding::FragmentStage;

    {
      const auto rendererBinding = QRhiShaderResourceBinding::uniformBuffer(
          0, bindingStages, renderer.m_rendererUBO);
      bindings.push_back(rendererBinding);
    }

    {
      const auto standardUniformBinding
          = QRhiShaderResourceBinding::uniformBuffer(
              1, bindingStages, m_processUBO);
      bindings.push_back(standardUniformBinding);
    }

    // Bind materials
    if (m_materialUBO)
    {
      const auto materialBinding = QRhiShaderResourceBinding::uniformBuffer(
          2, bindingStages, m_materialUBO);
      bindings.push_back(materialBinding);
    }

    // Bind samplers
    int binding = 3;
    for (auto sampler : this->m_samplers)
    {
      assert(sampler.texture);
      bindings.push_back(QRhiShaderResourceBinding::sampledTexture(
          binding,
          QRhiShaderResourceBinding::FragmentStage,
          sampler.texture,
          sampler.sampler));
      binding++;
    }
    m_srb->setBindings(bindings.begin(), bindings.end());
    ensure(m_srb->build());

    m_ps->setShaderResourceBindings(m_srb);

    ensure(m_renderPass);
    m_ps->setRenderPassDescriptor(this->m_renderPass);

    ensure(m_ps->build());
  }
}

void RenderedNode::customUpdate(
    Renderer& renderer,
    QRhiResourceUpdateBatch& res)
{
}

void RenderedNode::update(Renderer& renderer, QRhiResourceUpdateBatch& res)
{
  res.updateDynamicBuffer(
      m_processUBO, 0, sizeof(ProcessUBO), &this->node.standardUBO);

  if (m_materialSize > 0 && materialChangedIndex != node.materialChanged)
  {
    char* data = node.m_materialData.get();
    res.updateDynamicBuffer(m_materialUBO, 0, m_materialSize, data);
    materialChangedIndex = node.materialChanged;
  }

  customUpdate(renderer, res);
}

void RenderedNode::customRelease(Renderer&) {}

void RenderedNode::releaseWithoutRenderTarget(Renderer& r)
{
  customRelease(r);

  for (auto sampler : m_samplers)
  {
    delete sampler.sampler;
    // texture isdeleted elsewxheree
  }
  m_samplers.clear();

  delete m_processUBO;
  m_processUBO = nullptr;

  delete m_materialUBO;
  m_materialUBO = nullptr;
  m_materialSize = 0;

  delete m_ps;
  m_ps = nullptr;

  delete m_srb;
  m_srb = nullptr;
}
void RenderedNode::release(Renderer& r)
{
  releaseWithoutRenderTarget(r);

  if (m_texture)
  {
    delete m_texture;
    m_texture = nullptr;

    delete m_renderPass;
    m_renderPass = nullptr;

    delete m_renderTarget;
    m_renderTarget = nullptr;
  }
}
