#include "filternode.hpp"
#include "window.hpp"

Node::~Node() {}

RenderedNode* Node::createRenderer() const noexcept
{
  return new RenderedNode{*this};
}

ColorNode::~ColorNode() {}

ScreenNode::~ScreenNode() {}

ProductNode::~ProductNode() {}

NoiseNode::~NoiseNode() {}

FilterNode::~FilterNode() {}
