# Diablo Edit

> **Qt6 迁移版** — 原项目 [daidodo/diablo_edit](https://github.com/daidodo/diablo_edit)（MFC）的跨平台 Qt6 重构。

Diablo II 角色存档编辑器。加载和编辑 `.d2s` 文件。

已测试版本：1.09, 1.10, 1.13, Diablo II: Resurrected (PTR 2.4/2.5/2.6/3.1)

> ⚠️ 受限于资源条件，当前仅能进行 D2R 3.1 版本的测试。如需支持更多版本，欢迎协助提交对应版本的存档文件（`.d2s`）。如能提供该版本的原始 `.txt` 数据表文件，将更有效地推进开发工作。
>
> 解包工具参考：[D2RExtractor](https://github.com/levinium/D2RExtractor)
>
> 提交 `.txt` 文件的 PR 时，请将文件放在 `doc/source/txt/<版本号>/` 路径下，目录结构应与解包后的 `global/excel/` 保持一致（即 `doc/source/txt/<版本号>/global/excel/armor.txt` 等）。

## 截图

![1](https://user-images.githubusercontent.com/8170176/76164948-ebd06a00-614a-11ea-966a-efd01fde30d8.png)
![2](https://user-images.githubusercontent.com/8170176/76164957-fbe84980-614a-11ea-831e-121c01ef5034.png)
![3](https://user-images.githubusercontent.com/8170176/76164958-fe4aa380-614a-11ea-87a2-cad488f2f1c1.png)
![4](https://user-images.githubusercontent.com/8170176/76164961-01de2a80-614b-11ea-8416-659b750c66f3.png)
![5](https://user-images.githubusercontent.com/8170176/193444283-e95c319e-102f-4667-b26b-54ffe94cecb9.png)

## 下载

前往 [Releases](https://github.com/wheesys/diablo_edit/releases) 下载最新版本。

| 文件 | 说明 |
|---|---|
| `DiabloEdit-x86_64.AppImage` | Linux 便携版，下载后 `chmod +x` 即可运行 |
| `DiabloEdit-windows-x64.zip` | Windows 版，解压后运行 `DiabloEdit.exe` |

## 构建

### 依赖
- CMake 3.20+
- Qt6（Core, Gui, Widgets）
- C++17 编译器

### 编译
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --parallel
```

## 致谢

本仓库源自 [daidodo/diablo_edit](https://github.com/daidodo/diablo_edit)（MIT License），由 [@DoZerg](https://github.com/daidodo) 创建的原 MFC 版 Diablo II 角色编辑器。

本项目在其基础上进行了 Qt6 框架迁移和跨平台重构，感谢原作者的出色工作。

D2R 3.1 原始数据表（`.txt`）通过 [D2RExtractor](https://github.com/levinium/D2RExtractor) 解包获得，感谢 [@levinium](https://github.com/levinium) 提供的工具。

## License

[MIT](LICENSE) © 2020 DoZerg
