# 网络亚健康检测系统 (sub_health)

`subhealth` 是 `hikptool` 的一个子模块，用于检测 CLOS / 电互连网络中节点间的通信时延异常，并通过聚类分析定位故障域。
系统采用**三阶段流水线**：

```
Step 1 探测规划        Step 2 探测执行          Step 3 亚健康检测
topology.json  ──▶  probe_plan.json  ──▶  probe_result.json  ──▶  detection.json
(拓扑)              (探测计划)             (探测结果/时延)          (检测结果/故障域)
```
**可检测的五类故障域：**

| 故障域 | 含义 |
|--------|------|
| NODE_UPLINK | 节点到 L1 的上行链路故障（节点所有 intra + inter 时延均超阈值） |
| L1_UPLINK | L1 到 L2 的上行链路故障（正常数据域间中位数 > 域内中位数 × 4.0） |
| L2_DOWNLINK | L2 到 L1 的下行链路故障（3 个连续异常 EID 同属同一 L1） |
| INTRA_L1_DOWNLINK | L1 域内下行链路故障（域内聚类异常） |
| INTER_L1_DOWNLINK | 跨 L1 链路故障（个别跨 L1 链路问题） |
---

## 1. 环境要求
- **硬件**：ARM64 服务器（HiSilicon Kunpeng 鲲鹏）
- **系统**：Linux（openEuler / CentOS / Ubuntu 均可）
- **依赖工具**：
  - `curl`：全流程且不指定 `-t` 时，自动从 RESTCONF API 获取拓扑（仅电组网）
  - `urma_ping`：Step 2 真实探测时使用
  - `pthread` / `libcjson`：编译链接需要（本模块自带 cJSON 源码，无需单独安装）
> 注意：本模块**依赖 ARM64 环境**（`urma_ping` 只在鲲鹏服务器上可用），在 x86 开发机上只能编译验证算法部分（见第 8 节「运行测试」）。
## 2. 快速上手（一条命令跑完）
在 hikptool 编译产物目录下：
```bash
# 方式 A：自动获取拓扑（电组网，需要 RESTCONF 服务）
./hikptool sub_health -o detection.json
# 方式 B：手动指定拓扑文件（CLOS 光组网必须用这种方式）
./hikptool sub_health -t topology.json -o detection.json
```
完成后当前目录会生成：
| 文件 | 说明 |
|------|------|
| `probe_plan.json` | 探测计划（Step 1 输出） |
| `probe_result.json` | 探测结果（Step 2 输出，含各目的端口时延） |
| `urma_ping_output.log` | 每次 urma_ping 的完整原始输出（Step 2） |
| `sub_health_detect.log` | 检测诊断日志（Step 3，每个 UBPU 的完整分析过程） |
| `detection.json` | 结构化检测结果（Step 3 输出，故障域列表） |

## 3. 分步执行（推荐第一次运行时使用）
想一步步看过程，可以用 `-1 / -2 / -3` 只执行某一步：
```bash
# 第 1 步：探测规划（输入拓扑，输出探测计划）
./hikptool sub_health -1 -t topology.json -p probe_plan.json
# 第 2 步：探测执行（输入探测计划，输出探测结果；本机节点执行 urma_ping）
./hikptool sub_health -2 -p probe_plan.json -r probe_result.json
# 第 3 步：亚健康检测（输入探测结果，输出检测结果）
./hikptool sub_health -3 -r probe_result.json -o detection.json
```
## 4. 参数说明
```
hikptool sub_health [选项]
```
| 选项 | 长选项 | 说明 |
|------|--------|------|
| `-h` | `--help` | 显示帮助 |
| `-t` | `--topology` | 拓扑文件（Step 1 / 全流程输入） |
| `-p` | `--probe-plan` | 探测计划文件（Step 1 输出 / Step 2 输入） |
| `-r` | `--result` | 探测结果文件（Step 2 输出 / Step 3 输入） |
| `-o` | `--output` | 检测结果输出文件（Step 3 输出） |
| `-k` | `--coverage` | 链路覆盖次数，默认 5，范围 [3, 20] |
| `-s` | `--packet-size` | 探测包大小（字节），默认 4096，范围 [4, 4096] |
| `-T` | `--time-threshold` | 时延阈值 ms，默认 100，超过即标记异常 |
| `-1` | `--step1` | 只执行探测规划 |
| `-2` | `--step2` | 只执行探测执行 |
| `-3` | `--step3` | 只执行亚健康检测 |

## 5. 怎么准备拓扑文件

拓扑文件是 Step 1 的输入，描述网络结构。两种来源：

### 5.1 自动生成（仅电组网）

全流程执行且不指定 `-t` 时，系统自动调用 RESTCONF API（通过 `/run/ubm/socket/ubm_nuds/restconf.sock`）获取拓扑。**仅支持电组网**（单 L1，名称为 `1D-FULLMESSH`）。

### 5.2 手动编写（推荐测试用）

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
    },
  }
}
```

结构说明：

| 字段 | 说明 |
|------|------|
| `l2_switches[]` | L2 交换机名列表（可选，可为空数组） |
| `l1_switches{L1-NAME}` | L1 交换机名 → 节点映射，一个 L1 下可以有多个节点 |
| `l1_switches{ip}` | 节点 IP，按 IP 标识节点（必须是合法 IPv4） |
| `l1_switches{ubpu_id}` | UBPU 端口号（字符串键 "0"/"1"）→ EID |

> 电组网拓扑：L1 名固定为 `1D-FULLMESSH`，`l2_switches` 为空。
> 拓扑文件里 `src_eid` 不是计算出来的，是各节点各 UBPU 的 EID 原样透传。

## 6. 怎么看结果

### 6.1 结构化结果 `detection.json`

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

- `sub_health_dst_eids[]` / `sub_health_latencies[]`：异常目的端口 EID 及对应时延（intra 异常在前，inter 异常在后，一一对应）
- `sub_health_domain[]`：该 UBPU 检测到的故障域列表，取值为：
  `SUB_HEALTH_NODE_UPLINK` / `SUB_HEALTH_L1_UPLINK` / `SUB_HEALTH_L2_DOWNLINK` / `SUB_HEALTH_INTRA_L1_DOWNLINK` / `SUB_HEALTH_INTER_L1_DOWNLINK`
- 空对象（无异常）说明网络健康

### 6.2 诊断日志 `sub_health_detect.log`
每个 UBPU 的完整分析过程：时间阈值判定 → 空间聚类结果 → 中位数比较 → 故障域判定 → 异常链路列表。排查"为什么判成这个故障域"时看这个文件。

### 6.3 原始数据 `urma_ping_output.log`
每次 urma_ping 命令的完整命令行和原始输出，用于排查 Step 2 探测问题。
## 7. 参数与算法速查
| 参数 | 默认值 | 说明 |
|------|--------|------|
| `coverage_k` | 5 | 链路覆盖次数；电组网 intra 发包数 = k，CLOS intra 发包数 = k × 2 |
| `packet_size` | 4096 | urma_ping 的 `-s` 参数 |
| `time_threshold` | 100ms | 时延超过此值标记为潜在异常 |
- Step 2 时延处理：每个探测对多次 ping，**去掉最大最小后取中间 3 个平均**（Top-N 裁剪均值）；只保留完全成功的探测对输出
- Step 3 异常标记为 **OR 逻辑**：`时延 > 阈值` 或 `聚类判为异常`，任一满足即异常
- 聚类算法：Z-score 标准化 → 肘部法选 K → K-means++ → 均值比 > 2.0 判为异常 → 递归细化
## 8. 常见问题

**Q: 运行报错 "Failed to generate topology"？**
RESTCONF 服务不可用。确认 `/run/ubm/socket/ubm_nuds/restconf.sock` 存在；或改用 `-t topology.json` 手动指定拓扑。

**Q: 没有 `urma_ping` 时探测结果会怎样？**
Step 2 执行 `urma_ping <dst> -I <src> -s <size> -c <count>`。命令不存在时所有探测会被判定为完全超时并被过滤，`probe_result.json` 的时延数组为空，Step 3 检不出异常。请先安装 `urma_ping` 并加入 PATH。

**Q: probe_result.json 为空 / 没有本节点数据？**
Step 2 只处理本机 IP 的节点（通过 `ip addr` 和 slot IP 判断）。在非目标节点上执行，或 plan 中不含本机 IP 时，本节点没有探测任务。

**Q: detection.json 里没有异常，但网络感觉有问题？**
把 `-T` 阈值调小（如 `-T 10`），或调大 `-k` 覆盖次数增加采样。

**Q: 全流程和单步骤的文件名规则？**
全流程：`probe_plan.json`、`probe_result.json`、`detection.json`；单步骤：直接用 `-p/-r/-o` 指定。

## 9. 目录结构
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

## 10. 工作原理简述

```
                    ┌─────────────────────────────┐
  topology.json ───▶│  Step 1 探测规划 (probe_plan)│  L1 冗余检测、发包数计算、
   (拓扑文件)       └─────────────┬───────────────┘  生成 intra/inter 目的端口
                                  │ probe_plan.json
                    ┌─────────────▼───────────────┐
                    │  Step 2 探测执行 (probe_exec)│  定位本机节点、多线程并发
                    └─────────────┬───────────────┘  urma_ping、Top-N 裁剪均值
                                  │ probe_result.json
                    ┌─────────────▼───────────────┐
                    │  Step 3 亚健康检测 (detect)  │  阈值 + K-means 聚类
                    └─────────────┬───────────────┘  五类故障域判定
                                  │
                          detection.json + sub_health_detect.log
```
