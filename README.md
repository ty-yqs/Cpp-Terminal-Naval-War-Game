# C++ Terminal Naval War Game

## 1. 项目简介 (Introduction)
这是一个基于 C++17 和 `ncurses` 库开发的终端海战游戏。本项目不仅是一个简单的游戏，更是一个面向对象编程 (OOP) 的教学案例。它展示了如何使用 C++ 构建一个具有完整生命周期、即时渲染和交互逻辑的系统。

**核心特性：**
*   **面向对象设计**: 充分利用继承 (`Inheritance`) 和多态 (`Polymorphism`) 管理游戏实体。
*   **内存管理**: 使用智能指针 (`std::unique_ptr`) 自动管理对象生命周期，杜绝内存泄漏。
*   **即时渲染**: 使用 `ncurses` 库实现终端下的高性能字符绘图。
*   **游戏架构**: 经典的 `Input` -> `Update` -> `Render` 游戏循环模式。

---

## 2. 项目结构 (Project Structure)
```text
.
├── Makefile            # 自动化编译脚本
├── README.md           # 项目文档
├── ClassDiagram.md     # 类图设计文档
├── main.cpp            # 程序入口
├── game.h/cpp          # 游戏核心控制器 (上帝类)
├── world.h/cpp         # 地图与环境数据
├── renderer.h/cpp      # 负责屏幕绘制 (ncurses 封装)
├── input_manager.h/cpp # 输入处理
├── entity.h/cpp        # 所有物体的基类
├── ship.h/cpp          # 飞船基类 (继承自 Entity)
├── player_ship.h/cpp   # 玩家飞船 (继承自 Ship)
├── enemy_ship.h/cpp    # 敌人飞船 (继承自 Ship)
├── projectile.h/cpp    # 子弹/导弹 (继承自 Entity)
└── pickup.h/cpp        # 掉落道具 (继承自 Entity)
```

---

## 3. 环境准备与安装 (Prerequisites)

### macOS
macOS 通常预装了 ncurses 库。你需要安装编译器：
```bash
xcode-select --install
```

### Linux (Ubuntu/Debian)
需要安装 ncurses 开发库：
```bash
sudo apt-get update
sudo apt-get install libncurses5-dev libncursesw5-dev g++ make
```

### Windows
Windows 原生不支持 ncurses。推荐使用 **WSL (Windows Subsystem for Linux)** 安装 Ubuntu 子系统，然后按照 Linux 步骤操作。

---

## 4. 编译与运行 (Build & Run)

本项目使用 `Makefile` 管理编译过程，自动处理依赖关系。

```bash
# 1. 编译项目
make

# 2. 运行游戏
./game

# 2.1 指定地图文件 (可选)
./game map.txt

# 3. 清理编译产生的临时文件 (.o 文件)
make clean
```

**Makefile 原理解析**:
*   `clang++ -std=c++17`: 指定使用 C++17 标准。
*   `-lncurses`: 链接 ncurses 库，这是绘图的关键。
*   `-o game`: 输出可执行文件名为 `game`。

---

## 5. 从零开始：构建教程 (Step-by-Step Guide)

如果你想亲手重写这个项目，请遵循以下逻辑路径：

### 第一阶段：基础架构
1.  **创建 `main.cpp`**: 它是程序的入口，只负责实例化 `Game` 对象并调用 `runLoop()`。
2.  **设计 `Game` 类**:
    *   这是游戏的“大脑”。
    *   实现 `runLoop()`：一个 `while(running)` 循环。
    *   在循环中依次调用 `handleInput()`, `update()`, `render()`。
    *   控制帧率：使用 `usleep` 或 `std::this_thread::sleep_for` 防止 CPU 占用过高。

### 第二阶段：显示系统 (Renderer)
1.  **引入 ncurses**: 在 `Renderer` 构造函数中调用 `initscr()`, `noecho()`, `curs_set(0)` 等初始化函数。
2.  **封装绘图**: 不要直接在游戏逻辑里写 `mvaddch`。
    *   写一个 `drawEntity(const Entity& e)` 函数。
    *   **难点**: 如何支持多行字符？(如轰炸机)。
    *   **解法**: 使用 `stringstream` 按 `\n` 分割字符串，逐行绘制。

### 第三阶段：实体系统 (Entity Hierarchy)
这是 OOP 的核心。
1.  **`Entity` (基类)**:
    *   属性: `row`, `col`, `glyph` (显示字符), `color`。
    *   方法: `virtual void update() = 0;` (纯虚函数，强制子类实现)。
2.  **`Ship` (战斗单位)**:
    *   继承自 `Entity`。
    *   新增: `hp` (血量), `spawnProjectile()` (发射子弹)。
    *   **设计模式**: 飞船发射子弹时，不能直接把子弹加到 `Game` 的列表里（因为 `Ship` 不知道 `Game` 的存在）。
    *   **解法**: `Ship` 将子弹存入自己的 `newProjectiles_` 队列，`Game` 在每帧结束时提取它们。

### 第四阶段：游戏逻辑实现
1.  **玩家控制 (`PlayerShip`)**:
    *   在 `handleInput` 中根据 `InputState` 修改 `row/col`。
    *   实现边界检查：不能飞出屏幕。
2.  **敌人 AI (`EnemyShip`)**:
    *   使用 `rand()` 决定移动方向。
    *   使用计时器 (`timer`) 控制移动频率（例如每 10 帧移动一次），避免移动太快。
3.  **碰撞检测 (`Game::checkCollisions`)**:
    *   双重循环遍历：`Projectiles` vs `Enemies`，`Player` vs `Enemies`。
    *   如果坐标重叠 (`r1==r2 && c1==c2`)，则判定命中，扣血并标记死亡。

---

## 6. 核心类详细说明 (Class Details)

### `Entity` (entity.h/cpp)
*   **定位**: 抽象基类。
*   **关键点**: 使用 `std::string` 存储 `glyph_` 而不是 `char`，这使得我们可以轻松实现像轰炸机那样复杂的图形。

### `Game` (game.h/cpp)
*   **定位**: 中央控制器。
*   **容器管理**:
    ```cpp
    std::unique_ptr<PlayerShip> player_;
    std::vector<std::unique_ptr<EnemyShip>> enemies_;
    std::vector<std::unique_ptr<Projectile>> projectiles_;
    ```
    使用 `vector` + `unique_ptr` 是现代 C++ 管理对象集合的最佳实践，当对象从 vector 移除时，内存自动释放。

### `InputManager` (input_manager.h/cpp)
*   **定位**: 输入抽象层。
*   **设计**: 将 ncurses 的原始键值 (如 `KEY_UP`, `104`) 转换为游戏语义 (`dRow = -1`, `fireShell = true`)。这样如果以后想改键位，只需改这一个文件。

### `Renderer` (renderer.h/cpp)
*   **定位**: 视图层。
*   **多行渲染实现**:
    ```cpp
    void drawEntity(...) {
        stringstream ss(entity.getGlyph());
        string line;
        int r = entity.getRow();
        while(getline(ss, line)) {
            mvaddstr(r++, entity.getCol(), line.c_str());
        }
    }
    ```
    这段代码让我们可以给实体设置 "^\n=\nV" 这样的字符串，它会自动被画成 3 行。

### `PlayerShip` (player_ship.h/cpp)
*   **动态外观**: 在 `handleInput` 中，根据移动方向 (`dRow`) 实时修改 `glyph_`，实现飞船转向的视觉效果。

### `EnemyShip` (enemy_ship.h/cpp)
*   **工厂模式雏形**: 构造函数根据 `EnemyType` 枚举初始化不同的属性（血量、颜色、符号），这是一种简单的工厂模式实现。

---

## 7. 类图 (Class Diagram)

![Class Diagram](https://mermaid.ink/img/pako:eNqNWFtv4jgU_itR9oUOUEFhphRVlcpFLRK9CNod7Q6jkUtc8JLEGcfpZbvlt--xnYvtBNo-0OT7fHyuPj7w5i6ph92-u_RRHI8IWjEULEIH_iTijENO-KvzpjDxd3qKHmLO0JKfnRXoHyTkDqPPvyxoSX0dAkESrpyV_xqtK5ZSpoMPlPqOh5GngXVlUC1V13BSyYajb32gCWxTCR1LIg9xXDv4omErzGf0uXYgdjThIfWr4AuhCQilWONizG9prExMDTwo7UhZac84I0oiJB5BGEBAhETDN8T3M8feF6GeuPmaRHraZIjXkR30AL1cGuATXnLKtklIfif4V8TZ9pbRfwAkPt5unRA_F--xnhih8IO0KHgd6a5xtMEjFKAVlsJesLJidRlVBf9K2F0iQKMPpl0bRsKq_U7p9Rmh57DgatUSTlQd81sfvWJmR76pYkFCPVwSjNfY90toQOLYCq7EQQUfETYzDpnODI2zVi-sKaWl6ixo0BqFno8nYZTwmvycc1gCwvDYcL5T5nvOs_jUhZDnDYWTUhkKaBLyUtUL2s4ZyJ0HAa0VAVFmZmEwDMPI373_XEpXVku6l85ZuRuHOHgtpU6id68Rdjh8lDJFn_AdCTCziUfCqgkhMQk5Zk_IrxLaxT3Q4AEzyPHnSgiKPcIexZ-prXruecXpNf3_oGwQuVegEI9k9c2y3dTrUOyZth2g4vwFiNjKZq4akpY_WwlfUob_RH6C9yS2OLelQ-lVnCXPPEXNQnxnGfjkUWZbx-XdJW7JDXS_UnoQU7dNNWGf46IdlROUuZE-yQBXmPxB5kBp2oMrDk-aA3NX89q6k3ZL83hujPBjR58ky01iHjQFVYW4rqgK3y2Zj33MPMnlKq1T_e1N9xDaH1lKxRtRtjsoUcR2CVCYldBSnLgfndbPH73WT01YajKMJOEAGpsXlzu2ffWTeODT5QZ7e5dars1w6GGGWekkgHypV8BWRp_IhDOFca7ROLrbfJ2OQgiQiXgMPQ8og6VxCVeB2XHRiAWXiVdLJwuRgSS2F6RDXzq-YvlPXxOBLD_nuwcWjl-4KYBj2GbHtCVvySsUwhEygluPqJjQnOIWrRS_QIHZnTTX9bTk-WPpg04aNsi7utSQWBKGVj_SJ5x8YNim_VpfWJ6h8osD5kIMz8S8bz6YJKPKMbJKSh5XISEfylOUmNns27YuQmrUFfg-pTTSsea-QUdfV24nTZUBA5KGjFUgyoRywySWa7wUbcMnMaGhxalZ1hhkd9eeMtz4mgaFnJhf0urZbWFDQ6O1yFIR04gcqCoJOA3Im-JHvpudkdW6kk6HMZv6nZA9k5ns8oZ7OEwg5dB9aaj7eHF_Pbg5vyuA0Xh-N7v5azwroOHsfjLXgcHN1SB73zk-fNqE-eV4Oi1e725mt-PRTQFcTebzyXS852r8tKrv4_Pbm2tt6_FoMjyfWlunXfD0v2ZTfi8so4WXFZw0SeFyQFZo3ixsJm8LilCfssN9AVZ2NgvL2poF6w3NomztkqBl5TluO1gQqXeZ39m3gMPDM3vs1CZJwVYNROlkI2lrytC-HAravhEsWgtT3vT34SphijAuAlOX23BXjHhuH3oDbrhQUgESr64stoXL1zjAC7cPjx5-RInPF-4ifAexCIV_Uxpkkowmq7Xbf0R-DG-qP6Y_H2VLQMLtv7kvbr_Z67YOW93j9tfW8cnXTu9bp-G-uv3u0eHxcat11AWk2263e-8N91-po3vYPu4dtU867V6nd9I7gfWq3w7F1z6332kDgj0CN8VV-vuV-Pf-P6rotEc?type=png)

---

## 8. 进阶挑战 (Advanced Challenges)

如果你已经完全理解了本项目，尝试完成以下挑战来提升水平：

1.  **配置文件加载**:
    *   不要把敌人的血量写死在代码里 (`enemy_ship.cpp`)。
    *   试着创建一个 `config.txt`，程序启动时读取敌人的属性。
2.  **动画系统**:
    *   现在的爆炸只是物体消失。
    *   试着创建一个 `Explosion` 类 (继承自 Entity)，它会显示 `*` -> `O` -> `@` 然后消失。
3.  **多态的深层应用**:
    *   为 `Projectile` 创建子类 `Missile` 和 `Torpedo`，重写 `update()` 方法来实现不同的飞行轨迹（如导弹追踪，鱼雷慢速）。

---

## 9. 常见问题 (FAQ)

*   **Q: 为什么我看不到颜色？**
    *   A: 确保你的终端支持颜色。在 `Renderer` 中我们调用了 `start_color()`。
*   **Q: 为什么飞船移动时会留下残影？**
    *   A: 每一帧开始时必须调用 `clear()` 清空屏幕，本项目在 `Game::render()` 中已经处理。
*   **Q: 为什么修改了 .h 文件后编译报错？**
    *   A: 修改头文件通常需要重新编译所有依赖它的 .cpp 文件。建议运行 `make clean && make`。
