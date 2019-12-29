#pragma once
#include <private/qrhi_p.h>

struct Mesh
{
  static const inline QRhiVertexInputBinding m_vertexInputBindings[2]{
      {2 * sizeof(float)},
      {2 * sizeof(float)}};
  static const inline QRhiVertexInputAttribute m_vertexAttributeBindings[2]{
      {0, 0, QRhiVertexInputAttribute::Float2, 0},
      {1, 1, QRhiVertexInputAttribute::Float2, 0}};

  static const constexpr auto vertexShader = R"_(#version 450
        layout(location = 0) in vec2 position;
        layout(location = 1) in vec2 texcoord;

        layout(location = 0) out vec2 v_texcoord;

        layout(std140, binding = 0) uniform renderer_t {
          mat4 mvp;
          vec2 texcoordAdjust;
        } renderer;

        out gl_PerVertex { vec4 gl_Position; };

        void main()
        {
          v_texcoord = texcoord;
          gl_Position = renderer.mvp * vec4(position.xy, 0.0, 1.);
        }
    )_";

  static const constexpr float vertexArray[] = {
      -1,
      -1,
      3,
      -1,
      -1,
      3,
      (0 / 2) * 2.,
      (1. - (0 % 2) * 2.),
      (2 / 2) * 2.,
      (1. - (2 % 2) * 2.),
      (1 / 2) * 2.,
      (1. - (1 % 2) * 2.),
  };
};
