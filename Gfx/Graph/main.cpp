#include "filternode.hpp"
#include "window.hpp"

NodeModel::~NodeModel() {}

RenderedNode* NodeModel::createRenderer() const noexcept
{
  return new RenderedNode{*this};
}

ColorNode::~ColorNode() {}

ScreenNode::~ScreenNode() {}

ProductNode::~ProductNode() {}

NoiseNode::~NoiseNode() {}

FilterNode::~FilterNode() {}
