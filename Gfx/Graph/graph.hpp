#pragma once
#include "node.hpp"
#include "renderer.hpp"

#include <ossia/detail/algorithms.hpp>
struct OutputNode;
class Window;
struct Graph
{
  std::vector<Node*> nodes;
  std::vector<Edge*> edges;

  void addNode(Node* n) { nodes.push_back(n); }

  void removeNode(Node* n)
  {
    if (auto it = ossia::find(nodes, n); it != nodes.end())
    {
      nodes.erase(it);
    }
  }

  void maybeRebuild(Renderer& r);

  void createRenderer(OutputNode*, RenderState state);

  void setupOutputs(GraphicsApi graphicsApi);

  void relinkGraph();

  ~Graph();

private:
  std::vector<OutputNode*> outputs;
  std::vector<Renderer> renderers;

  std::vector<std::shared_ptr<Window>> unused_windows;

#if QT_CONFIG(vulkan)
  QVulkanInstance vulkanInstance;
  bool vulkanInstanceCreated{};
#endif
};
