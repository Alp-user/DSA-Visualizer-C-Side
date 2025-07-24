#ifndef SPRITES_H
#define SPRITES_H

#include <shader.h>
#include <cassert>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <optional>
#include <glad/glad.h>
#include <vector>
#include <unordered_map>
#include <variant>
#include <glm/glm.hpp>
#include <GLDebug.h>

#define CIRCLE_SQUARE_SIZE 14
#define C_S_VSIZE 8
#define C_S_OSIZE 9

using std::optional;
using std::variant;
using std::vector;
using std::pair;
using std::cout;
using std::unordered_map;

enum DRAW {
  CIRCLE = 0,
  RECTANGLE = 1,
  LINE = 2,
  TRIANGLE = 3,
  REMOVED = 4
};

template <class T>
void print_vector(const vector<T> &val);

extern "C"{
  struct CircleSquare {
    CircleSquare();
    CircleSquare(unsigned char c_s, float x, float y, float width, float height, 
                float thick, float r = 1.0f, float g = 1.0f, float b = 1.0f);
    
    unsigned char circle_or_square;
    float x, y;
    float width, height, thickness;
    float color[3];
    unsigned int buffer_index;
  };
}

extern unsigned int indices[];

class BaseSprite {
public:
  unsigned int program_id, vao_id, vbo_id, veo_id;
  unsigned int key_index, sprite_total, buffer_size;

  BaseSprite();
  void activate();
  void deactivate();
  void clean_resources();
};

class CircleSquareSprite : public BaseSprite {
public:
  vector<float> cpu_side_array_other;
  unordered_map<unsigned int, CircleSquare> sprites;
  float elimination_distance;
  float thickness;
  glm::mat4 orthogonal;

  CircleSquareSprite();
  void draw();
  unsigned int add_data(unsigned char c_s, float x, float y, float width, 
                       float height, float thick, float r = 1, float g = 1, float b = 1);
  unsigned int override_data(unsigned int sprite_id, float x, float y, float width, float height, float thick, float r, float g , float b);
  void load_c_s_data(unsigned int index, unsigned int length);
  void move(unsigned int sprite_id, float new_x, float new_y);
  void color(unsigned int sprite_id, float r, float g, float b);
  void scale(unsigned int sprite_id, float width, float height, float thickness);
  void remove(unsigned int sprite_id);
  void cleanup();

private:
  void add_data_cpu(CircleSquare& value);
  void load_data(unsigned int first, unsigned int length, float* data);
  void load_changed_data(unsigned int sprite_id);
};

void print_circle_square_debug(const CircleSquare& cs);
void print_cpu_side_array(const CircleSquareSprite& sprite);
void print_vbo_sprite_debug(const CircleSquareSprite& sprite);
void print_active_gl_states();
#endif
