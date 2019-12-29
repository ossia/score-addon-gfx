#pragma once
#include "node.hpp"

#if defined(_MSC_VER)
#pragma pack(push, 1)
#endif
struct
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((packed))
#endif
    ScreenUBO
{
  float mvp[16]{};
  float texcoordAdjust[2]{};

  float renderSize[2]{};
};

#if defined(_MSC_VER)
#pragma pack()
#endif

struct Renderer
{
  std::vector<Node*> nodes;
  std::vector<RenderedNode*> renderedNodes;

  RenderState state;
  QSize lastSize{};

  // Mesh
  QRhiBuffer* m_vertexBuffer = nullptr;

  // Material
  ScreenUBO screenUBO;
  QRhiBuffer* m_rendererUBO{};

  QRhiTexture* m_emptyTexture{};

  bool ready{};

  void init(QRhi& rhi);
  void release();

  void render();

  void update(QRhiResourceUpdateBatch& res);
  void draw(RenderedNode& node, QRhiCommandBuffer& cb);

  void maybeRebuild();
};
