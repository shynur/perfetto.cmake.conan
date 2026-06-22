有以下 3 种方式集成 perfetto.

> [!NOTE]
> 从 perfetto v54 起, 上游不再把 amalgamated SDK 文件 (`perfetto.h` / `perfetto.cc`)
> 提交进 git 仓库, 而是作为 GitHub release 资产 `perfetto-cpp-sdk-src.zip` 单独发布.
> 本仓库的 `CMakeLists.txt` 会在 **配置期自动下载** 与 submodule 当前 tag 对应的
> SDK 并放回 `perfetto/sdk/`, 因此 **构建前需联网**.  对 v53 及更早 (SDK 文件仍在
> submodule 内) 保持向后兼容, 不会触发下载.
>
> - SDK 来源可用 cache 变量 `PERFETTO_SDK_URL` 覆盖.
> - 版本戳 `perfetto/sdk/.sdk-version` 用于判断本地 SDK 是否与当前 tag 匹配;
>   升级 submodule tag 后会自动重新下载.

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
