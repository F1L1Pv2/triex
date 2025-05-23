# TRIEX - Simple Vulkan Triangle Example
vulkan triangle example made as simple as possible in C with nob.c build system

![LinuxOutput](https://github.com/user-attachments/assets/1f4b5a01-9e6b-40b1-9130-f5a24463c6ba)


# How to BUILD?

--------------

### LINUX

- Install following stuff
```sh
    sudo apt install libx11-dev libxrandr-dev libvulkan-dev libshaderc-dev vulkan-validationlayers vulkan-tools
```
- Compile it like this
```sh
    clang -o nob nob.c
    ./nob release run
```

### WINDOWS

- Download VulkanSDK using this site https://vulkan.lunarg.com/
- Download clang (for clang you will also need msvc not sure why) (or use mingw)
- Compile it like this
```sh
    clang -o nob.exe nob.c
    ./nob release run
```
> [!NOTE] 
> if you istalled VulkanSDK in a folder different than `C:/VulkanSDK` specify your folder at the top of `nob.c` `VULKAN_SDK_SEARCH_PATH` macro

--------------

## Things of note

> [!NOTE]
> If you are using compiler different than `clang` specify it in nob.c in `COMPILER_NAME` macro

> [!NOTE] 
> If you dont specify `release` in during running nob it will build with debug information and `run` option will run app after successfull compilation

> [!NOTE]
> You only need to compile nob.c once
