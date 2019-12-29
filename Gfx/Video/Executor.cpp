#include "Executor.hpp"

#include <Process/ExecutionContext.hpp>

#include <score/document/DocumentContext.hpp>

#include <ossia/dataflow/port.hpp>

#include <Gfx/GfxApplicationPlugin.hpp>
#include <Gfx/GfxContext.hpp>
#include <Gfx/GfxExec.hpp>
#include <Gfx/Graph/videonode.hpp>
#include <Gfx/Video/Process.hpp>
namespace Gfx::Video
{
class video_node final : public gfx_exec_node
{
public:
  video_node(const std::shared_ptr<video_decoder>& dec, gfx_exec_context& ctx)
      : gfx_exec_node{ctx}
  {
    switch (dec->pixel_format())
    {
      case AV_PIX_FMT_YUV420P:
        id = exec_context->ui->register_node(
            std::make_unique<YUV420Node>(dec));
        break;
      case AV_PIX_FMT_RGB0:
        id = exec_context->ui->register_node(std::make_unique<RGB0Node>(dec));
        break;
      default:
        qDebug() << "Unhandled pixel format: "
                 << av_get_pix_fmt_name(dec->pixel_format());
        break;
    }
    dec->seek(0);
  }

  ~video_node()
  {
    if (id >= 0)
      exec_context->ui->unregister_node(id);
  }

  std::string label() const noexcept override { return "Gfx::video_node"; }
};

ProcessExecutorComponent::ProcessExecutorComponent(
    Gfx::Video::Model& element,
    const Execution::Context& ctx,
    const Id<score::Component>& id,
    QObject* parent)
    : ProcessComponent_T{element, ctx, id, "gfxExecutorComponent", parent}
{
  auto n = std::make_shared<video_node>(
      element.decoder(), ctx.doc.plugin<DocumentPlugin>().exec);

  n->outputs().push_back(new ossia::value_outlet);

  this->node = n;
  m_ossia_process = std::make_shared<ossia::node_process>(n);
}
}
