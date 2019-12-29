#pragma once
#include "renderstate.hpp"
#include "uniforms.hpp"

#include <QShaderBaker>

#include <algorithm>
#include <vector>

#include <ossia/detail/flat_map.hpp>

class Node;
struct Port;
struct Edge;

struct Port
{
  Node* node{};
  ValueVariant value;
  Types type{};
  std::vector<Edge*> edges;
};

struct Edge
{
  Edge(Port* source, Port* sink) : source{source}, sink{sink}
  {
    source->edges.push_back(this);
    sink->edges.push_back(this);
  }

  ~Edge()
  {
    if (auto it = std::find(source->edges.begin(), source->edges.end(), this);
        it != source->edges.end())
      source->edges.erase(it);
    if (auto it = std::find(sink->edges.begin(), sink->edges.end(), this);
        it != sink->edges.end())
      sink->edges.erase(it);
  }

  Port* source{};
  Port* sink{};
};

struct Sampler
{
  QRhiSampler* sampler{};
  QRhiTexture* texture{};
};

#if defined(_MSC_VER)
#pragma pack(push, 1)
#endif
struct
    #if defined(__GNUC__) || defined(__clang__)
    __attribute__((packed))
    #endif
ProcessUBO
{
  float time{};
  float timeDelta{};

  float progress{};

  int32_t passIndex{};
};
#if defined(_MSC_VER)
#pragma pack()
#endif

struct Renderer;
class RenderedNode;
class Node
{
  friend class RenderedNode;
public:
  explicit Node();
  explicit Node(QString frag);
  explicit Node(QString vert, QString frag);
  virtual ~Node();

  virtual RenderedNode* createRenderer() const noexcept;

  std::vector<Port*> input;
  std::vector<Port*> output;

  ossia::flat_map<Renderer*, RenderedNode*> renderedNodes;

  ProcessUBO standardUBO{};
protected:
  void setShaders(QString vert, QString frag);

  QShader m_vertexS;
  QShader m_fragmentS;

public:
  bool addedToGraph{};
};


class RenderedNode
{
public:
  RenderedNode(const Node& node) noexcept
    : node{node}
  {

  }

  virtual ~RenderedNode()
  {

  }
  const Node& node;

  QRhiTexture* m_texture{};
  QRhiRenderTarget* m_renderTarget{};
  QRhiRenderPassDescriptor* m_renderPass{};

  std::vector<Sampler> m_samplers;

  // Pipeline
  QRhiShaderResourceBindings* m_srb{};
  QRhiGraphicsPipeline* m_ps{};

  QRhiBuffer* m_processUBO{};

  QRhiBuffer* m_materialUBO{};
  int m_materialSize{};

  friend struct Graph;
  friend struct Renderer;

  void createRenderTarget(RenderState state);
  void setScreenRenderTarget(RenderState state);

  // Render loop
  virtual void customInit(Renderer& renderer);
  void init(Renderer& renderer);

  virtual void customUpdate(Renderer& renderer, QRhiResourceUpdateBatch& res);
  void update(Renderer& renderer, QRhiResourceUpdateBatch& res);

  virtual void customRelease();
  void release();
  void releaseWithoutRenderTarget();

  QRhiGraphicsPipeline* pipeline() { return m_ps; }
  QRhiShaderResourceBindings* resources() { return m_srb; }
};
