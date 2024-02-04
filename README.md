# adbClient
> Interface of adbClient.



## Prebuid

**MacOs**:

Due to apple clang support `std::format` since [Xcode15.3](https://developer.apple.com/xcode/cpp/), for which developer have to install llvm previously.

1. Install `llvm-toolchain`;

```shell
brew install llvm@17  
```

2. Append llvm-toolchain to `PATH`;

```shell
echo 'export PATH="/opt/homebrew/opt/llvm@17/bin:$PATH" ' >> ~/.bash_profile
echo 'export PATH="/opt/homebrew/opt/llvm@17/bin:$PATH" ' >> ~/.zprofile
```



**Windows**:

1. Install visual studio 2019 or later.
2. Append visual studio `cmake.exe`, `cl.exe` and other relative tools to `PATH`;

```shell
# Example:
add "D:\visual studio 2019\2019\VC\Tools\MSVC\14.29.30133\bin\Hostx64\x64" to PATH
add "D:\visual studio 2019\2019\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin" to PATH
```



**Linux**:

1. Install `cmake` and other tools.

```shell
sudo apt install cmake
```

2. Update G++ to g++-13.

```shell
sudo apt install g++-13
```



## Build

```shell
$ mkdir build && cd build
$ cmake ..
$ cmake --build .

```







## Reference

- [android adb client](https://cs.android.com/android/platform/superproject/main/+/main:packages/modules/adb/client/commandline.cpp;drc=d9b830ce29ee2d2534f48a698696966af52a540c;l=1501)
- [pure-python-adb](https://github.com/Swind/pure-python-adb)