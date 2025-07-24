#include <initializer.h>

void initialize_render(){
  if(!gladLoadGL(glfwGetProcAddress)){
      glfwTerminate();
  }

}
