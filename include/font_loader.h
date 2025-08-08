#ifndef FONT_LOADER_H
#define FONT_LOADER_H
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unordered_map>
#include <stdlib.h>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <GLDebug.h>
#include <stb_image.h>

#define COL_NUMBER  11


enum FontOrder{
  FONT0, FONT1, FONT2, FONT3, FONT4, FONT5, FONT6, FONT7, FONT8, FONT9, FONT10
};

typedef struct{
  unsigned int id,width, height,page , x_advance;
  int posx, posy , xoff, yoff;
} char_info;

typedef struct{
  unsigned int line_height, size, scale_x, scale_y, number_of_pages, is_alpha;
  size_t number_of_lines; 
  char directory[50];
  char **atlases;
  char_info FontArray[256];
} FontType;

typedef struct{
  float *current_char_array;
  unsigned int index, rendered_index, capacity;
}char_cache;

extern "C" {
  typedef struct{
    char* text;
    unsigned int length;
    unsigned int start_vbo, end_vbo;
    float x,y;
    float font_size;
    float rotation;
  } Text;
}

typedef struct{
  Text *text_array;
  unsigned int capacity, size, index;
}cpu_char_info;


void add_char_info(const char *curchar,  float glo_x,  float glo_y, float font_size, float rotation);
void add_string_info(const char *string, float glo_x,  float glo_y, float font_size, float rotation);
void array_from_texts(unsigned int total_size);

void change_active_font(size_t number);

extern "C"{
  void generate_textures(FontType* curfont);
  void initialize_font_renderer(); //setup single vbo vao and element one here 
  void initialize_font(const char* file_name);
  unsigned int create_text(const char *text, float glo_x, float glo_y, float font_size); //generates in the array and returns its index
  unsigned int create_centered_text(const char *copy_text, float cx, float cy, float width
      , float height);
  unsigned int create_text_height(const char *text, float glo_x, float glo_y, unsigned int pixel_line_heigt); 

  void move_text(unsigned int text_id, float glo_x, float glo_y);
  void rotate_text(unsigned int text_id, float angle);
  void rotate_text_centered(unsigned int text_id, float angle);
  void scale_text(unsigned int text_id, unsigned int pixel_line_heigt);
  void modify_text(unsigned int text_id, const char* new_text);
  void override_text(unsigned int text_id, float glo_x, float glo_y, float font_size, float rotation);
  void delete_text(unsigned int text_id);
  void load_all(); void load_new();
  void render_text();
  void free_font_resources();
  void text_cleanup();

  void resize_window(float width, float height);
  void color_font(float r, float g, float b);
  void vs_off();
  void vs();
  Text *get_text(unsigned int text_id);
}


void print_char_info(char car);
void print_font_type(FontType* curfont);
void print_char_array_debug();
void print_vbo_debug();
void print_active_gl_states();



#endif


