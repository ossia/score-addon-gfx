#pragma once
#include "node.hpp"
#include "renderstate.hpp"
#include "uniforms.hpp"

class Window;
struct OutputNode : NodeModel
{
  static const constexpr auto filter = R"_(#version 450
    layout(location = 0) in vec2 v_texcoord;
    layout(location = 0) out vec4 fragColor;

    layout(binding = 3) uniform sampler2D tex;

    void main()
    {
        fragColor = texture(tex, v_texcoord);
    }
    )_";

  virtual ~OutputNode() {}

  std::shared_ptr<Window> window{};

protected:
  OutputNode() : NodeModel{filter} {}
};

struct ColorNode : NodeModel
{
  static const constexpr auto filter = R"_(#version 450
    layout(location = 0) in vec2 v_texcoord;
    layout(location = 0) out vec4 fragColor;

    layout(std140, binding = 1) uniform buf {
        vec4 color;
    } ubuf;


    void main()
    {
        fragColor = ubuf.color;
    }
    )_";

  ColorNode() : NodeModel{filter}
  {
    input.push_back(new Port{this, {}, Types::Vec4, {}});
    input.back()->value = ossia::vec4f{0.6, 0.3, 0.78, 1.};
    output.push_back(new Port{this, {}, Types::Image, {}});
  }

  virtual ~ColorNode();
};

struct NoiseNode : NodeModel
{
  static const constexpr auto filter = R"_(#version 450
    layout(location = 0) in vec2 v_texcoord;
    layout(location = 0) out vec4 fragColor;

    void main()
    {
        fragColor = vec4(sin(v_texcoord.x * 100), 0., cos(v_texcoord.y * 100), 1.);
    }
    )_";

  NoiseNode() : NodeModel{filter}
  {
    output.push_back(new Port{this, {}, Types::Image, {}});
  }
  virtual ~NoiseNode();
};

struct ProductNode : NodeModel
{
  static const constexpr auto filter = R"_(#version 450
    layout(location = 0) in vec2 v_texcoord;
    layout(location = 0) out vec4 fragColor;

    layout(binding = 1) uniform sampler2D t1;
    layout(binding = 2) uniform sampler2D t2;

    void main()
    {
        vec4 c1 = texture(t1, v_texcoord);
        vec4 c2 = texture(t2, v_texcoord);
        fragColor = c1 + c2;
    }
    )_";
  ProductNode() : NodeModel{filter}
  {
    input.push_back(new Port{this, {}, Types::Image, {}});
    input.push_back(new Port{this, {}, Types::Image, {}});
    output.push_back(new Port{this, {}, Types::Image, {}});
  }
  virtual ~ProductNode();
};

struct ScreenNode : OutputNode
{
  ScreenNode() : OutputNode{}
  {
    input.push_back(new Port{this, {}, Types::Image, {}});
  }
  virtual ~ScreenNode();
};
