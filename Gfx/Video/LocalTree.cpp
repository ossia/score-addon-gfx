#include "LocalTree.hpp"

#include <LocalTree/Property.hpp>
#include <Gfx/Video/Process.hpp>

namespace Gfx::Video
{
LocalTreeProcessComponent::LocalTreeProcessComponent(
    const Id<score::Component>& id, ossia::net::node_base& parent,
    Gfx::Video::Model& proc, const score::DocumentContext& sys, QObject* parent_obj)
    : LocalTree::ProcessComponent_T<Gfx::Video::Model>{
          parent, proc, sys, id, "gfxComponent", parent_obj}
{
}

LocalTreeProcessComponent::~LocalTreeProcessComponent()
{
}
}
