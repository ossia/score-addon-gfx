#pragma once
#include "node.hpp"

#include <QtShaderTools/QShaderBaker>
struct FilterNode : Node
{
  FilterNode(QString frag) : Node{frag}
  {
    const auto& d = m_fragmentS.description();
    for (auto& ub : d.combinedImageSamplers())
    {
      input.push_back(new Port{this, {}, Types::Image, {}});
    }
    for (auto& ub : d.uniformBlocks())
    {
      if (ub.blockName != "material_t")
        continue;

      for (auto& u : ub.members)
      {
        switch (u.type)
        {
          case QShaderDescription::Float:
            input.push_back(new Port{this, {}, Types::Float, {}});
            break;
          case QShaderDescription::Vec2:
            input.push_back(new Port{this, {}, Types::Vec2, {}});
            break;
          case QShaderDescription::Vec3:
            input.push_back(new Port{this, {}, Types::Vec3, {}});
            break;
          case QShaderDescription::Vec4:
            input.push_back(new Port{this, {}, Types::Vec4, {}});
            break;
        }
      }
    }
    output.push_back(new Port{this, {}, Types::Image, {}});
  }

  virtual ~FilterNode();
};
