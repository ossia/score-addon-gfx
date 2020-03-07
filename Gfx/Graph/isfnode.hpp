#pragma once
#include "node.hpp"
#include "renderer.hpp"
#include <isf.hpp>
#include <list>

struct ISFNode : NodeModel
{
  struct input_size_vis
  {
    int operator()(const isf::float_input&) noexcept
    {
      return 4;
    }

    int operator()(const isf::long_input&) noexcept
    {
      return 4;
    }

    int operator()(const isf::event_input&) noexcept
    {
      return 4; // bool
    }

    int operator()(const isf::bool_input&) noexcept
    {
      return 4; // bool
    }

    int operator()(const isf::point2d_input&) noexcept
    {
      return 2 * 4;
    }

    int operator()(const isf::point3d_input&) noexcept
    {
      return 3 * 4;
    }

    int operator()(const isf::color_input&) noexcept
    {
      return 4 * 4;
    }

    int operator()(const isf::image_input&) noexcept
    {
      return 0;
    }

    int operator()(const isf::audio_input&) noexcept
    {
      return 0;
    }

    int operator()(const isf::audioFFT_input&)noexcept
    {
      return 0;
    }
  };
  struct input_port_vis
  {
    ISFNode& self;
    char* data{};

    void operator()(const isf::float_input&) noexcept
    {
      self.input.push_back(new Port{&self, data, Types::Float, {}});
      data += 4;
    }

    void operator()(const isf::long_input&) noexcept
    {
      self.input.push_back(new Port{&self, data, Types::Int, {}});
      data += 4;
    }

    void operator()(const isf::event_input&) noexcept
    {
      self.input.push_back(new Port{&self, data, Types::Int, {}});
      data += 4;
    }

    void operator()(const isf::bool_input&) noexcept
    {
      self.input.push_back(new Port{&self, data, Types::Int, {}});
      data += 4;
    }

    void operator()(const isf::point2d_input&) noexcept
    {
      self.input.push_back(new Port{&self, data, Types::Vec2, {}});
      data += 2 * 4;
    }

    void operator()(const isf::point3d_input&) noexcept
    {
      self.input.push_back(new Port{&self, data, Types::Vec3, {}});
      data += 3 * 4;
    }

    void operator()(const isf::color_input&) noexcept
    {
      self.input.push_back(new Port{&self, data, Types::Vec4, {}});
      data += 4 * 4;
    }

    void operator()(const isf::image_input&) noexcept
    {
      self.input.push_back(new Port{&self, {}, Types::Image, {}});
    }

    void operator()(const isf::audio_input& audio) noexcept
    {
      self.audio_textures.push_back({});
      auto& data = self.audio_textures.back();
      data.fixedSize = audio.max;
      self.input.push_back(new Port{&self, &data, Types::Audio, {}});
    }

    void operator()(const isf::audioFFT_input&) noexcept
    {
    }
  };


  // Texture format: 1 row = 1 channel of N samples
  std::list<AudioTexture> audio_textures;

  static const inline QString defaultVert =
      R"_(#version 450
layout(location = 0) in vec2 position;
layout(location = 0) out vec2 isf_FragNormCoord;

void main(void) {
  gl_Position = vec4( position, 0.0, 1.0 );
  isf_FragNormCoord = vec2((gl_Position.x+1.0)/2.0, (gl_Position.y+1.0)/2.0);
}
      )_";;
  ISFNode(const isf::descriptor& desc, QString frag)
    : NodeModel{defaultVert, frag}
  {
    int sz = 0;
    for(const isf::input& input : desc.inputs)
      sz += std::visit(input_size_vis{}, input.data);

    m_materialData.reset(new char[sz]);
    std::fill_n(m_materialData.get(), sz, 0);
    char* cur = m_materialData.get();

    input_port_vis visitor{*this, cur};
    for(const isf::input& input : desc.inputs)
      std::visit(visitor, input.data);

    output.push_back(new Port{this, {}, Types::Image, {}});
  }

  virtual ~ISFNode();


  struct Rendered : RenderedNode
  {
    using RenderedNode::RenderedNode;
    QElapsedTimer t;

    void customInit(Renderer& renderer) override
    {
      QRhi& rhi = *renderer.state.rhi;
      auto& n = (ISFNode&)(node);
      for(auto& texture : n.audio_textures)
      {
          auto sampler = rhi.newSampler(
              QRhiSampler::Linear,
              QRhiSampler::Linear,
              QRhiSampler::None,
              QRhiSampler::ClampToEdge,
              QRhiSampler::ClampToEdge);
          sampler->build();

          m_samplers.push_back({sampler, renderer.m_emptyTexture});
          texture.samplers[&renderer] = {sampler, nullptr};
      }
    }

    void
    customUpdate(Renderer& renderer, QRhiResourceUpdateBatch& res) override
    {
      QRhi& rhi = *renderer.state.rhi;
      auto& n = (ISFNode&)node;
      for(auto& audio : n.audio_textures)
      {
        bool textureChanged = false;
        auto& [rhiSampler, rhiTexture] = audio.samplers[&renderer];
        const auto curSz = (rhiTexture) ? rhiTexture->pixelSize() : QSize{};
        int numSamples = curSz.width() * curSz.height();
        if(numSamples != audio.data.size())
        {
          delete rhiTexture;
          rhiTexture = nullptr;
          textureChanged = true;
        }

        if(!rhiTexture)
        {
          if (audio.channels > 0)
          {
            int samples = audio.data.size() / audio.channels;
            rhiTexture = rhi.newTexture(
                  QRhiTexture::R32F, {samples, audio.channels}, 1, QRhiTexture::Flag{});
            rhiTexture->build();
            textureChanged = true;
          }
          else
          {
            rhiTexture = nullptr;
            textureChanged = true;
          }
        }

        if(textureChanged)
        {
          std::vector<QRhiShaderResourceBinding> tmp;
          tmp.assign(m_srb->cbeginBindings(), m_srb->cendBindings());
          for(QRhiShaderResourceBinding& b : tmp)
          {
            if(b.data()->type == QRhiShaderResourceBinding::Type::SampledTexture)
            {
              if(b.data()->u.stex.sampler == rhiSampler)
              {
                b.data()->u.stex.tex = rhiTexture ? rhiTexture : renderer.m_emptyTexture;
              }
            }
          }
          m_srb->release();
          m_srb->setBindings(tmp.begin(), tmp.end());
          m_srb->build();
        }

        if(rhiTexture)
        {
          QRhiTextureSubresourceUploadDescription subdesc(audio.data.data(), audio.data.size() * 4);
          QRhiTextureUploadEntry entry{0, 0, subdesc};
          QRhiTextureUploadDescription desc{entry};
          res.uploadTexture(rhiTexture, desc);
        }
      }
    }

    void customRelease(Renderer& renderer) override
    {
      auto& n = (ISFNode&)(node);
      for(auto& texture : n.audio_textures)
        if(auto tex = texture.samplers[&renderer].second)
        {
          if(tex != renderer.m_emptyTexture)
            tex->releaseAndDestroyLater();
        }
    }
  };

  RenderedNode* createRenderer() const noexcept override
  {
    return new Rendered{*this};
  }
};
