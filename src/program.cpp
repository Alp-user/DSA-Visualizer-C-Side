#include <GLDebug.h>
#include <algorithm>
#include <glm/ext/matrix_transform.hpp>
#include <regex>
#include <shader.h>
#include <cstdio>
#include <sprites.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <sprites_single.h>
#include <font_renderer.h>

static unsigned int screen_width; 
static unsigned int screen_height;

GLFWwindow* initialize_window(const char* window_name, unsigned int screenwidth, unsigned int screenheight ) {
  screen_width = screenwidth; screen_height = screenheight;
  // Initialize GLFW if not already initialized
  assert(glfwInit());

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_SAMPLES, 4);
  

  GLFWwindow *window = glfwCreateWindow(screen_width,screen_height, window_name, NULL, NULL);
  if(window == NULL){
      printf("Failed to create window.\n");
      glfwTerminate();
      return NULL;
  }
  glfwMakeContextCurrent(window);
  
  // Set swap interval before loading GLAD
  glfwSwapInterval(1);
  
  if(!gladLoadGL(glfwGetProcAddress)){
      printf("Failed to initialize GLAD\n");
      glfwTerminate();
      return NULL;
  }
  
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_MULTISAMPLE); 

  return window;
}

void print_buffer_vector(CircleSquareSprite &first_sprite){
  float info[512];
  first_sprite.activate();
  glGetBufferSubData(GL_ARRAY_BUFFER,0, first_sprite.buffer_size * sizeof(float), info);
  printf("\n vbo");
  for(unsigned int i = 0; i<first_sprite.buffer_size; i++){
    printf(" %f", info[i]);
  }
  printf("\n buffer");
  printf("\n vector");
  //print_vector(first_sprite.cpu_side_array_other);
  first_sprite.deactivate();
  printf("\n vector");
  printf("\n hashmap");
  for(auto current : first_sprite.sprites){
    printf("\n key: %d ", current.first);
    printf(" %d %f %f %f %f %f %f %f %f %d", current.second.circle_or_square, current.second.x,
        current.second.y,current.second.width,current.second.height,
        current.second.thickness,current.second.color[0],current.second.color[1],
        current.second.color[2],current.second.buffer_index);
  }
  printf("\n hashmap");
}

int main(){
  GLFWwindow *window = initialize_window("oh no", 1920, 1080);
  glCheckError();
  initialize_font_renderer("/usr/share/fonts/TTF/CaskaydiaCoveNerdFontMono-Regular.ttf");
  glCheckError();
  new_sprite_renderer();
  glCheckError();
  unsigned int text1 = create_text_centered("5", 500, 500, 100, 100, 0.0);
  glCheckError();
  load_all_text_vbo();
  glCheckError();
  new_circle(500, 500, 150, 5.0, 1.0, 1.0, 1.0);
  glCheckError();

  
  print_texts_data();
  print_font_buffer();

  glCheckError();
  float pixel_height_set = 170;
  while(!glfwWindowShouldClose(window)){
    pixel_height_set -= 0.1;
    glClearColor(0.2f, 0.8f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glCheckError();
    load_all_text_vbo();
    draw_sprites();
    render_text();

    glfwSwapBuffers(window);
    glfwPollEvents();

  }
  glCheckError();
  destroy_renderer();
  glfwDestroyWindow(window);
  glfwTerminate();


  return 0;
}

