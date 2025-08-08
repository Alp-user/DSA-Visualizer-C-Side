#version 450 core
layout (location = 0) in float circle_square;
layout (location = 1) in vec3 incolor;
layout (location = 2) in vec2 incenter;
layout (location = 3) in vec2 width_height;
layout (location = 4) in float thickness;

layout (location = 0) uniform mat4 projection;

layout (location = 0) out float c_s;
layout (location = 1) out vec3 color;
layout (location = 2) out vec2 center;
layout (location = 3) out vec2 w_h;
layout (location = 4) out float inthickness;
 
void main(){
  switch(gl_VertexID){
    case 0:
      if(circle_square == 2){
        // inthickness is angle here
        gl_Position = vec4(-(width_height.x), (width_height.y), 0.0f,1.0f);
        gl_Position= vec4(gl_Position.x * cos(thickness) + 
        gl_Position.y * sin(thickness)
          ,gl_Position.x * -sin(thickness) + gl_Position.y * cos(thickness),
          0.0f,1.0f);
        gl_Position = projection * vec4(incenter.x + gl_Position.x, incenter.y + gl_Position.y, 0.0f,1.0f);
      }
      else if(circle_square ==  3){
        gl_Position = vec4(0, (width_height.y), 0.0f,1.0f);
        gl_Position= vec4(gl_Position.x * cos(thickness) + 
        gl_Position.y * sin(thickness)
          ,gl_Position.x * -sin(thickness) + gl_Position.y * cos(thickness),
          0.0f,1.0f);
        gl_Position = projection * vec4(incenter.x + gl_Position.x, incenter.y + gl_Position.y, 0.0f,1.0f);
      }
      else{
        gl_Position = projection * vec4(incenter.x - (width_height.x), incenter.y + (width_height.y), 0.0f,1.0f);
      }
      break;
    case 1: 
      if(circle_square == 2){
        gl_Position = vec4((width_height.x), (width_height.y), 0.0f,1.0f);
        gl_Position= vec4(gl_Position.x * cos(thickness) + 
        gl_Position.y * sin(thickness)
          ,gl_Position.x * -sin(thickness) + gl_Position.y * cos(thickness),
          0.0f,1.0f);
        gl_Position = projection * vec4(incenter.x + gl_Position.x, incenter.y + gl_Position.y, 0.0f,1.0f);
      }
      else if(circle_square ==  3){
        gl_Position = vec4(0, (width_height.y), 0.0f,1.0f);
        gl_Position= vec4(gl_Position.x * cos(thickness) + 
        gl_Position.y * sin(thickness)
          ,gl_Position.x * -sin(thickness) + gl_Position.y * cos(thickness),
          0.0f,1.0f);
        gl_Position = projection * vec4(incenter.x + gl_Position.x, incenter.y + gl_Position.y, 0.0f,1.0f);
      }
      else{
        gl_Position = projection * vec4(incenter.x + (width_height.x), incenter.y + (width_height.y),
        0.0f,1.0f);
      }
      break;
    case 2: 
      if(circle_square == 2 || circle_square == 3){
        gl_Position = vec4((width_height.x), -(width_height.y), 0.0f,1.0f);
        gl_Position= vec4(gl_Position.x * cos(thickness) + 
        gl_Position.y * sin(thickness)
          ,gl_Position.x * -sin(thickness) + gl_Position.y * cos(thickness),
          0.0f,1.0f);
        gl_Position = projection * vec4(incenter.x + gl_Position.x, incenter.y + gl_Position.y, 0.0f,1.0f);
      }
      else{
        gl_Position = projection * vec4(incenter.x + (width_height.x), incenter.y - (width_height.y),
        0.0f,1.0f);
      }
      break;
    case 3:
      if(circle_square == 2 || circle_square == 3){
        gl_Position = vec4(-(width_height.x),-(width_height.y), 0.0f,1.0f);
        gl_Position= vec4(gl_Position.x * cos(thickness) + 
        gl_Position.y * sin(thickness)
          ,gl_Position.x * -sin(thickness) + gl_Position.y * cos(thickness),
          0.0f,1.0f);
        gl_Position = projection * vec4(incenter.x + gl_Position.x, incenter.y + gl_Position.y, 0.0f,1.0f);
      }
      else{
        gl_Position = projection * vec4(incenter.x - (width_height.x), incenter.y - (width_height.y),
        0.0f,1.0f);
      }
      break;
    default:
      gl_Position = vec4(0.0f,0.0f,0.0f,1.0f);
  }
  c_s = circle_square;
  center = incenter;
  color = incolor;
  w_h = width_height;
  inthickness = thickness;
}
