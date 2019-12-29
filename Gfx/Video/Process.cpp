#include "Process.hpp"
#include <Gfx/TexturePort.hpp>
#include <Process/Dataflow/Port.hpp>

#include <wobjectimpl.h>
#include <Gfx/Graph/node.hpp>
#include <Gfx/Graph/nodes.hpp>
#include <QShaderBaker>
#include <Process/Dataflow/WidgetInlets.hpp>

W_OBJECT_IMPL(Gfx::Video::Model)
namespace Gfx::Video
{

Model::Model(
    const TimeVal& duration, const Id<Process::ProcessModel>& id,
    QObject* parent)
    : Process::ProcessModel{duration, id, "gfxProcess", parent}
{
  metadata().setInstanceName(*this);
  m_outlets.push_back(new TextureOutlet{Id<Process::Port>(0), this});

  // decoder.load("/home/kdab/test.h264", 60.);
  //setPath("/home/jcelerier/VDMX_Halloween_Hap_4K/inlight_rotate_full_hap.mov");
}

Model::~Model()
{
}

void Model::setPath(const QString& f)
{
  if(f == m_path)
    return;

  m_path = f;
  m_decoder = std::make_shared<video_decoder>();
  m_decoder->load(m_path.toStdString(), 60.);
  pathChanged(f);
}

QString Model::prettyName() const noexcept
{
  return tr("Video");
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
  return {"mkv", "mov", "mp4", "h264", "avi", "hap", "mpg", "mpeg"};
}

QSet<QString> DropHandler::fileExtensions() const noexcept
{
  return {"mkv", "mov", "mp4", "h264", "avi", "hap", "mpg", "mpeg"};
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
      p.creation.key = Metadata<ConcreteKey_k, Gfx::Video::Model>::get();
      p.setup = [str=filename] (Process::ProcessModel& m, score::Dispatcher& disp) {
        auto& midi = static_cast<Gfx::Video::Model&>(m);
        disp.submit(new ChangeVideo{midi, str});
      };
      vec.push_back(std::move(p));
    }
  }
  return vec;
}

}
template <>
void DataStreamReader::read(const Gfx::Video::Model& proc)
{
  readPorts(*this, proc.m_inlets, proc.m_outlets);

  m_stream << proc.m_path;
  insertDelimiter();
}

template <>
void DataStreamWriter::write(Gfx::Video::Model& proc)
{
  writePorts(
      *this,
      components.interfaces<Process::PortFactoryList>(),
      proc.m_inlets,
      proc.m_outlets,
      &proc);

  QString path;
  m_stream >> path;
  proc.setPath(path);
  checkDelimiter();
}

template <>
void JSONObjectReader::read(const Gfx::Video::Model& proc)
{
  readPorts(obj, proc.m_inlets, proc.m_outlets);
  obj["FilePath"] = proc.m_path;
}

template <>
void JSONObjectWriter::write(Gfx::Video::Model& proc)
{
  writePorts(
      obj,
      components.interfaces<Process::PortFactoryList>(),
      proc.m_inlets,
      proc.m_outlets,
      &proc);
  proc.setPath(obj["FilePath"].toString());
}
