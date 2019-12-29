#include "window.hpp"
#include "filternode.hpp"

Node::~Node() {}

RenderedNode* Node::createRenderer() const noexcept
{
  return new RenderedNode{*this};
}

ColorNode::~ColorNode() {}

ScreenNode::~ScreenNode() {}

ProductNode::~ProductNode() {}

NoiseNode::~NoiseNode() {}

FilterNode::~FilterNode() { }
