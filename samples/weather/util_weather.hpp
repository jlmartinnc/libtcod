/*
 * Copyright (c) 2010 Jice
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * The name of Jice may not be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY JICE ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL JICE BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <libtcod.hpp>
#include <vector>

class Weather {
 public:
  Weather() = default;
  Weather(int width, int height);

  void update(float delta_time);
  float getCloud(int x, int y);  // 0.0 : dark cloud, 1.0 : no cloud
  float getLightning(int x, int y);  // 0.0 : no lightning. 1.0 : full lightning light
  bool hasRainDrop();  // call for each cell on the map
  // when scrolling the map on the game side
  void move(int dx, int dy);
  // description of current weather
  const char* getWeather() const noexcept;
  const TCODColor& getAmbientLightColor() const noexcept { return ambientColor_; }
  // timeInSecond : between 0 and 3600*24
  void calculateAmbient(float timeInSeconds);
  // how fast the weather is changing. 0 : never changes, 1 : default > 1 : faster...
  void setChangeFactor(float f) { changeFactor_ = f; }
  // 0 : bad weather. 1 : good weather
  float getIndicator() const noexcept { return indicator_; }
  // to alter the weather
  float getIndicatorDelta() const noexcept { return indicator_bias_; }
  void setIndicatorDelta(float v) { indicator_bias_ = std::clamp(v, -1.0f, 1.0f); }

 private:
  struct lightning_t {
    int pos_x{}, pos_y{};
    float intensity{};  // 0-1
    float life{};  // in seconds
    int radius_squared{};
    float noise_x{};
  };

  float indicator_{};  // 0 : bad, 1 : good
  float indicator_bias_{0.0f};
  float noise_x_{20000.0f}, noise_y_{20000.0f};  // position in the noise space
  float dx_{20000.0f}, dy_{20000.0f};  // sub cell cloud map position
  float changeFactor_{1.0f};
  TCODHeightMap map_{};
  std::vector<lightning_t> lightnings_{};
  TCODColor ambientColor_{};
  float elapsed_time_{0.0f};
};
