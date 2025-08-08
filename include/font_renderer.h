#ifndef FONT_RENDERER_H
#define FONT_RENDERER_H

#include <ft2build.h>
#include FT_FREETYPE_H
#include <iostream>
#include <cassert>
#include <glad/glad.h>
#include <cmath>
#include <string>
#include <glm/glm.hpp>
#include <unordered_map>
#include <vector>
#include <fstream>
#include <shader.h>
#include <GLDebug.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <codecvt>
#include <locale>
#include <unordered_set>

#define CHAR_RENDER_SIZE 8
#define PIXEL_PADDING 5

using std::u32string;
using std::unordered_map;
using std::unordered_set;
using std::vector;

struct CharInfo;
struct Text;

static unsigned int vao_id, vbo_id, veo_id, program_id;

static FT_Library font_library;
static FT_Face font_face;
static GLuint font_texture;

static int pixel_size;
static glm::ivec2 texture_dimensions;
static unordered_map<char32_t, CharInfo> font_chars;
static unordered_map<unsigned int, Text> texts;
static vector<float> cpu_side_array;
static unordered_set<unsigned int> changed_keys;
static unsigned int text_id;
static unsigned int vbo_size;//in bytes

static glm::vec3 color;
static glm::mat4 orthogonal;

void texture_atlas_dimensions();
void texture_load_chars();
void texture_process(const char* font_path);
void initialize_font_renderer(const char* font_path);
void print_font_chars();
void print_font_buffer();
void print_projection_matrix();
void print_texts_data();

void activate();
void deactivate();
void render_text();



/* helper functions */
static int next_power_two(int number);
static glm::ivec2 rotate_around(const glm::ivec2& center, const glm::ivec2& other, float angle);
static glm::vec2 rotate_around(const glm::vec2& center, const glm::vec2& other, float angle);
static glm::vec2 rotate_around(const glm::vec2& other, float angle);
static glm::ivec2 rotate_around(const glm::ivec2& other, float angle);
/* END helper functions */

unsigned int create_text(const char* text, int x, int y, int pixel_height);
unsigned int create_text_centered(const char* text, int center_x, int center_y, int max_width, int max_height, float rotation);
void add_char_info(char current, Text& current_text, const CharInfo& current_char, glm::ivec2 coords);
void load_text_vbo(unsigned int text_id);
void load_all_text_vbo();

void move_text(unsigned int text_id, glm::ivec2 new_center);// this moves the center
void rotate_text(unsigned int text_id, float angle);// this rotates around the center
void remove_text(unsigned int text_id);// this zeros the text data so nothing renders
void scale_text(unsigned int text_id, int pixel_height);// this scales the text maintaining same center
void cleanup_text();

struct CharInfo{
  char32_t char_unit;

  glm::ivec2 atlas;
  glm::ivec2 dimensions;
  glm::ivec2 bearings;
  glm::ivec2 advances;
};

struct Text{
  u32string text;
  float scale_constant;
  float rotation;

  glm::ivec2 coordinates;
  glm::ivec2 center_coordinates;
  glm::ivec2 buffer_indices;
  glm::ivec2 box_dimensions;
};


#endif // !FONT_RENDERER_H
