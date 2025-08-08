#include <GLDebug.h>
#include <font_loader.h>
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/vector_float2.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <shader.h>


#define  line_skip(name)  fscanf(name, "%*[^\n]\n")
#define get_char_info(i) Fonts[active_font].FontArray[i]
#define CURRENT_FONT Fonts[active_font]
#define MAX_PAGES 15 // have a setting to set these numbers up at the start. If I want extra performance only one e.g.
#define PNG_WIDTH_HEIGHT 512 //SET it to 1024 so that you can upload any font size smaller than 1024 with glTexSubImage3D
#define CHAR_RENDER_SIZE 9
#define ALLOCATED_STORAGE sizeof(float)*CHAR_RENDER_SIZE*100 //100 characters
//Make those upper case
using std::unordered_map;
using glm::mat4;
using glm::vec2;

static FontType Fonts[10];
static size_t number_of_fonts_loaded = 0;
static int active_font = -1;
static unsigned int texture_object;
static unsigned int texture_offsets[10] = {0};
static unsigned int VBO, VEO, VAO;
static unsigned int vertex_shader_object, fragment_shader_object, shader_program;
static unsigned int indices[] = {0,1,2,2,3,0};
static unsigned int screen_width, screen_height;
static char_cache cached_chars;
static float font_size = 0.5f;
static cpu_char_info cpu_text_array;
static mat4 orthogonal_matrix;
static float color[3];
static unordered_map<unsigned int, Text> texts;
static unsigned int index_hash;
static unsigned int buffer_size;

static const char *vertex_shader = "#version 450 core\n"
  "layout (location = 0) in vec3 tex_coords;\n"
  "layout (location = 1) in vec2 scale_vec;\n"
  "layout (location = 2) in vec2 pos_coords;\n"
  "layout (location = 3) in float font_size;\n"
  "layout (location = 4) in float rotation;\n"
  "layout (location = 0) out vec3 tex_out;\n"
  "layout (location = 0) uniform mat4 projection;\n"
  "layout (location = 1) uniform vec2 width_height_png;\n"
   "vec2 quad_pos[6] = vec2[](\n"
   "vec2(0, 0),\n"
   "vec2(1, 0),\n"
   "vec2(1, 1),\n"
   "vec2(1, 1),\n"
   "vec2(0, 1),\n"
   "vec2(0, 0)\n"
   ");\n"
  "void main() {\n"
    "mat2 rot_mat = mat2(cos(rotation), -sin(rotation), sin(rotation), cos(rotation));\n"
    "vec2 scales = scale_vec * font_size;\n"
    "vec2 scaled_pos = quad_pos[gl_VertexID] * scales;\n"
    "vec2 rot_pos = (rot_mat * (scaled_pos)) ;\n"
    "vec2 trans_pos = rot_pos + pos_coords;\n"
    "if(gl_VertexID == 0 || gl_VertexID == 5){\n"
      "gl_Position =   projection * vec4(trans_pos, 0.0, 1.0);\n"
      "tex_out = tex_coords;\n"
    "}\n"
    "else if(gl_VertexID == 1){\n"
      "gl_Position =   projection * vec4(trans_pos, 0.0, 1.0);\n"
      "tex_out =  vec3(tex_coords.x + (scale_vec.x/width_height_png.x), tex_coords.y, tex_coords.z);\n"
    "}\n"
    "else if(gl_VertexID == 2 || gl_VertexID == 3){\n"
      "gl_Position =   projection * vec4(trans_pos, 0.0, 1.0);\n"
      "tex_out = vec3(tex_coords.x + (scale_vec.x/width_height_png.x), tex_coords.y - (scale_vec.y/width_height_png.y), tex_coords.z);\n"
    "}\n"
    "else if(gl_VertexID == 4){\n"
      "gl_Position =   projection * vec4(trans_pos, 0.0, 1.0);\n"
      "tex_out =  vec3(tex_coords.x, tex_coords.y - (scale_vec.y/width_height_png.y), tex_coords.z);\n" // should I account for the offset
    "}\n"
  "}\n";


static const char *fragment_shader = "#version 450 core\n"
  "layout (location = 0) in vec3 tex_out;\n"
  "layout (location = 0) out vec4 output_color;\n"
  "layout (binding = 10) uniform sampler2DArray texture_object;\n"
  "layout (location = 2) uniform vec3 color;"
  "void main() {\n"
    //"output_color = vec4(1.0, 0.0, 0.0, 1.0); // \n"
    "vec4 temp = texture(texture_object, tex_out);\n"
    "output_color =  temp + (vec4(color,1.0) * temp.a);\n"
  "}\n";

const char* long_str = "The sun rises early in the morning, casting a warm glow over the hills.\n"
"Birds chirp happily, greeting the new day with cheerful songs and fluttering wings.\n"
"A gentle breeze sweeps through the trees, rustling leaves in a soothing dance.\n"
"Children laugh and play in the park, their voices echoing with pure joy.\n"
"The river flows steadily, reflecting the sky and carrying small boats along.\n"
"Farmers tend to their fields, planting seeds and nurturing crops with care.\n"
"In the city, people rush to work, sipping coffee and checking their phones.\n"
"A dog barks in the distance, alerting its owner to a passing stranger.\n"
"Clouds drift lazily across the sky, forming shapes that spark the imagination.\n"
"At night, stars twinkle brightly, lighting up the dark with their gentle glow.\n";

void free_font_resources(){
  size_t i,j;
  glDeleteBuffers(1, &VBO);
  glDeleteBuffers(1, &VEO);
  glDeleteVertexArrays(1, &VAO);
  glDeleteTextures(1, &texture_object);
  glDeleteProgram(shader_program);
  free(cached_chars.current_char_array);
  for(i=0; i<number_of_fonts_loaded; i++){
    for(j = 0; j < Fonts[i].number_of_pages ; j++){//number_of_pages == length of atlases
      free(Fonts[i].atlases[j]);
    }
    free(Fonts[i].atlases); 
  }
  /*
  for(i=0; i<cpu_text_array.size; i++){
    free(cpu_text_array.text_array[i].text);
  }
  free(cpu_text_array.text_array);*/
  for(auto &current : texts){
    free(current.second.text);
  }
}

void print_array_buffer_size(GLuint vbo_id) {
    GLint size;
    glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
}

void initialize_font_renderer(){
  //VARIABLES
  //100 Characters default
  cached_chars.capacity = 100 * CHAR_RENDER_SIZE; cached_chars.index = cached_chars.rendered_index = 0;
  buffer_size = 100 * CHAR_RENDER_SIZE;
  cached_chars.current_char_array = (float *) malloc(sizeof(float) * cached_chars.capacity);
  if(cached_chars.current_char_array == NULL){
    assert(false);
  }

  cpu_text_array.size = cpu_text_array.capacity = cpu_text_array.index =  index_hash = 0;

  //TEXTURE
  glGenTextures(1, &texture_object);
  glActiveTexture(GL_TEXTURE10);
  glBindTexture(GL_TEXTURE_2D_ARRAY, texture_object);
  glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, 512, 511, MAX_PAGES);

  //SHADERS
  glCreateVertexArrays(1, &VAO); glBindVertexArray(VAO);
  glGenBuffers(1, &VBO); glGenBuffers(1,&VEO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO); glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VEO);
  glBufferData(GL_ARRAY_BUFFER,buffer_size * sizeof(float) , NULL, GL_DYNAMIC_DRAW);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
  glCheckError();

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, CHAR_RENDER_SIZE * sizeof(float),(void *) 0);
  glEnableVertexAttribArray(0);
  glVertexAttribDivisor(0,1);

  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, CHAR_RENDER_SIZE * sizeof(float), (void *)(3* sizeof(float)));
  glEnableVertexAttribArray(1);
  glVertexAttribDivisor(1,1);

  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, CHAR_RENDER_SIZE * sizeof(float),(void *)(5* sizeof(float)));
  glEnableVertexAttribArray(2);
  glVertexAttribDivisor(2,1);

  glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, CHAR_RENDER_SIZE * sizeof(float),(void *)(7* sizeof(float)));
  glEnableVertexAttribArray(3);
  glVertexAttribDivisor(3,1);

  glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, CHAR_RENDER_SIZE * sizeof(float),(void *)(8* sizeof(float)));
  glEnableVertexAttribArray(4);
  glVertexAttribDivisor(4,1);
  glCheckError();

  compile(vertex_shader, fragment_shader, &shader_program);
  glCheckError();

  //Uniform variables

  //Orthogonal matrix
  glUseProgram(shader_program);
  glCheckError();

  orthogonal_matrix = glm::ortho(0.0f,1920.0f, 1080.0f, 0.0f);
  glCheckError();
  glUniformMatrix4fv(0, 1, GL_FALSE,glm::value_ptr(orthogonal_matrix));

  
  //Texture unit
  glCheckError();
  glUniform1i(glGetUniformLocation(shader_program, "texture_object"), 10);

  //color
  glCheckError();
  color[0] = 1.0f; color[1] = 1.0f; color[2] = 1.0f;
  glCheckError();
  glUniform3fv(2, 1, color);
  glCheckError();
  vs_off();
}

void change_active_font(size_t number){
  vs();
  active_font = number;
  float width_height[2] = {(float)CURRENT_FONT.scale_x, (float)CURRENT_FONT.scale_y};
  glUniform2fv(1,1,width_height);
  vs_off();
}


void resize_window(float width, float height){
  vs();
  screen_width = width; screen_height = height;
  orthogonal_matrix = glm::ortho(0.0f,width,0.0f,height);
  glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(orthogonal_matrix));
  text_cleanup();
  vs_off();
}
void color_font(float r, float g, float b){
  vs();
  color[0] = r; color[1] = g; color[2] = b;
  glUniform3fv(2, 1, color);
  vs_off();
}

void initialize_font(const char* file_name){
  vs();
  FILE *fileHandle = fopen(file_name, "r");
  size_t path_name_size;
  assert(fileHandle);

  char path_name[150];
  active_font = number_of_fonts_loaded;

  CURRENT_FONT.size = 1;
  CURRENT_FONT.atlases = NULL;

  
  if(const char* path_ptr = strrchr(file_name, '/')){
    int i = 0;
    const char* current = file_name;
    for(;path_ptr!=current;i++,current++){
      path_name[i] = *current;
    }
    path_name[i] = '/'; path_name[i+1] = 0;
    path_name_size = i+1;
  }
  else path_name[0] = 0;
  strcpy(CURRENT_FONT.directory, path_name);


  line_skip(fileHandle);
  fscanf(fileHandle, "%*[^=]=%d %*s %*[^=]=%d %*[^=]=%d %*[^=]=%d %*s", &CURRENT_FONT.line_height, &CURRENT_FONT.scale_x, 
         &CURRENT_FONT.scale_y,&CURRENT_FONT.number_of_pages); 
  line_skip(fileHandle);

  CURRENT_FONT.is_alpha = 1;
  CURRENT_FONT.atlases = (char **) malloc(sizeof(char *) * (CURRENT_FONT.number_of_pages));
  assert(CURRENT_FONT.atlases);

  for(size_t i = 0; i<CURRENT_FONT.number_of_pages ; i++){
    CURRENT_FONT.atlases[i] = NULL;
    CURRENT_FONT.atlases[i] = (char *) malloc(sizeof(char) * (path_name_size + 20));
    if(CURRENT_FONT.atlases[i] == NULL){
      assert(false);
    }
    strcpy(CURRENT_FONT.atlases[i], path_name);
    //fscanf(fileHandle, "%*s %*s %*[^\"]\"%[^\"]\"",  );
    fscanf(fileHandle, "%*s %*s %*[^\"]\"%[^\"]\"", CURRENT_FONT.atlases[i] + path_name_size);
  }

  fscanf(fileHandle, "%*[^=]=%zu", &CURRENT_FONT.number_of_lines);

  for(size_t i = 0; i<CURRENT_FONT.number_of_lines; i++){
    unsigned int current_char;
    fscanf(fileHandle, "%*[^=]=%d", 
           &current_char);
    fscanf(fileHandle, "%*[^=]=%d %*[^=]=%d %*[^=]=%d %*[^=]=%d %*[^=]=%d %*[^=]=%d %*[^=]=%d %*[^=]=%d %*s", 
            &CURRENT_FONT.FontArray[current_char].posx, &CURRENT_FONT.FontArray[current_char].posy, &CURRENT_FONT.FontArray[current_char].width,
           &CURRENT_FONT.FontArray[current_char].height, &CURRENT_FONT.FontArray[current_char].xoff, &CURRENT_FONT.FontArray[current_char].yoff, &CURRENT_FONT.FontArray[current_char].x_advance,
           &CURRENT_FONT.FontArray[current_char].page);
    CURRENT_FONT.FontArray[current_char].id = current_char;
  }
  fclose(fileHandle);
  generate_textures(&CURRENT_FONT);
  number_of_fonts_loaded++;

  glUseProgram(shader_program);
  float width_height[2]= {(float)CURRENT_FONT.scale_x, (float)CURRENT_FONT.scale_y};
  glUniform2fv(1,1, width_height);
  vs_off();
  glCheckError();
}

void generate_textures(FontType *curfont){
  vs();
  unsigned char *data;
  size_t offset;
  size_t i = 0;
  int t1,t2,t3; //trash values for stbi_load function;
  if(active_font != 0){
    texture_offsets[active_font] = texture_offsets[active_font-1] + Fonts[active_font-1].number_of_pages;
  }  
  offset = texture_offsets[active_font];
  glActiveTexture(GL_TEXTURE10);
  glBindTexture(GL_TEXTURE_2D_ARRAY, texture_object);
  stbi_set_flip_vertically_on_load(1);
  for(i = 0;i<curfont->number_of_pages; i++){
    data = stbi_load(curfont->atlases[i], &t1, &t2, &t3, 4);
    assert(data);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, offset + i,t1, t2,1,GL_RGBA, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data); data = NULL;
  }
  vs_off();
  glCheckError();
}


void add_char_info(const char *curchar, float glo_x, float glo_y, float font_size, float rotation){
  float nposx, nposy;
  const char_info* cur_char = &get_char_info(*curchar);

  nposx = cur_char->posx /  ((float)CURRENT_FONT.scale_x);
  nposy = cur_char->posy /  ((float)CURRENT_FONT.scale_y);

  //assert(cached_chars.index < cached_chars.capacity);

  cached_chars.current_char_array[cached_chars.index++] = nposx;
  cached_chars.current_char_array[cached_chars.index++] = (1 - nposy); // + or -
  cached_chars.current_char_array[cached_chars.index++] = get_char_info(*curchar).page;
  
  cached_chars.current_char_array[cached_chars.index++] = cur_char->width;
  cached_chars.current_char_array[cached_chars.index++] = cur_char->height;

  cached_chars.current_char_array[cached_chars.index++] = glo_x ;
  cached_chars.current_char_array[cached_chars.index++] = glo_y ;

  cached_chars.current_char_array[cached_chars.index++] = font_size;
  cached_chars.current_char_array[cached_chars.index++] = rotation;
}

static unsigned int string_length(const char* string){
  int i = 0;
  int number_of_speacials = 0;
  for(;string[i] != 0; i++){
    if(string[i] == 10) number_of_speacials++;
  }
  return i - number_of_speacials;
}

static unsigned int string_length_special(const char *string, unsigned int *special_count){
  int i = 0;
  int number_of_speacials = 0;
  for(;string[i] != 0; i++){
    if(string[i] == 10) number_of_speacials++;
  }
  *special_count = number_of_speacials;
  return i - number_of_speacials;
}

void add_string_info(const char *string, float glo_x, float glo_y,float font_size, float rotation){
  const char* current = string;
  const char_info* cur_char_info = NULL;
  float calc_x_advance = 0;
  float calc_y_advance = 0;
  float xoff, yoff;
  float original_glo_x = glo_x;
  float original_glo_y = glo_y;
  glm::mat2 rotation_matrix(
    cos(rotation), -sin(rotation),
    sin(rotation), cos(rotation)
  );
  //float original_glo_y = glo_y;

  for(;*current != 0;current++){
    while(cached_chars.capacity <= cached_chars.index){
      cached_chars.capacity *= 2;
      cached_chars.current_char_array = (float *) realloc(cached_chars.current_char_array,sizeof(float) * cached_chars.capacity);
    }
    switch (*current){
      case 10:{
        calc_y_advance = CURRENT_FONT.line_height * font_size;
        glm::vec2 rotated_advance = rotation_matrix * glm::vec2(0, calc_y_advance);
        glo_x = original_glo_x;
        glo_y = original_glo_y;
        glo_x += rotated_advance.x;
        glo_y += rotated_advance.y;
        original_glo_x = glo_x;
        original_glo_y = glo_y;
        break;

      } 
      default:{
        calc_x_advance = (get_char_info((unsigned int) *current).x_advance * font_size);
        cur_char_info = &get_char_info((unsigned int) *current);
        xoff = cur_char_info->xoff * font_size;
        yoff = cur_char_info -> yoff * font_size;
        glm::vec2 rot_offs = rotation_matrix * glm::vec2(xoff, yoff) ;
        glm::vec2 rot_advance = rotation_matrix * glm::vec2(calc_x_advance, 0);
        add_char_info(current, glo_x + rot_offs.x , glo_y +  rot_offs.y, font_size, rotation); 
        glo_x += rot_advance.x;
        glo_y += rot_advance.y;
      }
    }
  }
}

unsigned int create_centered_text(const char *copy_text, float cx, float cy, float width
    , float height){
  glCheckError();
  float calc_width = 0;
  float current_line_width = 0;
  unsigned int number_of_lines = 1;
  for(const char* current = copy_text;*current!=0;current++){
        if(*current == 10){//newline
          number_of_lines++;
          if(current_line_width > calc_width) {
            calc_width = current_line_width;
          }
          current_line_width = 0;
          continue;
        }
        current_line_width += get_char_info(*current).x_advance;
  }
  // Check last line in case it's the widest
  if(current_line_width > calc_width) {
    calc_width = current_line_width;
  }

  float width_font = (float) width/calc_width;
  float height_font = (float) height/(CURRENT_FONT.line_height * number_of_lines);

  glCheckError();
  if(width_font > height_font){
    return create_text(copy_text, cx - (calc_width * height_font)/2, cy - (CURRENT_FONT.line_height * number_of_lines * height_font)/2, height_font);
  }
  else{
    return create_text(copy_text, cx - (calc_width * width_font)/2, cy - (CURRENT_FONT.line_height * number_of_lines * width_font)/2, width_font);
  }
}

unsigned int create_text_height(const char *copy_text, float glo_x, float glo_y, unsigned int pixel_line_height){
  glCheckError();
  float font_size = (float) pixel_line_height / CURRENT_FONT.line_height;
  return create_text(copy_text, glo_x, glo_y,font_size);
}

unsigned int create_text(const char *copy_text, float glo_x, float glo_y, float font_size){
  glCheckError();
  vs();
  glCheckError();

  texts.insert(std::pair<unsigned int, Text>(index_hash,{NULL,0,0, 0,0,0,0,0}));
  Text &current = texts[index_hash];
  unsigned int special_count;
  unsigned int ns_length = string_length_special(copy_text, &special_count);
  current.length = ns_length;//does not count newlines
  current.start_vbo = cached_chars.index; 
  current.end_vbo = current.start_vbo + (current.length * CHAR_RENDER_SIZE) - 1;
  current.x = glo_x;
  current.y = glo_y;
  current.font_size = font_size;
  current.rotation = 0.0f;
  current.text = NULL;
  current.text = (char *) malloc(sizeof(char) * (current.length +special_count+ 1) );
  if(current.text == NULL){
    printf("Allocation failed");
    assert(false);
  }
  strcpy(current.text, copy_text);
  glCheckError();
  add_string_info(current.text, glo_x, glo_y,font_size, 5.0f);

  glCheckError();
  load_new();
  glCheckError();
  vs_off();
  glCheckError();
  return index_hash++;
}

void modify_text(unsigned int text_id, const char *new_text){
  vs();

  auto cur_itr = texts.find(text_id);
  assert(cur_itr != texts.end());
  Text &current = cur_itr->second;

  unsigned int i = 0;
  unsigned int special_count;
  unsigned int ns_length = string_length_special(new_text, &special_count);
  unsigned int old_length = current.length;
  unsigned int last_index = cached_chars.index;
  unsigned int sizeof_float = sizeof(float);
  unsigned int shifting = 0;

  free(current.text);
  current.text = NULL;
  current.text = (char *) malloc(sizeof(char) * (ns_length + special_count + 1));
  assert(current.text);
  current.length = ns_length;
  strcpy(current.text, new_text);

  if(old_length < ns_length){
    shifting = (ns_length - old_length) * CHAR_RENDER_SIZE;
    while(cached_chars.capacity < shifting + cached_chars.index){
      cached_chars.capacity *= 2;
      cached_chars.current_char_array = (float *) realloc(cached_chars.current_char_array, sizeof(float) * cached_chars.capacity );
    }
    //old end_vbo
    for(i = cached_chars.index + shifting - 1; i > current.end_vbo + shifting; i--){
      cached_chars.current_char_array[i] = cached_chars.current_char_array[i-shifting];
    }
    //updated here
    current.end_vbo = i;
    for(auto cur = texts.begin(); cur != texts.end(); cur++){
      if(cur->second.start_vbo > current.start_vbo){
        cur->second.start_vbo += shifting;
        cur->second.end_vbo += shifting;
      }
    }
  }
  cached_chars.index = current.start_vbo;
  add_string_info(new_text, current.x,current.y,current.font_size, current.rotation);
  for(i = cached_chars.index; i <= current.end_vbo; i++){
    cached_chars.current_char_array[i] = 0;
  }
  cached_chars.index = last_index + shifting;
  cached_chars.rendered_index = cached_chars.index;
  buffer_size = cached_chars.index;
  glBufferData(GL_ARRAY_BUFFER, sizeof_float * buffer_size, cached_chars.current_char_array, GL_DYNAMIC_DRAW);
  vs_off();
}

Text *get_text(unsigned int text_id){
  auto it = texts.find(text_id);
  assert(it != texts.end());
  return &(it->second);
}

void delete_text(unsigned int text_id){
  modify_text(text_id, "");
}

void text_cleanup(){
  vs();
  unsigned int i;
  int nonempty_texts = 0;
  int total_text_length = 0;
  Text *prev = NULL;



  std::vector<unsigned int> collect_keys;
  for(auto &current : texts){
    auto &data = current.second;
    if(data.length != 0){//if they are not the same
      if(nonempty_texts == 0){
        data.start_vbo = 0;
        data.end_vbo =  data.length * CHAR_RENDER_SIZE - 1;
      }
      else{
        data.start_vbo = prev->end_vbo + 1;
        data.end_vbo = data.start_vbo + data.length * CHAR_RENDER_SIZE - 1;
      }
      prev = &data;
      total_text_length += data.length;
      nonempty_texts++;
    }  
    else{
      free(data.text);
      collect_keys.push_back(current.first);
    }
  }

  for(unsigned int key : collect_keys){
    texts.erase(key);
  }

  array_from_texts(total_text_length);
  texts.rehash(0);

  buffer_size = total_text_length * CHAR_RENDER_SIZE;
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * buffer_size, cached_chars.current_char_array, GL_DYNAMIC_DRAW);
  cached_chars.rendered_index = cached_chars.index;
  vs_off();
}

void array_from_texts(unsigned int total_size){
  float *new_array = NULL;
  float *old_array = cached_chars.current_char_array;

  if(total_size == 0){
    cached_chars.capacity = 100 * CHAR_RENDER_SIZE;
  }else{
    cached_chars.capacity = total_size * CHAR_RENDER_SIZE;
  }
  new_array = (float *) malloc(sizeof(float) * cached_chars.capacity);
  if(new_array == NULL){
    assert(false);
  }

  cached_chars.current_char_array = new_array;
  cached_chars.index = cached_chars.rendered_index = 0;
  

  for(auto &current : texts){
    auto &data = current.second;
    add_string_info(data.text, data.x, data.y, data.font_size, data.rotation);
  }

  free(old_array);
}


void move_text(unsigned int text_id, float glo_x, float glo_y){
  vs();
  auto it = texts.find(text_id);
  unsigned int i;

  if(it == texts.end()){
    assert(false);
  }
  //int i = 0;
  //
  float move_x = glo_x - it->second.x;
  float move_y = glo_y - it->second.y;

  it->second.x = glo_x;
  it->second.y = glo_y;

  //Every character is in the form 0,1,2 for texture coordinates
  // 3,4 for scaling
  // 5,6 for global positions
  for(i = it->second.start_vbo + 5; i < it->second.end_vbo;
  i += CHAR_RENDER_SIZE){
    cached_chars.current_char_array[i] += move_x;
    cached_chars.current_char_array[i+1] += move_y;
  }
  glBufferSubData(GL_ARRAY_BUFFER, sizeof(float) * it->second.start_vbo,
    sizeof(float)* (it->second.end_vbo-it->second.start_vbo + 1),
    cached_chars.current_char_array + it->second.start_vbo);
  vs_off();
}

void rotate_text_centered(unsigned int text_id, float angle){
  vs();
  auto it = texts.find(text_id);

  if(it == texts.end()){
    assert(false);
  }
  
  glm::mat2 rotation_matrix(
    cos(angle), -sin(angle),
    sin(angle), cos(angle)
  );
  float calc_width = 0;
  float calc_height;
  float current_line_width = 0;
  unsigned int number_of_lines = 1;
  for(const char* current = it->second.text;*current!=0;current++){
        if(*current == 10){//newline
          number_of_lines++;
          if(current_line_width > calc_width) {
            calc_width = current_line_width;
          }
          current_line_width = 0;
          continue;
        }
        current_line_width += get_char_info(*current).x_advance;
  }
  if(current_line_width > calc_width) {
    calc_width = current_line_width;// /2 for distance to center
  }
  calc_height = (float)CURRENT_FONT.line_height * number_of_lines /2;
  calc_width /= 2;
  glm::vec2 rotated_center = rotation_matrix * glm::vec2(calc_width, calc_height);
  glm::vec2 center_diff = glm::vec2(calc_width, calc_height) - rotated_center;

  it->second.rotation = angle;
  unsigned int current_index = cached_chars.index;
  cached_chars.index = it->second.start_vbo;
  add_string_info(it->second.text, it->second.x + center_diff.x, it->second.y + center_diff.y, it->second.font_size, it->second.rotation);
  cached_chars.index = current_index;

  glBufferSubData(GL_ARRAY_BUFFER, sizeof(float) * it->second.start_vbo,
    sizeof(float)* (it->second.end_vbo-it->second.start_vbo + 1),
    cached_chars.current_char_array + it->second.start_vbo);
  vs_off();
}

void rotate_text(unsigned int text_id, float angle){
  vs();
  auto it = texts.find(text_id);
  unsigned int i;

  if(it == texts.end()){
    assert(false);
  }
  
  it->second.rotation = angle;
  unsigned int current_index = cached_chars.index;
  cached_chars.index = it->second.start_vbo;
  add_string_info(it->second.text, it->second.x, it->second.y, it->second.font_size, it->second.rotation);
  cached_chars.index = current_index;

  glBufferSubData(GL_ARRAY_BUFFER, sizeof(float) * it->second.start_vbo,
    sizeof(float)* (it->second.end_vbo-it->second.start_vbo + 1),
    cached_chars.current_char_array + it->second.start_vbo);
  vs_off();
}

void scale_text(unsigned int text_id, unsigned int pixel_line_height){
  vs();
  auto it = texts.find(text_id);
  unsigned int i;

  if(it == texts.end()){
    assert(false);
  }
  
  it->second.font_size = (float) pixel_line_height / CURRENT_FONT.line_height;
  unsigned int current_index = cached_chars.index;
  cached_chars.index = it->second.start_vbo;
  add_string_info(it->second.text, it->second.x, it->second.y, it->second.font_size, it->second.rotation);
  cached_chars.index = current_index;

  glBufferSubData(GL_ARRAY_BUFFER, sizeof(float) * it->second.start_vbo,
    sizeof(float)* (it->second.end_vbo-it->second.start_vbo + 1),
    cached_chars.current_char_array + it->second.start_vbo);
  vs_off();
}

void load_all(){
  vs();
  cached_chars.rendered_index = cached_chars.index;
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) *(cached_chars.index), cached_chars.current_char_array);
  vs_off();
}

void load_new(){
  vs();
  glCheckError();
  print_array_buffer_size(VBO);
  if(buffer_size >= cached_chars.index){
    glBufferSubData(GL_ARRAY_BUFFER, cached_chars.rendered_index * sizeof(float), 
                    sizeof(float) * (cached_chars.index - cached_chars.rendered_index), 
                    cached_chars.current_char_array + cached_chars.rendered_index);
    cached_chars.rendered_index = cached_chars.index;
  glCheckError();
  }
  else{
    buffer_size = cached_chars.index;
  glCheckError();
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * buffer_size, cached_chars.current_char_array, GL_DYNAMIC_DRAW);
  glCheckError();
    cached_chars.rendered_index = cached_chars.index;
  }
  vs_off();
}

void render_text(){
  vs();
  glDrawArraysInstanced(GL_TRIANGLES, 0, 6,  cached_chars.rendered_index/CHAR_RENDER_SIZE);
  vs_off();

}

void vs(){
  glBindVertexArray(VAO);
  glActiveTexture(GL_TEXTURE10);
  glBindTexture(GL_TEXTURE_2D_ARRAY, texture_object);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VEO);
  glUseProgram(shader_program);
}

void vs_off(){
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glUseProgram(0);
}

//Debug

void print_char_array_debug() {
    vs();
    printf("\n--- Cached Characters Array Debug ---\n");
    printf("Format: [tex_x, tex_y, tex_layer, scale_x, scale_y, pos_x, pos_y, font_size, rotation]\n");
    printf("Total elements: %zu\n", cached_chars.index);
    
    for (size_t i = 0; i < cached_chars.index; i += CHAR_RENDER_SIZE) {
        // Print index
        printf("[%zu]: ", i/CHAR_RENDER_SIZE);
        
        // Print texture coordinates (3 elements)
        printf("tex(%.2f, %.2f, %.0f), ", 
               cached_chars.current_char_array[i], 
               cached_chars.current_char_array[i+1], 
               cached_chars.current_char_array[i+2]);
        
        // Print scale values (2 elements)
        printf("scale(%.2f, %.2f), ", 
               cached_chars.current_char_array[i+3], 
               cached_chars.current_char_array[i+4]);
        
        // Print position (2 elements)
        printf("pos(%.2f, %.2f)", 
               cached_chars.current_char_array[i+5], 
               cached_chars.current_char_array[i+6]);

        printf("font_size(%.2f)", 
               cached_chars.current_char_array[i+7]);

        printf("rotation(%.2f)", 
               cached_chars.current_char_array[i+8]);
        
        printf("\n");
        
        // Check if we've reached the end of valid data
        if (i + CHAR_RENDER_SIZE >= cached_chars.index) {
            break;
        }
    }
    printf("--- End of Debug Output ---\n\n");
    vs_off();
}

void print_char_info(char car) {
    vs();
    const char_info* cur_char = &get_char_info(car);
    printf("\n--- Character Info for '%c' (ID: %u) ---\n", car, cur_char->id);
    printf("Texture Position: (%d, %d)\n", cur_char->posx, cur_char->posy);
    printf("Dimensions: %u x %u\n", cur_char->width, cur_char->height);
    printf("Texture Page: %u\n", cur_char->page);
    printf("Offsets: (%d, %d)\n", cur_char->xoff, cur_char->yoff);
    printf("X Advance: %u\n", cur_char->x_advance);
    printf("-----------------------------\n");
    glCheckError();
    vs_off();
}

void print_font_type(FontType* curfont) {
    vs();
    printf("FontType Information:\n"); printf("Line Height: %u\n", curfont->line_height); printf("Size: %u\n", curfont->size);
    printf("Scale X: %u\n", curfont->scale_x);
    printf("Scale Y: %u\n", curfont->scale_y);
    printf("Number of Pages: %u\n", curfont->number_of_pages);
    printf("Is Alpha: %u\n", curfont->is_alpha);
    printf("Directory: %s\n", curfont->directory[0] ? curfont->directory : "(empty)");

    printf("Atlases:\n");
    for (unsigned int i = 0; i < curfont->number_of_pages; i++) {
        if (curfont->atlases[i]) {
            printf("  Atlas %u: %s\n", i, curfont->atlases[i]);
        } else {
            printf("  Atlas %u: (null)\n", i);
        }
    }

    printf("Font Array (char_info):\n");
    for (unsigned int i = 0; i <95; i++) {
        printf("  Character %u:\n", curfont->FontArray[i].id);
        printf("    ID: %u\n", curfont->FontArray[i].id);
        printf("    Width: %u\n", curfont->FontArray[i].width);
        printf("    Height: %u\n", curfont->FontArray[i].height);
        printf("    Page: %u\n", curfont->FontArray[i].page);
        printf("    X Advance: %u\n", curfont->FontArray[i].x_advance);
        printf("    Pos X: %d\n", curfont->FontArray[i].posx);
        printf("    Pos Y: %d\n", curfont->FontArray[i].posy);
        printf("    X Offset: %d\n", curfont->FontArray[i].xoff);
        printf("    Y Offset: %d\n", curfont->FontArray[i].yoff);
    }
    glCheckError();
    vs_off();
}

void print_vbo_debug() {
    vs();
    printf("\n--- OpenGL Vertex Buffer Debug ---\n");
    printf("Format: [tex_x, tex_y, tex_layer, scale_x, scale_y, pos_x, pos_y, font_size, rotation]\n");
    
    // Get buffer size and usage
    GLint buffer_size;
    glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &buffer_size);
    printf("Buffer Size: %d bytes (%d floats)\n", buffer_size, buffer_size / sizeof(float));
    
    // Print buffer contents (same format as print_char_array_debug)
    printf("\nBuffer Contents:\n");
    float* mapped_data = (float*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
    if(mapped_data) {
        size_t num_elements = buffer_size / sizeof(float);
        for(size_t i = 0; i < num_elements; i += CHAR_RENDER_SIZE) {
            printf("[%zu]: ", i/CHAR_RENDER_SIZE);
            printf("tex(%.2f, %.2f, %.0f), ", mapped_data[i], mapped_data[i+1], mapped_data[i+2]);
            printf("scale(%.2f, %.2f), ", mapped_data[i+3], mapped_data[i+4]);
            printf("pos(%.2f, %.2f)", mapped_data[i+5], mapped_data[i+6]);
            printf("fs(%.2f)\n", mapped_data[i+7]);
            printf("rot(%.2f)\n", mapped_data[i+8]);
            
            if(i + CHAR_RENDER_SIZE >= num_elements) break;
        }
        glUnmapBuffer(GL_ARRAY_BUFFER);
    } else {
        printf("Failed to map buffer for reading\n");
    }
    
    printf("--- End of VBO Debug Output ---\n\n");
    glCheckError();
    vs_off();
}
