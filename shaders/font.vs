#version 450 core
layout (location = 0) in vec2 pos_coords;
layout (location = 1) in vec2 scale_vec;
layout (location = 2) in float font_size;
layout (location = 3) in float rotation;
layout (location = 4) in vec2 tex_coords;
layout (location = 0) out vec2 tex_out;
layout (location = 0) uniform mat4 projection;
layout (location = 1) uniform ivec2 width_height_png;

vec2 quad_pos[4] = vec2[](
  vec2(0, 0),
  vec2(1, 0),
  vec2(1, 1),
  vec2(0, 1)
);

void main() {
  mat2 rot_mat = mat2(cos(rotation), -sin(rotation), sin(rotation), cos(rotation));
  vec2 scales = scale_vec * font_size;
  vec2 scaled_pos = quad_pos[gl_VertexID] * scales;
  vec2 rot_pos = (rot_mat * (scaled_pos)) ;//rotate about top left 
  vec2 trans_pos = rot_pos + pos_coords;
  if(gl_VertexID == 0){
    gl_Position =   projection * vec4(trans_pos, 0.0, 1.0);
    tex_out =  vec2(tex_coords.x, tex_coords.y + (scale_vec.y/width_height_png.y));  
  }
  else if(gl_VertexID == 1){
    gl_Position =   projection * vec4(trans_pos, 0.0, 1.0);
    tex_out =  vec2(tex_coords.x + (scale_vec.x/width_height_png.x), tex_coords.y + (scale_vec.y/width_height_png.y));
  }
  else if(gl_VertexID == 2){
    gl_Position =   projection * vec4(trans_pos, 0.0, 1.0);
    tex_out = vec2(tex_coords.x + (scale_vec.x/width_height_png.x), tex_coords.y);
  }
  else if(gl_VertexID == 3){
    gl_Position =   projection * vec4(trans_pos, 0.0, 1.0);
    tex_out = tex_coords;
  }
}
