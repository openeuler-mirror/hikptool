# 网络亚健康检测系统 (sub_health)

`subhealth` 是 `hikptool` 的一个子模块，用于检测UB CLOS 网络/ 电组网中节点间的通信时延异常，并通过聚类分析定位故障域。
系统采用**三阶段流水线**：

```
Step 1 探测规划        Step 2 探测执行          Step 3 亚健康检测
topology.json  ──▶  probe_plan.json  ──▶  probe_result.json  ──▶  detection.json
(拓扑)              (探测计划)             (探测结果/时延)          (检测结果/故障域)
```

当前支持以下五类故障域：

| 故障域                | 含义                                                                           |
| --------------------- | ------------------------------------------------------------------------------ |
| `NODE_UPLINK`       | 当前节点某个 UBPU 的所有有效 intra/inter 探测项均被判为异常                    |
| `L1_UPLINK`         | 排除聚类异常样本后，inter 时延中位数明显高于 intra 时延中位数                  |
| `L2_DOWNLINK`       | inter 聚类异常中存在连续异常区间，数量不少于3，并且连续异常目的端属于同一个 L1 |
| `INTRA_L1_DOWNLINK` | 当前 L1 域内出现聚类异常，或单个 intra 样本超过时间阈值                        |
| `INTER_L1_DOWNLINK` | 跨 L1 探测出现聚类异常，但不满足 `L2_DOWNLINK` 的判定条件                    |

故障域在输出中对应以下字符串：

- `SUB_HEALTH_NODE_UPLINK`
- `SUB_HEALTH_L1_UPLINK`
- `SUB_HEALTH_L2_DOWNLINK`
- `SUB_HEALTH_INTRA_L1_DOWNLINK`
- `SUB_HEALTH_INTER_L1_DOWNLINK`

---

## 1. 工作原理简述

```
                    ┌─────────────────────────────┐
  topology.json ───▶│  Step 1 探测规划 (probe_plan)│  L1 冗余检测、发包数计算、
   (拓扑文件)       └─────────────┬───────────────┘  生成 intra/inter 目的Primary EID信息
                                  │ probe_plan.json
                    ┌─────────────▼───────────────┐
                    │  Step 2 探测执行 (probe_exec)│  本机节点作为源节点探测、多线程并发
                    └─────────────┬───────────────┘  urma_ping、Top-N 裁剪均值
                                  │ probe_result.json + urma_ping_output.log
                    ┌─────────────▼───────────────┐
                    │  Step 3 亚健康检测 (detect)  │  阈值 + K-means 聚类
                    └─────────────┬───────────────┘  五类故障域判定
                                  │
                          detection.json + sub_health_detect.log
```

## 2. 当前版本限制

使用前请注意以下限制：

1. 当前仅支持指定UBPU Primary EID 的探测，无法指定UB Port EID。
2. Step 2 只执行属于本机节点的探测任务，不会在一台服务器上完成整个网络所有节点的探测。
3. 完全超时、命令执行失败或输出格式异常的探测任务不会写入时延数组。
4. 当前版本没有独立的 `LINK_DOWN` 故障域。
5. `detection.json` 为 `{}` 仅表示在有效时延样本中未检测到亚健康，不能单独证明网络完全健康。
6. 输出文件名固定，当前不支持通过命令行自定义输出文件名。
7. 输出文件写入当前工作目录，已有同名文件会被覆盖。

## 3. 环境要求

### 3.1 硬件环境

真实探测需要支持 URMA 通信的鲲鹏 ARM64 服务器。

其他架构可以用于源码检查、编译验证或算法测试，但不能保证能够完成真实的 `urma_ping` 探测。

### 3.2 操作系统

推荐使用 openEuler。

其他 Linux 发行版需要满足 hikptool、URMA 和相关运行依赖。

### 3.3 构建依赖

- C 编译器
- CMake 3.13 或更高版本
- Make 或其他 CMake 支持的构建工具
- pthread
- 系统数学库

cJSON 源码已经包含在本模块中，不需要额外安装系统 `libcjson` 软件包。

### 3.4 运行依赖

- `urma_ping`：执行真实 UB 时延探测
- `ip`：通过 `ip addr` 获取本机 IP
- `curl`：通过 RESTCONF 接口获取拓扑和 slot 信息
- RESTCONF Unix Socket：`/run/ubm/socket/ubm_nuds/restconf.sock`

使用 root 权限运行，以保证能够访问 RESTCONF Socket 和 URMA 设备。

## 4. 编译程序

### 4.1 获取源码

```bash
git clone https://atomgit.com/openeuler/hikptool.git
cd hikptool
```

### 4.2 配置项目

```bash
cmake -S . -B build
```

如果使用较新的 CMake 时出现以下兼容性错误：

```text
Compatibility with CMake < 3.5 has been removed
```

可以增加兼容参数：

```bash
cmake -S . -B build \
  -DCMAKE_POLICY_VERSION_MINIMUM=3.5
```

### 4.3 编译项目

```bash
cmake --build build -j"$(nproc)"
```

编译完成后，可执行文件位于：

```text
build/hikptool
```

## 5. 目录结构

```
hikptool/ub/subhealth/
├── README.md                  # 本文件（使用说明）
├── sub_health.h               # 头文件：数据结构、常量、模块接口
├── sub_health_cmd.c           # 命令入口、参数解析、流程编排
├── topology_generator.c       # 拓扑生成（RESTCONF XML → JSON，仅电组网）
├── probe_plan.c               # 探测规划（L1 冗余检测、发包数计算、目的端口匹配）
├── probe_execute.c            # 探测执行（多线程 urma_ping、Top-N 裁剪均值）
├── sub_health_detect.c        # 亚健康检测（阈值 + 聚类、五类故障域判定）
├── unified_clustering.h       # 聚类算法接口
├── unified_clustering.c       # 统一聚类算法（Z-score + 肘部法 + K-means++）
├── cJSON.h / cJSON.c          # 内嵌 JSON 解析库
```

## 6. 全流程执行

在 hikptool 编译产物目录下：

### 6.1 自动获取拓扑

电组网可以不指定拓扑文件，由程序通过 RESTCONF 接口自动生成拓扑：

```bash
./hikptool sub_health
```

自动获取模式仅支持电组网

### 6.2 手动指定拓扑

Clos 组网必须手动指定拓扑文件：

```bash
./hikptool sub_health -t topology.json
```

电组网也可以通过 `-t` 手动指定拓扑文件。

完成后当前目录会生成：

| 文件                      | 说明                                             |
| ------------------------- | ------------------------------------------------ |
| `probe_plan.json`       | 探测计划（Step 1 输出）                          |
| `probe_result.json`     | 探测结果（Step 2 输出，含各目的端口时延）        |
| `urma_ping_output.log`  | 每次 urma_ping 的完整原始输出日志（Step 2）      |
| `sub_health_detect.log` | 检测诊断日志（Step 3，每个 UBPU 的完整分析过程） |
| `detection.json`        | 结构化检测结果（Step 3 输出，故障域列表）        |

## 7. 分步执行

分步执行，可以用 `-1 / -2 / -3` 只执行某一步：

```bash
# 第 1 步：探测规划（输入拓扑，输出探测计划）
./hikptool sub_health -1 -t topology.json 
# 第 2 步：探测执行（输入探测计划，输出探测结果；本机节点执行 urma_ping）
./hikptool sub_health -2 -p probe_plan.json
# 第 3 步：亚健康检测（输入探测结果，输出检测结果）
./hikptool sub_health -3 -r probe_result.json
```

## 8. 参数说明

```
hikptool sub_health [选项]
```

| 选项   | 长选项               | 说明                                          |
| ------ | -------------------- | --------------------------------------------- |
| `-h` | `--help`           | 显示帮助                                      |
| `-t` | `--topology`       | 拓扑文件（Step 1 / 全流程输入）               |
| `-p` | `--probe-plan`     | 探测计划文件（Step 2 输入）                   |
| `-r` | `--result`         | 探测结果文件（Step 3 输入）                   |
| `-k` | `--coverage`       | 链路覆盖次数，默认 5，范围 [3, 20]            |
| `-s` | `--packet-size`    | 探测包大小（字节），默认 4096，范围 [4, 4096] |
| `-T` | `--time-threshold` | 时延阈值 ms，默认 100，超过即标记异常         |
| `-1` | `--step1`          | 只执行探测规划                                |
| `-2` | `--step2`          | 只执行探测执行                                |
| `-3` | `--step3`          | 只执行亚健康检测                              |

## 9. 拓扑文件

拓扑文件是 Step 1 的输入，描述网络结构。两种来源：

### 9.1 自动生成（仅电组网支持RESTCONF接口）

全流程执行且不指定 `-t` 时，系统自动调用 RESTCONF API（通过 `/run/ubm/socket/ubm_nuds/restconf.sock`）获取拓扑。**仅支持电组网**（单 L1，名称为 `1D-FULLMESSH`）。

### 9.2 手动编写（CLOS组网）

CLOS 光组网必须手动提供。格式如下（`topology.json`）：

```json
{
  "l2_switches": [
    "L2-SW01",
    "L2-SW02"
  ],
  "l1_switches": {
    "L1-SW01": {
      "192.168.1.1": {
        "0": "0000:0000:0000:0000:0000:0000:0000:0001",
        "1": "0000:0000:0000:0000:0000:0000:0000:0005"
    },
      "192.168.1.2": {
        "0": "0000:0000:0000:0000:0000:0000:0000:0002",
        "1": "0000:0000:0000:0000:0000:0000:0000:0006"
      },
      "192.168.1.3": {
        "0": "0000:0000:0000:0000:0000:0000:0000:0003",
        "1": "0000:0000:0000:0000:0000:0000:0000:0007"
      },
      "192.168.1.4": {
        "0": "0000:0000:0000:0000:0000:0000:0000:0004",
        "1": "0000:0000:0000:0000:0000:0000:0000:0008"
      }
    },
    "L1-SW02":{
      "192.168.2.1": {
        "0": "0000:0000:0000:0000:0000:0000:0000:0011",
        "1": "0000:0000:0000:0000:0000:0000:0000:0015"
      },
      "192.168.2.2": {
        "0": "0000:0000:0000:0000:0000:0000:0000:0012",
        "1": "0000:0000:0000:0000:0000:0000:0000:0016"
      },
      "192.168.2.3": {
        "0": "0000:0000:0000:0000:0000:0000:0000:0013",
        "1": "0000:0000:0000:0000:0000:0000:0000:0017"
      },
      "192.168.2.4": {
        "0": "0000:0000:0000:0000:0000:0000:0000:0014",
        "1": "0000:0000:0000:0000:0000:0000:0000:0018"
      }
    }
  }
}
```

结构说明：

| JSON 路径                                     | 必填 | 说明                                       |
| --------------------------------------------- | ---- | ------------------------------------------ |
| `l2_switches`                               | 是   | L2 交换机名称数组；电组网填写空数组 `[]` |
| `l1_switches`                               | 是   | L1 交换机和节点的映射对象                  |
| `l1_switches.<L1_NAME>`                     | 是   | 指定 L1 下的节点集合                       |
| `l1_switches.<L1_NAME>.<NODE_IP>`           | 是   | 节点 IPv4 地址，必须是合法 IPv4            |
| `l1_switches.<L1_NAME>.<NODE_IP>.<UBPU_ID>` | 是   | UBPU 到Primary EID 的映射                 |
| UBPU 对应的值                                 | 是   | 对应 UBPU 的 Primary EID                   |

> 电组网拓扑：L1 名固定为 `1D-FULLMESSH`，`l2_switches` 为空。
> 拓扑中每个 UBPU 对应的 Primary EID，会在探测计划和结果中透传为 src_eid。

## 10. 查看检测结果

### 10.1 `detection.json`

```json
{
  "192.168.1.1": {
    "0": {
      "src_eid": "0000:0000:0000:0000:0000:0000:0000:0001",
      "sub_health_dst_eids": ["0000:0000:0000:0000:0000:0000:0000:0002", "0000:0000:0000:0000:0000:0000:0000:0003","0000:0000:0000:0000:0000:0000:0000:0004"],
      "sub_health_latencies": [3.366, 2.534, 3.586],
      "sub_health_domain": ["SUB_HEALTH_INTRA_L1_DOWNLINK"]
    },
    "1": {
      "src_eid": "0000:0000:0000:0000:0000:0000:0000:0005",
      "sub_health_dst_eids": ["0000:0000:0000:0000:0000:0000:0000:0006", "0000:0000:0000:0000:0000:0000:0000:0007","0000:0000:0000:0000:0000:0000:0000:0008"],
      "sub_health_latencies": [5.16, 3.853, 4.38433],
      "sub_health_domain": ["SUB_HEALTH_INTRA_L1_DOWNLINK"]
    }
  }
}
```

字段说明：

| 字段                     | 说明                                            |
| ------------------------ | ----------------------------------------------- |
| `src_eid`              | 当前节点当前 UBPU 的源 Primary EID              |
| `sub_health_dst_eids`  | 被标记为异常的目的 Primary EID 数组             |
| `sub_health_latencies` | 异常目的 Primary EID 对应的时延数组，单位为毫秒 |
| `sub_health_domain`    | 当前 UBPU 命中的故障域数组                      |

`sub_health_dst_eids` 和 `sub_health_latencies` 按相同索引一一对应。

异常链路按照以下顺序写入：

1. intra 异常链路
2. inter 异常链路

只有检测到异常的节点和 UBPU 才会写入 `detection.json`。

### 10.2 空检测结果

下面的结果：

```json
{}
```

表示程序没有在当前有效时延样本中检测到符合规则的亚健康故障域。

它不等同于“网络一定健康”，因为以下任务可能没有进入时延数组：

- 完全超时
- `urma_ping` 命令不存在
- `urma_ping` 执行失败
- 输出数量不符合预期
- 成功样本数不足

遇到空结果时，应同时检查：

1. Step 2 的任务执行汇总。
2. `urma_ping_output.log`。
3. `probe_result.json` 中是否包含本机节点和有效时延。
4. 是否存在大量超时或执行失败。

### 10.3 sub_health_detect.log

`sub_health_detect.log` 记录每个节点和 UBPU 的详细检测过程，包括：

- 时间阈值判断
- intra/inter 聚类结果
- 中位数计算和比较
- 连续异常位置检查
- L1 映射检查
- 最终故障域判断
- 异常目的 EID 和时延

需要分析“为什么被判定为某个故障域”时，应优先查看该日志。

### 10.4 urma_ping_output.log

`urma_ping_output.log` 记录每次探测执行的命令和原始输出，可用于排查：

- `urma_ping` 是否存在
- 源 EID 和目的 EID 是否正确
- 是否出现请求超时
- 输出数量是否符合预期
- 命令是否执行失败

## 11. 参数与算法速查

| 参数               | 默认值 | 说明                                                              |
| ------------------ | ------ | ----------------------------------------------------------------- |
| `coverage_k`     | 5      | 链路覆盖次数；电组网 intra 发包数 = k，CLOS intra 发包数 = k × 2 |
| `packet_size`    | 4096   | urma_ping 的 `-s` 参数                                          |
| `time_threshold` | 100ms  | 时延超过此值标记为潜在异常                                        |

- Step 2 时延处理：每个探测对多次 ping，**取最近 k 个成功样本并排序，对称裁剪；然后计算TOP-N平均值**（Top-N 裁剪均值）；只保留命令执行成功、输出数量符合预期且成功样本数不少于 coverage_k 的探测对；允许部分请求超时
- Step 3 异常标记为 **OR 逻辑**：`时延 > 阈值` 或 `聚类判为异常`，任一满足即异常
- 聚类算法：Z-score 标准化 → 肘部法选 K → K-means++ → 均值比 > 2.0 判为异常 → 递归细化

## 12. 常见问题

**Q: 电组网运行报错 "Failed to generate topology"？**
RESTCONF 服务不可用。确认 `/run/ubm/socket/ubm_nuds/restconf.sock` 存在；或改用 `-t topology.json` 手动指定拓扑。

**Q: 没有 `urma_ping` 时探测结果会怎样？**
Step 2 执行 `urma_ping <dst> -I <src> -s <size> -c <count>`。命令不存在时，探测任务会被判定为执行失败并从结果数组中过滤，`probe_result.json` 的时延数组为空，Step 3 检不出异常。请先安装 `urma_ping` 并加入 PATH。

### Q：detection.json 是空对象

先检查 `probe_result.json` 是否存在有效时延，再检查 `urma_ping_output.log`。

重点关注：

- 命令执行失败
- 完全超时
- 输出数量异常
- 成功样本不足
- 本机节点没有探测任务

确认探测数据有效后，才考虑适当降低 `-T` 或增加 `-k`。

### Q：能否指定输出路径

当前版本不支持。

以下文件名固定：

```text
probe_plan.json
probe_result.json
detection.json
urma_ping_output.log
sub_health_detect.log
```

### Q: detection.json 里没有异常，但网络感觉有问题？**

把 `-T` 阈值调小（如 `-T 10`），或调大 `-k` 覆盖次数增加采样。
