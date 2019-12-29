#include "Executor.hpp"

#include <Process/ExecutionContext.hpp>
#include <score/document/DocumentContext.hpp>
#include <ossia/dataflow/port.hpp>
#include <Gfx/Graph/filternode.hpp>

#include <Gfx/Filter/Process.hpp>
#include <Gfx/GfxContext.hpp>
#include <Gfx/GfxExec.hpp>
#include <Gfx/GfxApplicationPlugin.hpp>
#include <Process/Dataflow/Port.hpp>
namespace Gfx::Filter
{
class filter_node final : public gfx_exec_node
{
public:
  filter_node(const QString& frag, gfx_exec_context& ctx)
    : gfx_exec_node{ctx}
  {
    auto n = std::make_unique<FilterNode>(frag);

    id = exec_context->ui->register_node(std::move(n));
  }

  ~filter_node()
  {
    exec_context->ui->unregister_node(id);
  }

  std::string label() const noexcept override
  {
    return "Gfx::filter_node";
  }
};

ProcessExecutorComponent::ProcessExecutorComponent(
    Gfx::Filter::Model& element, const Execution::Context& ctx,
    const Id<score::Component>& id, QObject* parent)
    : ProcessComponent_T{element, ctx, id, "gfxExecutorComponent", parent}
{
  auto n = std::make_shared<filter_node>(element.fragment(), ctx.doc.plugin<DocumentPlugin>().exec);

  int i = 0;
  for(auto& ctl : element.inlets())
  {
    std::pair<ossia::value*, bool>& p = n->add_control();
    if(auto ctrl = dynamic_cast<Process::ControlInlet*>(ctl))
    {
      *p.first = ctrl->value(); // TODO does this make sense ?
      p.second = true; // we will send the first value

      std::weak_ptr<gfx_exec_node> weak_node = n;
      QObject::connect(
            ctrl,
            &Process::ControlInlet::valueChanged,
            this,
            con_unvalidated{ctx, i, weak_node});

    }
    i++;
  }
  n->outputs().push_back(new ossia::value_outlet);

  this->node = n;
  m_ossia_process = std::make_shared<ossia::node_process>(n);
}
}
