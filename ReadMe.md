# Conan

```bash
make conan
```

安装到 `~/.conan/data/Perfetto/<VERSION>/` 目录下.

# CMake
## from package

先用 `cmake --install` 安装, 再在你的项目中

```cmake
add_executable(YourTarget)
target_link_libraries(YourTarget Perfetto::perfetto)

find_package(Perfetto)
```

## from source

```cmake
add_subdirectory(/path/to/this/repo)

add_executable(YourTarget)
target_link_libraries(YourTarget Perfetto::perfetto)
```
