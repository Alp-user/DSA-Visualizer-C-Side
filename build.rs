fn main() {
    println!("cargo:rerun-if-changed=include/src/my_font_load.cpp");
    println!("cargo:rerun-if-changed=include/src/glad.c");
    println!("cargo:rerun-if-changed=include/src/GLDebug.cpp");
    println!("cargo:rerun-if-changed=include/src/shader.cpp");
    println!("cargo:rerun-if-changed=include/src/sprites.cpp");
    println!("cargo:rerun-if-changed=include/src/sprites_single.cpp");
    println!("cargo:rerun-if-changed=include/src/stb_image.c");
    println!("cargo:rerun-if-changed=include/src/initializer.cpp");
    
    println!("cargo:rustc-link-lib=GL");
    println!("cargo:rustc-link-lib=glfw");

    cc::Build::new()
        .cpp(true)
        .include("include/include")
        .files(["include/src/my_font_load.cpp",
        "include/src/glad.c",
        "include/src/GLDebug.cpp",
        "include/src/shader.cpp",
        "include/src/sprites.cpp",
        "include/src/sprites_single.cpp",
        "include/src/stb_image.c",
        "include/src/initializer.cpp"])
        .compile("rendering");

}
