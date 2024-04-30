# Culkan
A minimalist wrapper around the Vulkan API to write easy, portable, and (somewhat) safe compute shaders in C!

# Why was this made?
I wanted a compute shader to parallelise one of my projects, it was meant to run on a super-computer, so I needed something portable, fast, and able to run without a window, so Vulkan was the best choice.
Sadly I'm not very experienced with Vulkan, and I would've had a hard time transferring my whole algorithm to it.
So I made this side project that automatically manages variables, memory, and the pipeline, so I could focus on the algorithm itself.

# Advantages
- Very easy to use, and doesn't require much code to get started. I would say it's about as hard as CUDA, so quite easy if you have a basic understanding of parallel programming.
- Fast & Portable, because so is Vulkan.
- Error handling and checking, if you want, there are automatic memory checks, that you can enable or disable, which prevents basic mistakes like writing to a variable that doesn't exist, or not finding the shader file, or Vulkan not being supported. All of that without needing to mess with Validation Layers.
- Can also be used with C++
- No need to create a window, or use a library like GLFW, it's meant to be used without a window, and it's very lightweight.
- Can work with any shader language you like, I chose GLSL in the examples simply because it's the closest to C in terms of syntax
# Disadvantages
- Not very flexible, as it's meant to be simple, it doesn't support many features of Vulkan, and it's not meant to be used for complex graphics, only for compute shaders.

# Should I use this?
If all you need is to send data to the GPU, make it do some calculations, and get the results back, then it's perfect.
For example : doing operations on very large arrays, matrices and so on...
However, if you need to do more complex operations, you should use Vulkan directly, or a wrapper with more features.

# How can I use it?
A basic program will look like this :
- Step 1: Init the Culkan instance with `culkanInit()`, which takes a shader binding layout, and a shader file.
- Step 2: Write to the variables with `culkanWriteBinding()` or `culkanWriteGPUVariable()`.
- Step 3: Finish the setup with `culkanSetup()`.
- Step 4: Run the shader with `culkanRun()`.
- Step 5: Read the results with `culkanReadBinding()` or `culkanReadGPUVariable()`.
- Step 6: Free the resources with `culkanDestroy()`.

So it's quite simple, and definitely less verbose than Vulkan.

# Requirements
- A C or C++ compiler
- Vulkan libraries
- GPU drivers that support Vulkan

You should be able to run on any architecture/OS/distro with these requirements, I tested it on Linux with an AMD GPU (Manjaro, AMD Lucienne).

# Miscelaneous
I am a bit stupid, so if the code is a bit stupid, that's why.
Clang tidy is mad at me for a few reasons, I'll have to fix that. FIXME
The casing and naming style may change in the future, because I'm not sure on if I want to follow Vulkan's naming style or dissociate from it, but it shouldn't be too bad to change it in your code if you ever use this.
I promise I will do better documentation and examples in the future, and clean up some stuff. But I don't want to be too off-schedule with my main project.
I learned a bit about Vulkan, so that's cool.
If this is useful to you, then I'm happy :D
