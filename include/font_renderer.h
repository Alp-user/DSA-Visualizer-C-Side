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


void texture_atlas_dimensions();
void texture_load_chars();
void texture_process(const char* font_path);
void activate();
void deactivate();

extern "C"{
  void initialize_font_renderer(const char* font_path);
  void render_text();
  unsigned int create_text(const char* text, int x, int y, int pixel_height);
  unsigned int create_text_centered(const char* text, int center_x, int center_y, int max_width, int max_height, float rotation);
  void load_text_vbo(unsigned int text_id);
  void load_all_text_vbo();
  void set_uniform_matrix(float width, float height, float cam_horizontal, float cam_vertical);

  void move_text(unsigned int text_id, int center_x, int center_y);// this moves the center
  void rotate_text(unsigned int text_id, float angle);// this rotates around the center
  void remove_text(unsigned int text_id);// this zeros the text data so nothing renders
  void scale_text(unsigned int text_id, int pixel_height);// this scales the text maintaining same center
  void modify_text(unsigned int text_id, const char* text);
  Text* get_text(unsigned int text_id);
  void cleanup_text();

  struct Text{
    u32string text;
    float scale_constant;
    float rotation;

    glm::ivec2 coordinates;
    glm::ivec2 center_coordinates;
    glm::ivec2 buffer_indices;
    glm::ivec2 box_dimensions;
  };
}


/* helper functions */
/* END helper functions */
void print_font_chars();
void print_font_buffer();
void print_projection_matrix();
void print_texts_data();

void add_char_info(char current, Text& current_text, const CharInfo& current_char, glm::ivec2 coords);

struct CharInfo{
  char32_t char_unit;

  glm::ivec2 atlas;
  glm::ivec2 dimensions;
  glm::ivec2 bearings;
  glm::ivec2 advances;
};



#endif // !FONT_RENDERER_H
