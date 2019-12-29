#include "Process.hpp"
#include <Gfx/TexturePort.hpp>
#include <Process/Dataflow/Port.hpp>

#include <wobjectimpl.h>
#include <Gfx/Graph/node.hpp>
#include <Gfx/Graph/nodes.hpp>
#include <QFileInfo>
#include <QShaderBaker>
#include <Process/Dataflow/WidgetInlets.hpp>

W_OBJECT_IMPL(Gfx::Filter::Model)
namespace Gfx::Filter
{

Model::Model(
    const TimeVal& duration, const Id<Process::ProcessModel>& id,
    QObject* parent)
    : Process::ProcessModel{duration, id, "gfxProcess", parent}
{
  metadata().setInstanceName(*this);
  m_outlets.push_back(new TextureOutlet{Id<Process::Port>(0), this});

  setFragment(R"_(#version 450
    layout(location = 0) in vec2 v_texcoord;
    layout(location = 0) out vec4 fragColor;

    // Shared uniform buffer for the whole render window
    layout(std140, binding = 0) uniform renderer_t {
      mat4 mvp;
      vec2 texcoordAdjust;

      vec2 renderSize;
    } renderer;

    // Time-dependent uniforms, only relevant during execution
    layout(std140, binding = 1) uniform process_t {
      float time;
      float timeDelta;
      float progress;

      int passIndex;
    } process;

    // Everything here will be exposed as UI controls
    layout(std140, binding = 2) uniform material_t {
      vec4 color;
    } material;


    void main()
    {
      // Important : texture origin depends on the graphics API used (Vulkan, D3D, etc).
      // Thus the following adjustment is required :
      vec2 texcoord = vec2(v_texcoord.x, renderer.texcoordAdjust.y + renderer.texcoordAdjust.x * v_texcoord.y);

      fragColor = vec4(material.color.rgb * (1+sin(process.progress * 100))/2. * texcoord.xxy, 1.);
    }
  )_");
}

Model::~Model()
{
}

void Model::setFragment(const QString& f)
{
  if(f == m_fragment)
    return;
  m_fragment = f;

  for(auto inlet : m_inlets)
    delete inlet;
  m_inlets.clear();

  QShaderBaker b;
  b.setSourceString(m_fragment.toLatin1(), QShader::Stage::FragmentStage);

  b.setGeneratedShaders({
      {QShader::SpirvShader, 100},
      {QShader::GlslShader,
       120}, // Only GLSL version supported by RHI right now.
      {QShader::HlslShader, 100},
      {QShader::MslShader, 100},
  });

  int i = 0;
  auto s = b.bake();

  const auto& d = s.description();

  for(auto& ub : d.combinedImageSamplers())
  {
    m_inlets.push_back(new TextureInlet{Id<Process::Port>(i++), this});
  }

  for(auto& ub : d.uniformBlocks())
  {
    if(ub.blockName != "material_t")
      continue;

    for(auto& u : ub.members)
    {
      switch(u.type)
      {
        case QShaderDescription::Float:
          m_inlets.push_back(new Process::FloatSlider{Id<Process::Port>(i++), this});
          m_inlets.back()->hidden = true;
          m_inlets.back()->setCustomData(u.name);
          controlAdded(m_inlets.back()->id());
          break;
        case QShaderDescription::Vec4:
          m_inlets.push_back(new Process::HSVSlider{Id<Process::Port>(i++), this});
          m_inlets.back()->hidden = true;
          m_inlets.back()->setCustomData(u.name);
          controlAdded(m_inlets.back()->id());
          break;
        default:
          m_inlets.push_back(new Process::ControlInlet{Id<Process::Port>(i++), this});
          m_inlets.back()->hidden = true;
          m_inlets.back()->setCustomData(u.name);
          controlAdded(m_inlets.back()->id());
          break;
      }
    }
  }

  inletsChanged();
  outletsChanged();
}

QString Model::prettyName() const noexcept
{
  return tr("GFX Filter");
}

void Model::startExecution()
{
}

void Model::stopExecution()
{
}

void Model::reset()
{
}

void Model::setDurationAndScale(const TimeVal& newDuration) noexcept
{
}

void Model::setDurationAndGrow(const TimeVal& newDuration) noexcept
{
}

void Model::setDurationAndShrink(const TimeVal& newDuration) noexcept
{
}

QSet<QString> DropHandler::mimeTypes() const noexcept
{
  return {}; // TODO
}

QSet<QString> LibraryHandler::acceptedFiles() const noexcept
{
  return {"frag", "glsl"};
}

QSet<QString> DropHandler::fileExtensions() const noexcept
{
  return {"frag", "glsl"};
}

std::vector<Process::ProcessDropHandler::ProcessDrop> DropHandler::dropData(
    const std::vector<DroppedFile>& data,
    const score::DocumentContext& ctx) const noexcept
{
  std::vector<Process::ProcessDropHandler::ProcessDrop> vec;
  {
    for (const auto& [filename, file]: data)
    {
      Process::ProcessDropHandler::ProcessDrop p;
      p.creation.key = Metadata<ConcreteKey_k, Gfx::Filter::Model>::get();
      p.creation.prettyName = QFileInfo{filename}.baseName();
      p.setup = [str=file] (Process::ProcessModel& m, score::Dispatcher& disp) {
        auto& midi = static_cast<Gfx::Filter::Model&>(m);
        disp.submit(new ChangeFragmentShader{midi, QString{str}});
      };
      vec.push_back(std::move(p));
    }
  }
  return vec;
}
}
template <>
void DataStreamReader::read(const Gfx::Filter::Model& proc)
{
  readPorts(*this, proc.m_inlets, proc.m_outlets);

  m_stream << proc.m_fragment;
  insertDelimiter();
}

template <>
void DataStreamWriter::write(Gfx::Filter::Model& proc)
{
  writePorts(
      *this,
      components.interfaces<Process::PortFactoryList>(),
      proc.m_inlets,
      proc.m_outlets,
      &proc);

  m_stream >> proc.m_fragment;
  checkDelimiter();
}

template <>
void JSONObjectReader::read(const Gfx::Filter::Model& proc)
{
  readPorts(obj, proc.m_inlets, proc.m_outlets);
  obj["Fragment"] = proc.fragment();
}

template <>
void JSONObjectWriter::write(Gfx::Filter::Model& proc)
{
  writePorts(
      obj,
      components.interfaces<Process::PortFactoryList>(),
      proc.m_inlets,
      proc.m_outlets,
      &proc);
  proc.m_fragment = obj["Fragment"].toString();
}
