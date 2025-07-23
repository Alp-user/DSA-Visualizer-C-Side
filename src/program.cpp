#include <GLDebug.h>
#include <algorithm>
#include <shader.h>
#include <cstdio>
#include <sprites.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <font_loader.h>
#include <sprites_single.h>

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
  for(int i = 0; i<first_sprite.buffer_size; i++){
    printf(" %f", info[i]);
  }
  printf("\n buffer");
  printf("\n vector");
  print_vector(first_sprite.cpu_side_array_other);
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
  initialize_font_renderer();
  initialize_font("/home/alp/code_files/c++/works/tree_listener/font/Unnamed.fnt");
  CircleSquareSprite fs;
  unsigned int circle = fs.add_data(0, 0, 500, 50, 50, 4);
  unsigned int square = fs.add_data(1, 100, 500, 50, 50, 4);
  unsigned int rectangle = fs.add_data(1, 200, 500, 10, 50, 4);
  unsigned int line = fs.add_data(2, 300, 500, 70, 5, 0);
  unsigned int triangle = fs.add_data(3, 400, 500, 50, 50, 0);
  
  // Test movement for all sprites
  fs.move(circle, 100.0f, 100.0f);
  fs.move(square, 200.0f, 100.0f);
  fs.move(rectangle, 300.0f, 100.0f);
  fs.move(line, 400.0f, 100.0f);
  fs.move(triangle, 500.0f, 100.0f);

  // Test coloring for all sprites
  fs.color(circle, 1.0f, 0.0f, 0.0f);    // Red circle
  fs.color(square, 0.0f, 1.0f, 0.0f);    // Green square
  fs.color(rectangle, 0.0f, 0.0f, 1.0f); // Blue rectangle
  fs.color(line, 1.0f, 1.0f, 0.0f);      // Yellow line
  fs.color(triangle, 1.0f, 0.0f, 1.0f);  // Magenta triangle
  fs.remove(triangle);
  fs.cleanup();

  // Test scaling for all sprites
  fs.scale(circle, 60.0f, 60.0f, 6.0f);
  fs.scale(square, 60.0f, 60.0f, 6.0f);
  fs.scale(rectangle, 20.0f, 60.0f, 6.0f);
  fs.scale(line, 80.0f, 10.0f, 0.0f);
  //fs.scale(triangle, 60.0f, 60.0f, 0.0f);

  // Test removing one sprite (triangle)

  // Test cleanup (should remove all remaining sprites)

  print_vbo_sprite_debug(fs);
  print_cpu_side_array(fs);
  create_text("Here", 100, 100);
  
  glCheckError();
  while(!glfwWindowShouldClose(window)){
    glClearColor(0.2f, 0.8f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glCheckError();
    //first_sprite.draw();
    //draw_sprites();
    glCheckError();
    fs.draw();
    render_text();
    //move_text(text2,500 + 400 * sin(glfwGetTime()), 700 );
    //move_text(text6,500 + 400 * cos(glfwGetTime()), 700 );
    //first_sprite.move(tri1, 200 + cos(glfwGetTime()) * 100, 200 + sin(glfwGetTime()) * 100);
    //first_sprite.color(square1, fabs(cos(glfwGetTime())), 1.0f, 1.0f);
    
    glCheckError();

    glfwSwapBuffers(window);
    glfwPollEvents();

  }
  glCheckError();
  free_font_resources();
  fs.clean_resources();
  glfwDestroyWindow(window);
  glfwTerminate();


  return 0;
}

