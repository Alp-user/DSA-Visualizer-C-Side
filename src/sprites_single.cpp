#include <sprites_single.h>

static CircleSquareSprite* nodes_sprites;

void new_sprite_renderer(){
  nodes_sprites = new CircleSquareSprite();
}

void destroy_sprite_renderer(){
  nodes_sprites->~CircleSquareSprite();
}

unsigned int new_line(float x, float y,float width, float height, float angle, float r, float g, float b){
  return nodes_sprites->add_data(LINE, x, y, width,  height, angle, r,g,b);
}

unsigned int new_circle(float x, float y,float radius, float thickness, float r, float g, float b){
  return nodes_sprites->add_data(CIRCLE, x, y, radius,  radius, thickness, r,g,b);
}

unsigned int new_triangle(float x, float y,float width, float height, float angle, float r, float g, float b){
  return nodes_sprites->add_data(TRIANGLE, x, y, width,  height, angle, r,g,b);
}

unsigned int new_rectangle(float x, float y,float width, float height, float thickness, float r, float g, float b){
  return nodes_sprites->add_data(RECTANGLE, x, y, width,  height, thickness, r,g,b);
}

unsigned int new_square(float x, float y,float width, float thickness, float r, float g, float b){
  return nodes_sprites->add_data(RECTANGLE, x, y, width, width, thickness, r,g,b);
}

void move_sprite(unsigned int sprite_id, float new_x, float new_y){
  nodes_sprites->move(sprite_id, new_x, new_y);
}

void color_sprite(unsigned int sprite_id, float r, float g, float b){
  nodes_sprites->color(sprite_id, r, g, b);
}

void scale_sprite(unsigned int sprite_id, float width, float height, float thickness){
  nodes_sprites->scale(sprite_id, width, height, thickness);
}

void rotate_sprite(unsigned int sprite_id, float thickness){
  nodes_sprites->scale(sprite_id, nodes_sprites->sprites[sprite_id].x, nodes_sprites->sprites[sprite_id].y, thickness);
}

void remove_sprite(unsigned int sprite_id){
  nodes_sprites->remove(sprite_id);
}

void shrink_renderer(){
  nodes_sprites->cleanup();
}

void destroy_renderer(){
  nodes_sprites->clean_resources();
}

void override_sprite(unsigned int sprite_id, float x, float y, float width, float height, float thick, float r, float g , float b){
  nodes_sprites->override_data(sprite_id, x, y, width, height, thick, r,g,b);
}

CircleSquare* get_sprite(unsigned int sprite_id){
  auto it = nodes_sprites->sprites.find(sprite_id);
  assert(it != nodes_sprites->sprites.end());
  return &(it->second);
}

void draw_sprites(){ nodes_sprites->draw();
}

