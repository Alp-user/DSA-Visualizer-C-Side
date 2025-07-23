#include <sprites.h>
#include <shader.h>
#include <cassert>
#include <cstddef>
#include <fstream>
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
//4*2 vertices

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
void print_vector(const vector<T> &val) {
  for(int i = 0; i < val.size(); i++) {
    cout << val[i] << " ";
  }
  cout << std::endl;
}

struct CircleSquare {
  CircleSquare() {}
  CircleSquare(unsigned char c_s, float x, float y, float width, float height, float thick, float r = 1.0f, float g = 1.0f, float b = 1.0f)
      : circle_or_square(c_s), x(x), y(y), width(width), height(height), thickness(thick) {
    color[0] = r; color[1] = g; color[2] = b;
  }
  unsigned char circle_or_square;
  float x, y;
  float width, height, thickness;
  float color[3];
  unsigned int buffer_index;
};
static unsigned int indices[] = {0, 1, 2, 2, 3, 0};

class BaseSprite {
public:
  unsigned int program_id, vao_id, vbo_id, veo_id;
  unsigned int key_index, sprite_total, buffer_size;

  BaseSprite() {}

  void activate() {
    glBindVertexArray(vao_id);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, veo_id);
    glUseProgram(program_id);
  }

  void deactivate() {
    glBindVertexArray(0);
    glUseProgram(0);
  }
};

class CircleSquareSprite : public BaseSprite {
public:
  vector<float> cpu_side_array_other;
  unordered_map<unsigned int, CircleSquare> sprites;
  float elimination_distance;
  float thickness;
  glm::mat4 orthogonal;

  CircleSquareSprite() : BaseSprite() {
    sprite_total = buffer_size = key_index = 0;
    vao_id = program_id = 0;

    thickness = 5;
    orthogonal = glm::ortho(0.0f, 1920.0f, 0.0f, 1080.0f);
    for(int i = 0; i < 16; i++) {
      cout << orthogonal[i/4][i%4] << " ";
    }

    std::ifstream v_s("shaders/c_s.vs");
    std::ifstream f_g("shaders/c_s.fg");
    char *v_ss = read_stream_to_cstr(v_s);
    char *f_gs = read_stream_to_cstr(f_g);
    compile(v_ss, f_gs, &program_id);
    free(v_ss); free(f_gs);

    glUseProgram(program_id);
    glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(orthogonal));

    glCreateVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);
    glGenBuffers(1, &vbo_id);
    glGenBuffers(1, &veo_id);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, veo_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glBufferData(GL_ARRAY_BUFFER, CIRCLE_SQUARE_SIZE * sizeof(float), NULL, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, C_S_OSIZE * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribDivisor(0, 1);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, C_S_OSIZE * sizeof(float), (void *)(sizeof(float) * 1));
    glEnableVertexAttribArray(1);
    glVertexAttribDivisor(1, 1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, C_S_OSIZE * sizeof(float), (void *)(sizeof(float) * 4));
    glEnableVertexAttribArray(2);
    glVertexAttribDivisor(2, 1);

    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, C_S_OSIZE * sizeof(float), (void *)(sizeof(float) * 6));
    glEnableVertexAttribArray(3);
    glVertexAttribDivisor(3, 1);

    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, C_S_OSIZE * sizeof(float), (void *)(sizeof(float) * 8));
    glEnableVertexAttribArray(4);
    glVertexAttribDivisor(4, 1);
    glCheckError();

    glBindVertexArray(0);
    glUseProgram(0);
    glCheckError();
  }

  void draw() {
    activate();
    glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, sprite_total);
    deactivate();
  }

  unsigned int add_data(unsigned char c_s, float x, float y, float width, float height, float thick, float r = 1, float g = 1, float b = 1) {
    activate();
    int id1, id2;
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &id1);
    glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &id2);
    printf(" ARRAY %d ELEMENT %d ", id1, id2);
    sprites.insert(pair(key_index, CircleSquare(c_s, x, y, width, height, thick, r, g, b)));
    CircleSquare &data = sprites[key_index];
    add_data_cpu(data);
    sprite_total++;
    load_c_s_data(key_index, 1);
    deactivate();
    glCheckError();
    return key_index++;
  }

  void load_c_s_data(unsigned int index, unsigned int length) {
    auto it = sprites.find(index);
    assert(it != sprites.end());
    CircleSquare &data = it->second;
    load_data(data.buffer_index, length * C_S_OSIZE, cpu_side_array_other.data() + data.buffer_index);
    glCheckError();
  }

  void move(unsigned int sprite_id, float new_x, float new_y) {
    activate();
    auto it = sprites.find(sprite_id);
    if(it == sprites.end()) {
      cout << "Invalid key";
      assert(false);
    } else {
      CircleSquare &current = it->second;
      current.x = new_x; current.y = new_y;
      load_changed_data(sprite_id);
    }
    deactivate();
    glCheckError();
  }

  void color(unsigned int sprite_id, float r, float g, float b) {
    activate();
    auto it = sprites.find(sprite_id);
    if(it == sprites.end()) {
      cout << "Invalid key";
      assert(false);
    } else {
      CircleSquare &current = it->second;
      current.color[0] = r; current.color[1] = g; current.color[2] = b;
      load_changed_data(sprite_id);
    }
    deactivate();
    glCheckError();
  }

  void scale(unsigned int sprite_id, float width, float height, float thickness) {
    activate();
    auto it = sprites.find(sprite_id);
    if(it == sprites.end()) {
      cout << "Invalid key";
      assert(false);
    } else {
      CircleSquare &current = it->second;
      current.width = width; current.height = height; current.thickness = thickness;
      load_changed_data(sprite_id);
    }
    deactivate();
    glCheckError();
  }

  void remove(unsigned int sprite_id) {
    activate();
    auto it = sprites.find(sprite_id);
    if(it == sprites.end()) {
      cout << "Invalid key";
      assert(false);
    } else {
      CircleSquare &current = it->second;
      current.width = 0; current.height = 0; current.circle_or_square = 4;
      load_changed_data(sprite_id);
    }
    deactivate();
    glCheckError();
  }

  void cleanup() {
    activate();
    int buf_ind = 0;

    vector<unsigned int> collect_keys;
    for(auto &current : sprites) {
      auto &data = current.second;
      if(data.circle_or_square == 4) {
        collect_keys.push_back(current.first);
      } else {
        data.buffer_index = buf_ind;
        cpu_side_array_other[buf_ind++] = (data.circle_or_square);
        cpu_side_array_other[buf_ind++] = (data.color[0]);
        cpu_side_array_other[buf_ind++] = (data.color[1]);
        cpu_side_array_other[buf_ind++] = (data.color[2]);
        cpu_side_array_other[buf_ind++] = (data.x);
        cpu_side_array_other[buf_ind++] = (data.y);
        cpu_side_array_other[buf_ind++] = (data.width);
        cpu_side_array_other[buf_ind++] = (data.height);
        cpu_side_array_other[buf_ind++] = (data.thickness);
      }
    }
    for(unsigned int key : collect_keys) {
      sprites.erase(key);
    }
    sprite_total -= collect_keys.size();
    cpu_side_array_other.resize(C_S_OSIZE * sprite_total);
    sprites.rehash(0);
    print_vector(cpu_side_array_other);
    buffer_size = cpu_side_array_other.size();
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * buffer_size, cpu_side_array_other.data(), GL_DYNAMIC_DRAW);
    deactivate();
    glCheckError();
  }

private:
  void add_data_cpu(CircleSquare& value) {
    value.buffer_index = cpu_side_array_other.size();
    cpu_side_array_other.push_back(value.circle_or_square);
    cpu_side_array_other.push_back(value.color[0]);
    cpu_side_array_other.push_back(value.color[1]);
    cpu_side_array_other.push_back(value.color[2]);
    cpu_side_array_other.push_back(value.x);
    cpu_side_array_other.push_back(value.y);
    cpu_side_array_other.push_back(value.width);
    cpu_side_array_other.push_back(value.height);
    cpu_side_array_other.push_back(value.thickness);
    print_vector(cpu_side_array_other);
    glCheckError();
  }

  void load_data(unsigned int first, unsigned int length, float* data) {
    activate();
    if(buffer_size >= first + length) {
      glBufferSubData(GL_ARRAY_BUFFER, first * sizeof(float), sizeof(float) * length, data);
    } else {
      buffer_size = cpu_side_array_other.size();
      glBufferData(GL_ARRAY_BUFFER, sizeof(float) * buffer_size, NULL, GL_DYNAMIC_DRAW);
      load_data(0, cpu_side_array_other.size(), cpu_side_array_other.data());
    }
    deactivate();
    glCheckError();
  }

  void load_changed_data(unsigned int sprite_id) {
    auto it = sprites.find(sprite_id);
    assert(it != sprites.end());
    CircleSquare &current = it->second;

    int index = current.buffer_index;
    cpu_side_array_other[index + 0] = current.circle_or_square;
    cpu_side_array_other[index + 1] = current.color[0];
    cpu_side_array_other[index + 2] = current.color[1];
    cpu_side_array_other[index + 3] = current.color[2];
    cpu_side_array_other[index + 4] = current.x;
    cpu_side_array_other[index + 5] = current.y;
    cpu_side_array_other[index + 6] = current.width;
    cpu_side_array_other[index + 7] = current.height;
    cpu_side_array_other[index + 8] = current.thickness;

    load_c_s_data(sprite_id, 1);
    glCheckError();
  }
};

