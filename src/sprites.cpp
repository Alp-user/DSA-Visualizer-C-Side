#include "sprites.h"

template <class T>
void print_vector(const vector<T> &val) {
  for(size_t i = 0; i < val.size(); i++) {
    cout << val[i] << " ";
  }
  cout << std::endl;
}

CircleSquare::CircleSquare() {}
CircleSquare::CircleSquare(unsigned char c_s, float x, float y, float width, float height, float thick, float r, float g, float b)
    : circle_or_square(c_s), x(x), y(y), width(width), height(height), thickness(thick) {
  color[0] = r; color[1] = g; color[2] = b;
}

unsigned int indices[] = {0, 1, 2, 2, 3, 0};

BaseSprite::BaseSprite() {}
void BaseSprite::activate() {
  glCheckError();
  glBindVertexArray(vao_id);
  glUseProgram(program_id);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, veo_id);
  glCheckError();
}
void BaseSprite::deactivate() {
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER,0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
  glUseProgram(0);
}

void BaseSprite::clean_resources() {
  glDeleteBuffers(1, &veo_id);
  glDeleteBuffers(1, &vbo_id);
  glDeleteVertexArrays(1, &vao_id);
  glDeleteProgram(program_id);
  veo_id = 0; vbo_id = 0; vao_id = 0; program_id = 0;
}

CircleSquareSprite::CircleSquareSprite() : BaseSprite() {
  sprite_total = buffer_size = key_index = 0;
  vao_id = program_id = 0;

  thickness = 5;
  orthogonal = glm::ortho(0.0f, 1920.0f, 1080.0f, 0.0f);
  glCheckError();

  std::ifstream v_s("/home/alp/code_files/c++/works/tree_listener/shaders/vertex.vs");
  std::ifstream f_g("/home/alp/code_files/c++/works/tree_listener/shaders/fragment.fg");
  char *v_ss = read_stream_to_cstr(v_s);
  char *f_gs = read_stream_to_cstr(f_g);
  compile(v_ss, f_gs, &program_id);
  free(v_ss); free(f_gs);

  glCheckError();
  glUseProgram(program_id);
  glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(orthogonal));
  glUniform2f(1, 0.0, 0.0);
  glCheckError();

  glCreateVertexArrays(1, &vao_id);
  glBindVertexArray(vao_id);
  glGenBuffers(1, &vbo_id);
  glGenBuffers(1, &veo_id);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, veo_id);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
  glBufferData(GL_ARRAY_BUFFER, CIRCLE_SQUARE_SIZE * sizeof(float), NULL, GL_DYNAMIC_DRAW);

  glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, C_S_OSIZE * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glVertexAttribDivisor(0, 1);

  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, C_S_OSIZE * sizeof(float), (void *)(sizeof(float) * 1));
  glEnableVertexAttribArray(1);
  glVertexAttribDivisor(1, 1);

  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, C_S_OSIZE * sizeof(float), (void *)(sizeof(float) * 4));
  glEnableVertexAttribArray(2);
  glVertexAttribDivisor(2, 1);

  glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, C_S_OSIZE * sizeof(float), (void *)(sizeof(float) * 6));
  glEnableVertexAttribArray(3);
  glVertexAttribDivisor(3, 1);

  glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, C_S_OSIZE * sizeof(float), (void *)(sizeof(float) * 8));
  glEnableVertexAttribArray(4);
  glVertexAttribDivisor(4, 1);
  glCheckError();

  glBindVertexArray(0);
  glUseProgram(0);
  glCheckError();
}

void CircleSquareSprite::draw() {
  activate();
  glCheckError();
  glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, sprite_total);
  glCheckError();
  deactivate();
}

void CircleSquareSprite::sprite_uniform_matrix(float width, float height, float cam_horizontal, float cam_vertical){
  activate();
  glm::mat4 camera_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(-cam_horizontal, cam_vertical, 0.0));
  glm::mat4 final_matrix = glm::ortho(0.0f, width, height, 0.0f, -10.0f, 10.0f) * camera_matrix;
  glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(final_matrix));
  glUniform2f(1, cam_horizontal, cam_vertical);
  glCheckError();
  deactivate();

}

unsigned int CircleSquareSprite::add_data(unsigned char c_s, float x, float y, float width, float height, float thick, float r, float g, float b) {
  activate();
  int id1, id2;
  glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &id1);
  glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &id2);
  sprites.insert(pair(key_index, CircleSquare(c_s, x, y, width, height, thick, r, g, b)));
  CircleSquare &data = sprites[key_index];
  add_data_cpu(data);
  sprite_total++;
  load_c_s_data(key_index, 1);
  deactivate();
  glCheckError();
  return key_index++;
}

unsigned int CircleSquareSprite::override_data(unsigned int sprite_id, float x, float y, float width, float height, float thick, float r, float g , float b){
  auto it = sprites.find(sprite_id);
  assert(it != sprites.end());
  CircleSquare &current = it->second;
  current.x = x; current.y = y;
  current.width = width; current.height = height; current.thickness = thick;
  current.color[0] = r, current.color[1] = g; current.color[2] = b;
  load_changed_data(sprite_id);
  return sprite_id;
}

void CircleSquareSprite::load_c_s_data(unsigned int index, unsigned int length) {
  auto it = sprites.find(index);
  assert(it != sprites.end());
  CircleSquare &data = it->second;
  load_data(data.buffer_index, length * C_S_OSIZE, cpu_side_array_other.data() + data.buffer_index);
  glCheckError();
}

void CircleSquareSprite::move(unsigned int sprite_id, float new_x, float new_y) {
  activate();
  auto it = sprites.find(sprite_id);
  if(it == sprites.end()) {
    cout << "Invalid key";
    assert(false);
  } else {
    CircleSquare &current = it->second;
    current.x = new_x; current.y = new_y;
    load_changed_data(sprite_id);
  }
  deactivate();
  glCheckError();
}

void CircleSquareSprite::color(unsigned int sprite_id, float r, float g, float b) {
  activate();
  auto it = sprites.find(sprite_id);
  if(it == sprites.end()) {
    cout << "Invalid key";
    assert(false);
  } else {
    CircleSquare &current = it->second;
    current.color[0] = r; current.color[1] = g; current.color[2] = b;
    load_changed_data(sprite_id);
  }
  deactivate();
  glCheckError();
}

void CircleSquareSprite::scale(unsigned int sprite_id, float width, float height, float thickness) {
  activate();
  auto it = sprites.find(sprite_id);
  if(it == sprites.end()) {
    cout << "Invalid key";
    assert(false);
  } else {
    CircleSquare &current = it->second;
    current.width = width; current.height = height; current.thickness = thickness;
    load_changed_data(sprite_id);
  }
  deactivate();
  glCheckError();
}

void CircleSquareSprite::remove(unsigned int sprite_id) {
  activate();
  auto it = sprites.find(sprite_id);
  if(it == sprites.end()) {
    cout << "Invalid key";
    assert(false);
  } else {
    CircleSquare &current = it->second;
    current.width = 0; current.height = 0; current.circle_or_square = 4;
    load_changed_data(sprite_id);
  }
  deactivate();
  glCheckError();
}

void CircleSquareSprite::cleanup() {
  activate();
  int buf_ind = 0;

  vector<unsigned int> collect_keys;
  for(auto &current : sprites) {
    auto &data = current.second;
    if(data.circle_or_square == 4) {
      collect_keys.push_back(current.first);
    } else {
      data.buffer_index = buf_ind;
      cpu_side_array_other[buf_ind++] = (data.circle_or_square);
      cpu_side_array_other[buf_ind++] = (data.color[0]);
      cpu_side_array_other[buf_ind++] = (data.color[1]);
      cpu_side_array_other[buf_ind++] = (data.color[2]);
      cpu_side_array_other[buf_ind++] = (data.x);
      cpu_side_array_other[buf_ind++] = (data.y);
      cpu_side_array_other[buf_ind++] = (data.width);
      cpu_side_array_other[buf_ind++] = (data.height);
      cpu_side_array_other[buf_ind++] = (data.thickness);
    }
  }
  for(unsigned int key : collect_keys) {
    sprites.erase(key);
  }
  sprite_total -= collect_keys.size();
  cpu_side_array_other.resize(C_S_OSIZE * sprite_total);
  sprites.rehash(0);
  buffer_size = cpu_side_array_other.size();
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * buffer_size, cpu_side_array_other.data(), GL_DYNAMIC_DRAW);
  deactivate();
  glCheckError();
}

void CircleSquareSprite::add_data_cpu(CircleSquare& value) {
  value.buffer_index = cpu_side_array_other.size();
  cpu_side_array_other.push_back(value.circle_or_square);
  cpu_side_array_other.push_back(value.color[0]);
  cpu_side_array_other.push_back(value.color[1]);
  cpu_side_array_other.push_back(value.color[2]);
  cpu_side_array_other.push_back(value.x);
  cpu_side_array_other.push_back(value.y);
  cpu_side_array_other.push_back(value.width);
  cpu_side_array_other.push_back(value.height);
  cpu_side_array_other.push_back(value.thickness);
  glCheckError();
}

void CircleSquareSprite::load_data(unsigned int first, unsigned int length, float* data) {
  activate();
  if(buffer_size >= first + length) {
    glBufferSubData(GL_ARRAY_BUFFER, first * sizeof(float), sizeof(float) * length, data);
  } else {
    buffer_size = cpu_side_array_other.size();
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * buffer_size, NULL, GL_DYNAMIC_DRAW);
    load_data(0, cpu_side_array_other.size(), cpu_side_array_other.data());
  }
  deactivate();
  glCheckError();
}

void CircleSquareSprite::load_changed_data(unsigned int sprite_id) {
  auto it = sprites.find(sprite_id);
  assert(it != sprites.end());
  CircleSquare &current = it->second;

  int index = current.buffer_index;
  cpu_side_array_other[index + 0] = current.circle_or_square;
  cpu_side_array_other[index + 1] = current.color[0];
  cpu_side_array_other[index + 2] = current.color[1];
  cpu_side_array_other[index + 3] = current.color[2];
  cpu_side_array_other[index + 4] = current.x;
  cpu_side_array_other[index + 5] = current.y;
  cpu_side_array_other[index + 6] = current.width;
  cpu_side_array_other[index + 7] = current.height;
  cpu_side_array_other[index + 8] = current.thickness;

  load_c_s_data(sprite_id, 1);
  glCheckError();
}

void print_circle_square_debug(CircleSquare& cs) {
    printf("\n--- CircleSquare Debug ---\n");
    printf("Type: %s\n", cs.circle_or_square == CIRCLE ? "Circle" : "Square");
    printf("Position: (%.2f, %.2f)\n", cs.x, cs.y);
    printf("Dimensions: %.2f x %.2f\n", cs.width, cs.height);
    printf("Thickness: %.2f\n", cs.thickness);
    printf("Color: (%.2f, %.2f, %.2f)\n", cs.color[0], cs.color[1], cs.color[2]);
    printf("Buffer Index: %u\n", cs.buffer_index);
    printf("-----------------------------\n");
}

void print_cpu_side_array(CircleSquareSprite& sprite) {
    sprite.activate();
    printf("\n--- CPU Side Array Debug ---\n");
    printf("Format: [circle(0)/square(1), r, g, b, x, y, width, height, thickness]\n");
    printf("Total elements: %zu\n", sprite.cpu_side_array_other.size());
    
    for (size_t i = 0; i < sprite.cpu_side_array_other.size(); i += C_S_OSIZE) {
        printf("[%zu]: ", i/C_S_OSIZE);
        printf("type(%d), ", static_cast<int>(sprite.cpu_side_array_other[i]));
        printf("color(%.2f, %.2f, %.2f), ", sprite.cpu_side_array_other[i+1], sprite.cpu_side_array_other[i+2], sprite.cpu_side_array_other[i+3]);
        printf("pos(%.2f, %.2f), ", sprite.cpu_side_array_other[i+4], sprite.cpu_side_array_other[i+5]);
        printf("dim(%.2f, %.2f), ", sprite.cpu_side_array_other[i+6], sprite.cpu_side_array_other[i+7]);
        printf("thick(%.2f)\n", sprite.cpu_side_array_other[i+8]);
    }
    printf("--- End of Debug Output ---\n\n");
    sprite.deactivate();
}

void print_vbo_sprite_debug(CircleSquareSprite& sprite) {
    sprite.activate();
    printf("\n--- OpenGL Vertex Buffer Debug ---\n");
    printf("Format: [circle(0)/square(1), r, g, b, x, y, width, height, thickness]\n");
    
    // Get buffer size
    GLint buffer_size;
    glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &buffer_size);
    printf("Buffer Size: %d bytes (%ld floats)\n", buffer_size, buffer_size / sizeof(float));
    
    // Print buffer contents
    printf("\nBuffer Contents:\n");
    float* mapped_data = (float*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
    if(mapped_data) {
        size_t num_elements = buffer_size / sizeof(float);
        for(size_t i = 0; i < num_elements; i += C_S_OSIZE) {
            printf("[%zu]: ", i/C_S_OSIZE);
            printf("type(%d), ", static_cast<int>(mapped_data[i]));
            printf("color(%.2f, %.2f, %.2f), ", mapped_data[i+1], mapped_data[i+2], mapped_data[i+3]);
            printf("pos(%.2f, %.2f), ", mapped_data[i+4], mapped_data[i+5]);
            printf("dim(%.2f, %.2f), ", mapped_data[i+6], mapped_data[i+7]);
            printf("thick(%.2f)\n", mapped_data[i+8]);
        }
        glUnmapBuffer(GL_ARRAY_BUFFER);
    } else {
        printf("Failed to map buffer for reading\n");
    }
    printf("--- End of VBO Debug Output ---\n\n");
    sprite.deactivate();
}

void print_active_gl_states(){
    GLint vao;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vao);
    std::cout << "Bound VAO: " << vao << std::endl;

    // 2. Get active program
    GLint current_program;
    glGetIntegerv(GL_CURRENT_PROGRAM, &current_program);
    std::cout << "Active program: " << current_program << std::endl;

    // 3. Get buffer bindings
    GLint array_buffer, element_buffer;
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &array_buffer);
    glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &element_buffer);
    std::cout << "Bound VBO: " << array_buffer << std::endl;
    std::cout << "Bound EBO: " << element_buffer << std::endl;
}

