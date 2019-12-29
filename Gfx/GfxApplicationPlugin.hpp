#pragma once
#include <Gfx/GfxContext.hpp>
#include <Gfx/GfxExec.hpp>
#include <score/plugins/documentdelegate/plugin/DocumentPlugin.hpp>
#include <score/plugins/application/GUIApplicationPlugin.hpp>

namespace Gfx
{
class DocumentPlugin final
    : public score::DocumentPlugin
{
public:
  DocumentPlugin(
      const score::DocumentContext& doc,
      Id<score::DocumentPlugin> id,
      QObject* parent);
  ~DocumentPlugin() override;


  gfx_window_context context;
  gfx_exec_context exec{context};
};

class ApplicationPlugin final : public score::GUIApplicationPlugin
{
public:
  ApplicationPlugin(const score::GUIApplicationContext& app);

protected:
  void on_createdDocument(score::Document& doc) override;
};
}
