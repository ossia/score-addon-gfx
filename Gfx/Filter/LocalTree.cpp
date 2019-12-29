#include "LocalTree.hpp"

#include <LocalTree/Property.hpp>
#include <Gfx/Filter/Process.hpp>

namespace Gfx::Filter
{
LocalTreeProcessComponent::LocalTreeProcessComponent(
    const Id<score::Component>& id, ossia::net::node_base& parent,
    Gfx::Filter::Model& proc, const score::DocumentContext& sys, QObject* parent_obj)
    : LocalTree::ProcessComponent_T<Gfx::Filter::Model>{
          parent, proc, sys, id, "gfxComponent", parent_obj}
{
}

LocalTreeProcessComponent::~LocalTreeProcessComponent()
{
}
}
