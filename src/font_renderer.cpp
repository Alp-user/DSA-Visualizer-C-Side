#include <font_renderer.h>
#include <utility>

static unsigned int indices_element[] = {0,1,2,2,3,0};

void initialize_font_renderer(const char* font_path){
  texture_process(font_path);
  //SHADERS
  std::ifstream text_vs("/home/alp/code_files/c++/works/tree_listener/shaders/font.vs");
  std::ifstream text_fg("/home/alp/code_files/c++/works/tree_listener/shaders/font.fg");
  compile(read_stream_to_cstr(text_vs), read_stream_to_cstr(text_fg), &program_id);
  //BUFFERS
  glCheckError();
  glCreateVertexArrays(1, &vao_id); glBindVertexArray(vao_id);
  glCheckError();
  glGenBuffers(1, &vbo_id); glGenBuffers(1,&veo_id);

  glCheckError();
  glBindBuffer(GL_ARRAY_BUFFER, vbo_id); glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, veo_id);
  glBufferData(GL_ARRAY_BUFFER,1 , NULL, GL_DYNAMIC_DRAW);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices_element), indices_element, GL_STATIC_DRAW);
  glCheckError();

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, CHAR_RENDER_SIZE * sizeof(float),(void *) 0);
  glEnableVertexAttribArray(0);
  glVertexAttribDivisor(0,1);

  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, CHAR_RENDER_SIZE * sizeof(float), (void *)(2* sizeof(float)));
  glEnableVertexAttribArray(1);
  glVertexAttribDivisor(1,1);

  glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, CHAR_RENDER_SIZE * sizeof(float),(void *)(4* sizeof(float)));
  glEnableVertexAttribArray(2);
  glVertexAttribDivisor(2,1);

  glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, CHAR_RENDER_SIZE * sizeof(float),(void *)(5* sizeof(float)));
  glEnableVertexAttribArray(3);
  glVertexAttribDivisor(3,1);

  glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, CHAR_RENDER_SIZE * sizeof(float),(void *)(6* sizeof(float)));
  glEnableVertexAttribArray(4);
  glVertexAttribDivisor(4,1);
  glCheckError();

  //uniform and static variable
  text_id = 0;
  vbo_size = 1;
  orthogonal = glm::ortho(0.0,1920.0,1080.0,0.0,-10.0,10.0);
  color = glm::vec3(1.0f, 1.0f, 1.0f);
  
  glUseProgram(program_id);
  glUniformMatrix4fv(0,1,GL_FALSE, glm::value_ptr(orthogonal));
  glUniform2iv(1,1, glm::value_ptr(texture_dimensions));
  glUniform3fv(2,1,glm::value_ptr(color));
  glCheckError();

  deactivate();

}

void texture_process(const char* font_path){
  FT_Error error = FT_Init_FreeType(&font_library);
  assert(!error);

  error = FT_New_Face(font_library, font_path, 0, &font_face);
  assert(!error);

  std::cout << "Step 2: Font loaded successfully" << std::endl;
  std::cout << "  Font family: " << font_face->family_name << std::endl;
  std::cout << "  Font style: " << font_face->style_name << std::endl;

  pixel_size = 48;
  error = FT_Set_Pixel_Sizes(font_face, 0, pixel_size);
  assert(!error);

  /* END FreeType initialization */
  /* Creating texture object */

  glGenTextures(1, &font_texture);
  glActiveTexture(GL_TEXTURE10);
  glBindTexture(GL_TEXTURE_2D, font_texture);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glCheckError();

  /* END Creating texture object */
  /* Calculate texture atlas size and create texture storage*/

  texture_atlas_dimensions();//sets texture_dimensions
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);//reads in 1 byte chunks
  glTexStorage2D(GL_TEXTURE_2D, 1, GL_R8, texture_dimensions.x, texture_dimensions.y);
  texture_load_chars();
  //print_font_chars();

}

void texture_load_chars(){
  int current_x = 0; 
  int current_y = 0;
  for(int ascii_code = 32; ascii_code < 127; ascii_code++){

    FT_Load_Char(font_face, (char32_t)ascii_code, FT_LOAD_RENDER);
    const FT_Bitmap& bitmap = font_face->glyph->bitmap;
    const FT_GlyphSlot& glyph = font_face->glyph;

    if(current_x + bitmap.width > texture_dimensions.x){
      current_x = 0;
      current_y += pixel_size + PIXEL_PADDING;
    }

    //flips
    vector<unsigned char> flipped(bitmap.width * bitmap.rows);
    for (int row = 0; row < bitmap.rows; ++row) {
      memcpy(&flipped[row * bitmap.width],
         &glyph->bitmap.buffer[(bitmap.rows - 1 - row) * bitmap.width],
         bitmap.width);
    }

    CharInfo& current_char = font_chars[(char32_t) ascii_code];
    current_char.char_unit = (char32_t) ascii_code;

    current_char.dimensions = glm::ivec2(bitmap.width, bitmap.rows);
    current_char.bearings = glm::ivec2(glyph->bitmap_left, -glyph->bitmap_top);
    current_char.atlas = glm::ivec2(current_x, current_y);
    current_char.advances = glm::ivec2(glyph->advance.x >> 6, glyph->advance.y >> 6);

    glTexSubImage2D(GL_TEXTURE_2D, 0, current_x, current_y ,
        bitmap.width, bitmap.rows, GL_RED,
        GL_UNSIGNED_BYTE, flipped.data());

    current_x += bitmap.width + PIXEL_PADDING;
  }
}

void texture_atlas_dimensions(){
  int total_area;
  unsigned int total_width = 0;
  unsigned int max_height = 0;
  for(int ascii_code = 32; ascii_code < 127; ascii_code++){
    FT_Load_Char(font_face, (char)ascii_code, FT_LOAD_RENDER);
    total_width += font_face->glyph->bitmap.width;
    max_height = max_height > font_face->glyph->bitmap.rows ? max_height : font_face->glyph->bitmap.rows;
  }
  //20% padding
  total_area = max_height * total_width;
  total_area *= 1.2;
  int log_total_area = next_power_two(total_area);
  if(log_total_area % 2 == 0){
    texture_dimensions.x = texture_dimensions.y = pow(2, log_total_area / 2.0);
  }
  else{
    texture_dimensions.x = pow(2, ((int) log_total_area/2 + 1));
    texture_dimensions.y = pow(2, (int) log_total_area/2 );
  }

  std::cout<<"Texture width: "<< texture_dimensions.x<<" Texture height: "<<texture_dimensions.y<<std::endl;
}

void activate(){
  glActiveTexture(GL_TEXTURE10);
  glUseProgram(program_id);
  glBindVertexArray(vao_id);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, veo_id);

}

void deactivate(){
  glUseProgram(0);
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

unsigned int create_text(const char* text, int x, int y, int pixel_height){//only ascii. Take x,y as top left.
  Text& current_text = texts[text_id];
  current_text.coordinates.x = x;
  current_text.coordinates.y = y;
  current_text.rotation = 0;
  current_text.scale_constant = (float)pixel_height / pixel_size;
  current_text.buffer_indices.x = cpu_side_array.size();

  const char* current = text;
  unsigned int text_length = 0;
  unsigned int lines = 1;

  //y is top left but line is written from below so one lineheight below
  glm::ivec2 current_coords(x, y + pixel_height);
  int line_width = 0;
  int max_line_width = 0;

  for(; *current != '\0'; current++){
    switch (*current){
      case 10:{
        max_line_width = std::max(max_line_width, line_width);
        line_width = 0;
        lines++;
        current_coords.y += pixel_height;
        current_coords.x = x;
        break;
      }
      default:{
        const auto& current_char = font_chars[(char32_t) *current];
        glm::ivec2 char_coords = current_coords + glm::ivec2(
            current_char.bearings.x * current_text.scale_constant,
            current_char.bearings.y * current_text.scale_constant
        );
        add_char_info(*current, current_text, current_char, char_coords);
        text_length++;
        line_width += current_char.advances.x * current_text.scale_constant;
        current_coords.x += current_char.advances.x * current_text.scale_constant;
      }
    }
  }
  
  max_line_width = std::max(max_line_width, line_width);
  
  int total_width = max_line_width;
  int total_height = lines * pixel_height;
  u32string new_text(text, text + text_length);
  
  current_text.center_coordinates.x = x + total_width / 2;
  current_text.center_coordinates.y = y + total_height / 2;
  current_text.text = std::move(new_text);
  current_text.buffer_indices.y = cpu_side_array.size();//exclusive
  current_text.box_dimensions.x = total_width;
  current_text.box_dimensions.y = total_height;

  changed_keys.insert(text_id);
  return text_id++;
}

void cleanup_text(){
  vector<unsigned int> keys_to_remove;
  vector<float> new_cpu_side_array;
  std::swap(new_cpu_side_array, cpu_side_array);//cpu_side_array is empty now
  for(auto& [key, value] : texts){
    if(value.scale_constant == 0){
      keys_to_remove.push_back(key);
    }
    else{
      unsigned int first_index = cpu_side_array.size();
      cpu_side_array.insert(cpu_side_array.end(),
        new_cpu_side_array.begin() + value.buffer_indices.x,
        new_cpu_side_array.begin() + value.buffer_indices.y);
      unsigned int end_index = cpu_side_array.size();
      value.buffer_indices = glm::ivec2(first_index, end_index);
    }
  }
  for(const auto& key : keys_to_remove){
    texts.erase(key);
  }
  activate();
  vbo_size = cpu_side_array.size();
  glBufferData(GL_ARRAY_BUFFER, cpu_side_array.size() * sizeof(float), cpu_side_array.data(), GL_DYNAMIC_DRAW);
  deactivate();
}

unsigned int create_text_centered(const char* text, int center_x, int center_y, int max_width, int max_height, float rotation){
  unsigned int lines = 1;
  int line_width = 0;
  int max_line_width = 0;
  
  const char* current = text;
  for(; *current != '\0'; current++){
    switch (*current){
      case 10:{
        max_line_width = std::max(max_line_width, line_width);
        line_width = 0;
        lines++;
        break;
      }
      default:{
        const auto& current_char = font_chars[(char32_t) *current];
        line_width += current_char.advances.x;
      }
    }
  }
  
  max_line_width = std::max(max_line_width, line_width);
  
  int reference_total_height = lines * pixel_size;
  
  // Calculate scale ratios to fit within bounds
  float width_ratio = (float)max_width / max_line_width;
  float height_ratio = (float)max_height / reference_total_height;
  
  // Pick the smaller ratio and apply 0.9 padding
  float final_scale = std::min(width_ratio, height_ratio) * 0.9f;
  int final_pixel_height = pixel_size * final_scale;
  
  // Calculate final dimensions
  int final_width = max_line_width * final_scale;
  int final_height = reference_total_height * final_scale;
  
  // Create text at calculated position
  unsigned int new_text_id = create_text(text, center_x - final_width / 2,center_y - final_height / 2, final_pixel_height);
  // Apply rotation if needed
  if(rotation != 0.0f){
    rotate_text(new_text_id, rotation);
  }
  
  return new_text_id;
}

void add_char_info(char current, Text& current_text, const CharInfo& current_char, glm::ivec2 coords){
  // Position (location = 0) vec2 pos_coords
  cpu_side_array.push_back(coords.x);
  cpu_side_array.push_back(coords.y);
  
  // Scale vector (location = 1) vec2 scale_vec - character dimensions scaled
  cpu_side_array.push_back(current_char.dimensions.x );
  cpu_side_array.push_back(current_char.dimensions.y );
  
  // Font size (location = 2) float font_size
  cpu_side_array.push_back(current_text.scale_constant);
  
  // Rotation (location = 3) float rotation  
  cpu_side_array.push_back(current_text.rotation);
  
  // Texture coordinates (location = 4) vec2 tex_coords - normalized atlas coordinates
  cpu_side_array.push_back((float)current_char.atlas.x / texture_dimensions.x);
  cpu_side_array.push_back((float)current_char.atlas.y / texture_dimensions.y);
}

//if you want to modify and give a larger text than old one will be removed and new will be created
//if text was modified, end vbo will not change. It will not shrink but only zero the rest
void load_text_vbo(unsigned int text_id){//this function assumes you did not give it a larger text
  activate();
  const auto& find_it = texts.find(text_id);
  assert(find_it != texts.end());
  const Text& current_text = find_it->second;
  unsigned int total_size = cpu_side_array.size() * sizeof(float);

  if(total_size > vbo_size){
    vbo_size = total_size *= 2;
    glBufferData(GL_ARRAY_BUFFER, vbo_size , nullptr, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, cpu_side_array.size() * sizeof(float), cpu_side_array.data());
  }
  else{
    glBufferSubData(GL_ARRAY_BUFFER, current_text.buffer_indices.x * sizeof(float),
        (current_text.buffer_indices.y - current_text.buffer_indices.x) * sizeof(float),
        cpu_side_array.data() + current_text.buffer_indices.x);
  }
  deactivate();
}

void load_all_text_vbo(){
  for(const auto& key : changed_keys){
    load_text_vbo(key);
  }
  changed_keys.clear();
}

void rotate_text(unsigned int text_id, float angle){
  const auto& find_it = texts.find(text_id);
  assert(find_it != texts.end());
  Text& current_text = find_it->second;
  float rotation_difference = angle - current_text.rotation;
  current_text.rotation = angle;
  
  for(unsigned int i = current_text.buffer_indices.x; i < current_text.buffer_indices.y; i += CHAR_RENDER_SIZE){
    glm::vec2 rotated_values = rotate_around((glm::vec2)current_text.center_coordinates,
        glm::vec2(cpu_side_array[i], cpu_side_array[i+1]), rotation_difference);
    cpu_side_array[i] = rotated_values.x;
    cpu_side_array[i+1] = rotated_values.y;
    cpu_side_array[i+5] = angle;
  }
  current_text.coordinates = rotate_around(current_text.center_coordinates,current_text.coordinates, angle);
  changed_keys.insert(text_id);
}

void move_text(unsigned int text_id, glm::ivec2 new_center){
  const auto& find_it = texts.find(text_id);
  assert(find_it != texts.end());
  Text& current_text = find_it->second;
  
  glm::ivec2 difference = new_center - current_text.center_coordinates;
  for(unsigned int i = current_text.buffer_indices.x; i < current_text.buffer_indices.y; i += CHAR_RENDER_SIZE){
    cpu_side_array[i] += difference.x;
    cpu_side_array[i+1] += difference.y;
  }
  current_text.center_coordinates = new_center;
  current_text.coordinates = current_text.coordinates + difference;
  changed_keys.insert(text_id);
}

void remove_text(unsigned int text_id){
  const auto& find_it = texts.find(text_id);
  assert(find_it != texts.end());
  Text& current_text = find_it->second;
  current_text.scale_constant = 0;//Marker for deletion

  
  for(unsigned int i = current_text.buffer_indices.x; i < current_text.buffer_indices.y; i += CHAR_RENDER_SIZE){
    cpu_side_array[i] = 0;     // Position x
    cpu_side_array[i+1] = 0;   // Position y
    cpu_side_array[i+2] = 0;   // Scale x
    cpu_side_array[i+3] = 0;   // Scale y
    cpu_side_array[i+4] = 0;   // Scale constant
    cpu_side_array[i+5] = 0;   // Rotation
    cpu_side_array[i+6] = 0;   // Texture coords x
    cpu_side_array[i+7] = 0;   // Texture coords y
  }
  changed_keys.insert(text_id);
}

void scale_text(unsigned int text_id, int pixel_height){
  const auto& find_it = texts.find(text_id);
  assert(find_it != texts.end());
  Text& current_text = find_it->second;
  
  // Calculate scale ratio
  float new_scale_constant = (float)pixel_height / pixel_size;
  float scale_ratio = new_scale_constant / current_text.scale_constant;
  
  // Calculate new dimensions
  int new_width = current_text.box_dimensions.x * scale_ratio;
  int new_height = current_text.box_dimensions.y * scale_ratio;
  
  // Calculate new top-left coordinates to maintain center
  int new_x = current_text.center_coordinates.x - new_width / 2;
  int new_y = current_text.center_coordinates.y - new_height / 2;
  
  // Update text properties
  current_text.scale_constant = new_scale_constant;
  current_text.coordinates.x = new_x;
  current_text.coordinates.y = new_y;
  current_text.box_dimensions.x = new_width;
  current_text.box_dimensions.y = new_height;
  
  // Recreate character positions similar to create_text
  unsigned int lines = 1;
  glm::ivec2 current_coords(new_x, new_y + pixel_height);
  unsigned int buffer_index = current_text.buffer_indices.x;
  
  for(const char32_t& current : current_text.text){
    switch (current){
      case 10:{
        lines++;
        current_coords.y += pixel_height;
        current_coords.x = new_x;
        break;
      }
      default:{
        const auto& current_char = font_chars[current];
        glm::ivec2 char_coords = current_coords + glm::ivec2(
            current_char.bearings.x * new_scale_constant,
            current_char.bearings.y * new_scale_constant
        );
        
        // Update buffer data directly
        cpu_side_array[buffer_index] = char_coords.x;
        cpu_side_array[buffer_index+1] = char_coords.y;
        cpu_side_array[buffer_index+2] = current_char.dimensions.x;
        cpu_side_array[buffer_index+3] = current_char.dimensions.y;
        cpu_side_array[buffer_index+4] = new_scale_constant;
        cpu_side_array[buffer_index+5] = current_text.rotation;
        cpu_side_array[buffer_index+6] = (float)current_char.atlas.x / texture_dimensions.x;
        cpu_side_array[buffer_index+7] = (float)current_char.atlas.y / texture_dimensions.y;
        
        buffer_index += CHAR_RENDER_SIZE;
        current_coords.x += current_char.advances.x * new_scale_constant;
      }
    }
  }
  changed_keys.insert(text_id);
}

void render_text(){
  activate();
  glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, cpu_side_array.size() / CHAR_RENDER_SIZE);
  deactivate();
}

static int next_power_two(int number){
  return log2(number) + 1;
}

static glm::ivec2 rotate_around(const glm::ivec2& center, const glm::ivec2& other, float angle){
  glm::mat2 rotation_matrix(
    cos(angle), -sin(angle),
    sin(angle), cos(angle)
  );
  glm::ivec2 difference = (glm::ivec2)(rotation_matrix * (other - center));
  return (center + difference);
}

static glm::vec2 rotate_around(const glm::vec2& center, const glm::vec2& other, float angle){
  glm::mat2 rotation_matrix(
    cos(angle), -sin(angle),
    sin(angle), cos(angle)
  );
  glm::vec2 difference = (glm::vec2)(rotation_matrix * (other - center));
  return (center + difference);
}

static glm::ivec2 rotate_around(const glm::ivec2& other, float angle){
  glm::mat2 rotation_matrix(
    cos(angle), -sin(angle),
    sin(angle), cos(angle)
  );
  glm::ivec2 difference = (glm::ivec2)(rotation_matrix * (other));
  return difference;
}

static glm::vec2 rotate_around(const glm::vec2& other, float angle){
  glm::mat2 rotation_matrix(
    cos(angle), -sin(angle),
    sin(angle), cos(angle)
  );
  glm::vec2 difference = (glm::vec2)(rotation_matrix * (other));
  return difference;
}

void print_font_chars(){
  std::cout << "Font Characters Information:" << std::endl;
  std::cout << "Total characters: " << font_chars.size() << std::endl;
  std::cout << "----------------------------------------" << std::endl;
  
  for(const auto& pair : font_chars){
    const CharInfo& info = pair.second;
    std::cout << "Character: '" << (char)info.char_unit << "' (ASCII: " << (int)info.char_unit << ")" << std::endl;
      std::cout << "  Atlas position: (" << info.atlas.x << ", " << info.atlas.y << ")" << std::endl;
      std::cout << "  Dimensions: " << info.dimensions.x << "x" << info.dimensions.y << std::endl;
      std::cout << "  Bearing: (" << info.bearings.x << ", " << info.bearings.y << ")" << std::endl;
      std::cout << "  Advance: (" << info.advances.x << ", " << info.advances.y << ")" << std::endl;
    std::cout << "  ---" << std::endl;
  }
}

void print_font_buffer(){
  std::cout << "\n========================================" << std::endl;
  std::cout << "Font Buffer Contents" << std::endl;
  std::cout << "========================================" << std::endl;
  
  // Get buffer size
  GLint buffer_size = 0;
  glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
  glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &buffer_size);
  
  std::cout << "Buffer ID: " << vbo_id << std::endl;
  std::cout << "Buffer Size: " << buffer_size << " bytes" << std::endl;
  
  if(buffer_size == 0){
    std::cout << "Buffer is empty!" << std::endl;
    return;
  }
  
  // Calculate number of elements
  int num_floats = buffer_size / sizeof(float);
  int num_vertices = num_floats / CHAR_RENDER_SIZE;
  
  std::cout << "Number of floats: " << num_floats << std::endl;
  std::cout << "Number of vertices: " << num_vertices << std::endl;
  std::cout << "Vertex size: " << CHAR_RENDER_SIZE << " floats per vertex" << std::endl;
  std::cout << "----------------------------------------" << std::endl;
  
  // Retrieve buffer data
  float* buffer_data = new float[num_floats];
  glGetBufferSubData(GL_ARRAY_BUFFER, 0, buffer_size, buffer_data);
  
  std::cout << "Vertex Data Layout:" << std::endl;
  std::cout << "Position(x,y) | TexCoord(u,v) | Width | Height | Offset(x,y)" << std::endl;
  std::cout << "----------------------------------------" << std::endl;
  
  for(int i = 0; i < num_vertices; i++){
    int base_idx = i * CHAR_RENDER_SIZE;
    std::cout << "Vertex " << i << ": ";
    std::cout << std::fixed;
    std::cout.precision(2);
    
    // Position (x, y)
    std::cout << "(" << buffer_data[base_idx] << ", " << buffer_data[base_idx + 1] << ") | ";
    
    // Texture coordinates (u, v)
    std::cout << "(" << buffer_data[base_idx + 2] << ", " << buffer_data[base_idx + 3] << ") | ";
    
    // Width
    std::cout << buffer_data[base_idx + 4] << " | ";
    
    // Height
    std::cout << buffer_data[base_idx + 5] << " | ";
    
    // Offset (x, y)
    std::cout << "(" << buffer_data[base_idx + 6] << ", " << buffer_data[base_idx + 7] << ")";
    
    std::cout << std::endl;
  }
  
  std::cout << "========================================" << std::endl;
  
  delete[] buffer_data;
}

void print_projection_matrix(){
  std::cout << "\n========================================" << std::endl;
  std::cout << "Projection Matrix (Uniform Location 0)" << std::endl;
  std::cout << "========================================" << std::endl;
  
  const float* matrix_ptr = glm::value_ptr(orthogonal);
  
  std::cout << "Matrix values (column-major order):" << std::endl;
  std::cout << std::fixed;
  std::cout.precision(6);
  
  for(int row = 0; row < 4; row++) {
    std::cout << "Row " << row << ": ";
    for(int col = 0; col < 4; col++) {
      // GLM uses column-major storage, so matrix[col][row]
      std::cout << matrix_ptr[col * 4 + row];
      if(col < 3) std::cout << ", ";
    }
    std::cout << std::endl;
  }
  
  std::cout << "\nglm::ortho parameters used:" << std::endl;
  std::cout << "Left: 0, Right: 1920" << std::endl;
  std::cout << "Bottom: 1080, Top: 0" << std::endl;
  std::cout << "Near: -10, Far: 10" << std::endl;
  
  std::cout << "\nWith input position (300, 300):" << std::endl;
  glm::vec4 test_pos(300, 300, 0, 1);
  glm::vec4 result = orthogonal * test_pos;
  std::cout << "Transform result: (" << result.x << ", " << result.y << ", " << result.z << ", " << result.w << ")" << std::endl;
  std::cout << "NDC coordinates: (" << result.x/result.w << ", " << result.y/result.w << ")" << std::endl;
  
  std::cout << "========================================" << std::endl;
}

void print_texts_data(){
  std::cout << "\n========================================" << std::endl;
  std::cout << "Texts Map Data" << std::endl;
  std::cout << "========================================" << std::endl;
  
  if(texts.empty()){
    std::cout << "No texts created yet!" << std::endl;
    std::cout << "========================================" << std::endl;
    return;
  }
  
  std::cout << "Total texts: " << texts.size() << std::endl;
  std::cout << "Next text ID: " << text_id << std::endl;
  std::cout << "----------------------------------------" << std::endl;
  
  // Helper function to convert u32string to regular string for display
  std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
  
  for(const auto& pair : texts){
    unsigned int id = pair.first;
    const Text& text_data = pair.second;
    
    std::cout << "Text ID: " << id << std::endl;
    
    // Convert u32string to displayable string
    std::string display_text;
    try {
      display_text = converter.to_bytes(text_data.text);
    } catch(...) {
      display_text = "[conversion failed]";
    }
    
    std::cout << "  Content: \"" << display_text << "\"" << std::endl;
    std::cout << "  Length: " << text_data.text.length() << " characters" << std::endl;
    std::cout << "  Position: (" << text_data.coordinates.x << ", " << text_data.coordinates.y << ")" << std::endl;
    std::cout << "  Center: (" << text_data.center_coordinates.x << ", " << text_data.center_coordinates.y << ")" << std::endl;
    std::cout << "  Scale: " << text_data.scale_constant << "x" << std::endl;
    std::cout << "  Rotation: " << text_data.rotation << " radians" << std::endl;
    std::cout << "  Buffer range: [" << text_data.buffer_indices.x << ", " << text_data.buffer_indices.y << ")" << std::endl;
    
    // Calculate buffer size for this text
    int buffer_chars = (text_data.buffer_indices.y - text_data.buffer_indices.x) / CHAR_RENDER_SIZE;
    std::cout << "  Buffer characters: " << buffer_chars << std::endl;
    
    std::cout << "  ---" << std::endl;
  }
  
  std::cout << "CPU Array size: " << cpu_side_array.size() << " floats" << std::endl;
  std::cout << "Characters in buffer: " << cpu_side_array.size() / CHAR_RENDER_SIZE << std::endl;
  std::cout << "VBO size: " << vbo_size << " bytes" << std::endl;
  
  std::cout << "========================================" << std::endl;
}


