#pragma once
#include <Gfx/GfxContext.hpp>
#include <Process/ExecutionContext.hpp>


namespace Gfx
{


class gfx_exec_context final : public Execution::ExecutionAction
{
public:
  gfx_exec_context(gfx_window_context& w): ui{&w} { }
  gfx_window_context* ui{};

  void startTick() override
  {
    edges.clear();
  }

  void setEdge(port_index source, port_index sink)
  {
    edges.insert({source, sink});
  }

  void endTick() override
  {
    if(edges != prev_edges)
    {
      {
        std::lock_guard l{ui->edges_lock};
        ui->new_edges = edges;
      }

      prev_edges = edges;
      ui->edges_changed = true;
    }
  }

  ossia::flat_set<std::pair<port_index, port_index>> prev_edges;
  ossia::flat_set<std::pair<port_index, port_index>> edges;
};


}
