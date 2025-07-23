//use tree_sitter::{InputEdit, Language, Parser, Point};
mod c_side;
use glfw::{fail_on_errors, Action, Context, Key, WindowEvent};
use std::ffi::CString;

fn main() {
    let mut glfw = glfw::init(fail_on_errors!()).unwrap();
    glfw.window_hint(glfw::WindowHint::ContextVersion(4, 5));
    glfw.window_hint(glfw::WindowHint::Samples(Some(4)));
    let (mut window, events) = glfw.create_window(1000, 1080, "RustGL", glfw::WindowMode::Windowed)
        .expect("Failed to create window!");
    window.make_current();
    window.set_key_polling(true);
    // the supplied function must be of the type:
    // `&fn(symbol: &'static str) -> *const std::os::raw::c_void`
    gl::load_with(|s| window.get_proc_address(s));
    unsafe {
        gl::Enable(gl::BLEND);
        gl::Enable(gl::MULTISAMPLE);
        gl::BlendFunc(gl::SRC_ALPHA, gl::ONE_MINUS_SRC_ALPHA);
    }

    unsafe{
        c_side::initialize_render();
        c_side::new_sprite_renderer();
        c_side::initialize_font_renderer();
        c_side::initialize_font(CString::new("include/font/Unnamed.fnt").expect("Error").as_ptr());
        c_side::create_text(CString::new("Something").expect("Error").as_ptr(), 600.0, 700.0);

        let line_id = c_side::new_line(200.0, 200.0, 40.0, 3.0, 57.0, 0.0, 0.0, 0.0);
        let circle_id = c_side::new_circle(100.0, 100.0, 30.0, 2.0, 1.0, 0.0, 0.0);
        let triangle_id = c_side::new_triangle(150.0, 150.0, 50.0, 50.0, 45.0, 0.0, 1.0, 0.0);
        let rectangle_id = c_side::new_rectangle(250.0, 250.0, 60.0, 40.0, 2.0, 0.0, 0.0, 1.0);
        let square_id = c_side::new_square(300.0, 300.0, 50.0, 2.0, 1.0, 1.0, 0.0);

        println!("{0}", gl::GetError());
    }

    while !window.should_close() {
        window.swap_buffers();
    // Poll for and process events
        glfw.poll_events();

        for (_, event) in glfw::flush_messages(&events) {
            match event {
                glfw::WindowEvent::Key(Key::Escape, _, Action::Press, _) => {
                    window.set_should_close(true)
                },
                glfw::WindowEvent::Key(Key::Space, _, Action::Press, _) => {
                    println!("Good Job\n");
                }
                _ => {},
            }
        }
        unsafe{
            gl::ClearColor(0.0, 1.0, 1.0, 1.0);
            gl::Clear(gl::COLOR_BUFFER_BIT);
            c_side::render_text();
            c_side::draw_sprites();
            c_side::destroy_renderer();
        }
    }
}

fn handle_window_event(window:&mut glfw::Window, event: glfw::WindowEvent){
    match event {
        glfw::WindowEvent::Key(Key::Escape, _, Action::Press, _) => {
            window.set_should_close(true)
        },
        glfw::WindowEvent::Key(Key::Space, _, Action::Press, _) => {
            println!("Good Job\n");
        }
        _ => {},
    }
}
