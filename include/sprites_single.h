#define SPRITES_SINGLE_H
#ifdef SPRITES_SINGLE_H
#include <sprites.h>

extern "C"{
  void new_sprite_renderer();
  void destroy_sprite_renderer();

  unsigned int new_line(float x, float y,float width, float height, float angle, float r, float g, float b);

  unsigned int new_circle(float x, float y,float radius, float thickness, float r, float g, float b);

  unsigned int new_triangle(float x, float y,float width, float height, float angle, float r, float g, float b);

  unsigned int new_rectangle(float x, float y,float width, float height, float thickness, float r, float g, float b);

  unsigned int new_square(float x, float y,float width, float thickness, float r, float g, float b);

  void move_sprite(unsigned int sprite_id, float new_x, float new_y);

  void color_sprite(unsigned int sprite_id, float r, float g, float b);

  void scale_sprite(unsigned int sprite_id, float width, float height, float thickness);

  void rotate_sprite(unsigned int sprite_id, float thickness);

  void override_sprite(unsigned int sprite_id,float x, float y,float width, float height,float thick, float r , float g , float b );

  void remove_sprite(unsigned int sprite_id);

  void sprite_cleanup();

  void destroy_renderer();

  CircleSquare* get_sprite(unsigned int sprite_id);

  void draw_sprites();
}

#endif

