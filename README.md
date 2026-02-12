# 项目名称

**三通道切换箱控制系统**

---

## 项目简介

这是一个三通道高压切换箱的主控制系统，该系统双路冗余触发，双路状态反馈，状态OLED显示，外部电源异常监控，一路报警输出。三路内部设备温度监控，温度联动散热系统。并预留128M存储器、3路按键开关、RS485通讯端口（暂不开发此三部分功能）。

---

## 硬件配置

> **注意！！！** 以下配置已经再CUBEMX中完成，后续代码开发不需要重新配置

### MCU配置

- **MCU型号**：STM32F103RCT6
- **外部晶振**：8MHz HSE
  - `OSC_IN` → OSC_IN
  - `OSC_OUT` → OSC_OUT

---

### 切换信号使能

| 信号 | 引脚 | 配置 |
|------|------|------|
| K1_EN | PB9 | 下降沿使能，开启中断，优先级4 |
| K2_EN | PB8 | 下降沿使能，开启中断，优先级4 |
| K3_EN | PA15 | 下降沿使能，开启中断，优先级4 |

---

### 驱动继电器使能

| 信号 | 引脚 | 配置 |
|------|------|------|
| K1_1_ON | PC0 | 推挽输出,低电平使能，初始高电平，内部上拉，高速 |
| K1_1_OFF | PC1 | 推挽输出,低电平使能，初始高电平，内部上拉，高速 |
| K2_1_ON | PC2 | 推挽输出,低电平使能，初始高电平，内部上拉，高速 |
| K2_1_OFF | PC3 | 推挽输出,低电平使能，初始高电平，内部上拉，高速 |
| K3_1_ON | PC7 | 推挽输出,低电平使能，初始高电平，内部上拉，高速 |
| K3_1_OFF | PC6 | 推挽输出,低电平使能，初始高电平，内部上拉，高速 |
| K1_2_ON | PA12 | 推挽输出,低电平使能，初始高电平，内部上拉，高速 *(2025-7-7日修改)* |
| K1_2_OFF | PA3 | 推挽输出,低电平使能，初始高电平，内部上拉，高速 |
| K2_2_ON | PA4 | 推挽输出,低电平使能，初始高电平，内部上拉，高速 |
| K2_2_OFF | PA5 | 推挽输出,低电平使能，初始高电平，内部上拉，高速 |
| K3_2_ON | PD2 | 推挽输出,低电平使能，初始高电平，内部上拉，高速 |
| K3_2_OFF | PA7 | 推挽输出,低电平使能，初始高电平，内部上拉，高速 |

---

### 驱动继电器状态反馈

| 信号 | 引脚 | 配置 |
|------|------|------|
| K1_1_STA | PC4 | 输入，高电平有效，初始低电平，内部下拉，高速 |
| K2_1_STA | PC5 | 输入，高电平有效，初始低电平，内部下拉，高速 |
| K3_1_STA | PB0 | 输入，高电平有效，初始低电平，内部下拉，高速 |
| K1_2_STA | PB1 | 输入，高电平有效，初始低电平，内部下拉，高速 |
| K2_2_STA | PB10 | 输入，高电平有效，初始低电平，内部下拉，高速 |
| K3_2_STA | PB11 | 输入，高电平有效，初始低电平，内部下拉，高速 |

---

### 接触器状态反馈

| 信号 | 引脚 | 配置 |
|------|------|------|
| SW1_STA | PA8 | 输入，高电平有效，初始低电平，内部下拉，高速 |
| SW2_STA | PC9 | 输入，高电平有效，初始低电平，内部下拉，高速 |
| SW3_STA | PC8 | 输入，高电平有效，初始低电平，内部下拉，高速 |

---

### 外部电源监控

| 信号 | 引脚 | 配置 |
|------|------|------|
| DC_CTRL | PB5 | 上升沿使能，内部上拉，开启中断，优先级2 |

---

### 异常报警输出

| 信号 | 引脚 | 配置 |
|------|------|------|
| ALARM | PB4 | 推挽输出，低电平使能，初始高电平，内部上拉，高速 |

---

### 风扇控制

| 信号 | 引脚 | 配置 |
|------|------|------|
| FAN_PWM | PA6 | 使用TIM3的channel通道输出初始值为50%占空比的脉冲信号<br>Prescaler (PSC): 35 // 预分频器 (72MHz/(35+1) = 2MHz)<br>Counter Period (ARR): 99 // 自动重装载值 (2MHz/(99+1) = 20KHz)<br>Counter Mode: Up // 向上计数模式<br>Auto-reload preload: Enable // 使能自动重装载预装载<br>Pulse: 50 // 占空比50% (可根据需要调整) |
| FAN_SEN | PC12 | 内置上拉，采用单位时间内检测到低电平脉冲数量方式计算出转速<br>计算公式：风扇转速(RPM) = (脉冲频率 × 60秒) ÷ 每转脉冲数<br>每一秒更新一次转速 |

---

### 报警蜂鸣器驱动

| 信号 | 引脚 | 配置 |
|------|------|------|
| BEEP | PB3 | 推挽输出，低电平有效，初始高电平，内部上拉，高速 |

---

### NTC热敏电阻

| 信号 | 引脚 | 配置 |
|------|------|------|
| NTC_1 | PA0 | ADC1_IN0,开启DMA通道并启用中断，优先级5，采样时间55.5，右对齐，连续扫描 |
| NTC_2 | PA1 | ADC1_IN1,开启DMA通道并启用中断，优先级5，采样时间55.5，右对齐，连续扫描 |
| NTC_3 | PA2 | ADC1_IN2,开启DMA通道并启用中断，优先级5，采样时间55.5，右对齐，连续扫描 |

---

### OLED显示

**通信方式**：I2C通讯

**硬件参数**：
- **显示屏尺寸**：2.42英寸
- **分辨率**：128×64
- **控制芯片**：SSD1309
- **显示颜色**：白色
- **显示区域**：55.01×27.49（mm）
- **工作电压**：3.3V

**参考规格书**：见DOC文件夹中
- `2.42插接-ZJY242-2864ASWPG01.pdf`
- `SSD1309.pdf`
- `ZJY242I0400WG01.pdf`

**引脚配置**：

| 信号 | 引脚 | 说明 |
|------|------|------|
| I2C1_SDA | PB7 | 连接到OLED显示屏SDA引脚 |
| I2C1_SCL | PB6 | 连接到OLED显示屏SCL引脚 |

---

### 调试串口

| 信号 | 引脚 |
|------|------|
| USART3_TX | PC10 |
| USART3_RX | PC11 |

---

### RS485端口（暂不开发此功能）

| 信号 | 引脚 | 配置 |
|------|------|------|
| USART1_TX | PA9 | - |
| USART1_RX | PA10 | - |
| RS485DE | PA11 | 高电平发送，低电平接收，初始设置低电平，内置下拉电阻 |

---

### 外部存储器

**通信方式**：SPI2接口

**配置参数**：
- **预分频系数**：16
- **模式**：FULL-DUPLEX MASTER
- **NSS**：SOFTWARE CONTROL（软件控制CS信号）

**参考规格书**：见DOC文件夹中
- `W25Q128JVSIQ技术文档.pdf`

**引脚配置**：

| 信号 | 引脚 | 说明 |
|------|------|------|
| FLASH_CS | PB12 | 连接到NOR FLASH CS引脚，GPIO手动控制 |
| SPI2_SCK | PB13 | 连接到NOR FLASH CLK引脚 |
| SPI2_MISO | PB14 | 连接到NOR FLASH DO引脚 |
| SPI2_MOSI | PB15 | 连接到NOR FLASH DI引脚 |

**重要修复说明（2025-01-28）**：
- 修复了CubeMX配置与代码实现的严重冲突
- PB12从硬件NSS输出改为普通GPIO输出
- 确保W25Q128所需的精确CS时序控制

---

### 按键接口（暂不开发此功能）

| 信号 | 引脚 | 配置 | 功能 |
|------|------|------|------|
| KEY1 | PC13 | 中断，上升、下降沿使能，开启中断，优先级4 | 长按3秒输出所有系统运行日志 |
| KEY2 | PC14 | 中断，上升、下降沿使能，开启中断，优先级4 | 长按3秒清空所有系统运行日志并输出"已清空"反馈 |
| KEY3 | PC15 | 中断，上升、下降沿使能，开启中断，优先级4 | （已规划，待实现） |

---


## 功能特性

本系统是一个高安全级别的三通道高压切换箱控制系统，具备完整的安全监控、状态反馈、温度保护和人机交互功能。系统采用双路冗余设计，确保在高压环境下的可靠运行。

### 核心特性概览

- **安全监控**：15种异常类型实时检测，多级报警机制，智能异常解除
- **继电器控制**：三通道互锁保护，双路冗余控制，精确状态反馈
- **温度保护**：三路NTC监控，联动风扇控制，过温保护机制
- **人机交互**：OLED分区显示，串口调试输出，实时状态监控
- **系统启动**：完整自检流程，LOGO显示，进度条指示

---

### 主体工作流程

#### 系统启动流程

1. **LOGO显示**（2秒）
   - 系统上电后，OLED显示公司LOGO
   - 维持2秒

2. **系统自检**（3秒）
   - OLED显示进度条
   - 执行智能状态识别与主动纠错检测

#### 自检内容（4个步骤）

**第一步：识别当前期望状态**

根据K1_EN、K2_EN、K3_EN的组合，确定真值表中应该处于的状态：

| K1_EN | K2_EN | K3_EN | 状态 |
|-------|-------|-------|------|
| 1 | 1 | 1 | 全部关断状态 |
| 0 | 1 | 1 | Channel_1打开状态 |
| 1 | 0 | 1 | Channel_2打开状态 |
| 1 | 1 | 0 | Channel_3打开状态 |

**第二步：继电器状态检查与主动纠错**

- 检查继电器状态：对比K1_1_STA、K1_2_STA、K2_1_STA、K2_2_STA、K3_1_STA、K3_2_STA与真值表期望值
- 发现错误时的处理：
  1. 立即输出错误报警信息（如："Channel_1继电器状态错误"）
  2. 同时控制继电器进行纠正（如：开启K1_1和K1_2继电器）
  3. 纠正后重新检查
- 最终判定：
  - 纠正成功 → 自检通过
  - 纠正失败 → 持续产生错误报警，自检失败

**第三步：接触器状态检查与报错**

- 检查接触器状态：对比SW1_STA、SW2_STA、SW3_STA与真值表期望值
- 发现错误时的处理：
  1. 输出错误报警信息（如："Channel_1接触器状态异常"）
  2. 不进行纠正（接触器由外部控制，系统无法主动纠正）
  3. 直接判定为自检失败

**第四步：温度安全检测**

- 温度安全检测：NTC_1、NTC_2、NTC_3三路温度传感器测量值均在60℃以下（确保热安全）

**自检时序流程**：
```
2秒LOGO显示 → 期望状态识别 → 继电器纠错 → 接触器检查 → 温度检测 → 结果判定
整个流程使用进度条表示在OLED上，按步骤划分百分比，同时输出异常标志
```

---

### 1、安全监控系统

系统具备完善的异常检测与报警功能：

- **异常类型**：支持15种异常类型（A~O），涵盖使能冲突、继电器异常、接触器异常、温度异常、自检异常、电源异常
- **智能位图管理**：使用16位标志位图高效管理所有异常状态
- **多级报警机制**：
  - ALARM引脚：任何异常时立即输出低电平
  - 蜂鸣器分级报警：
    - K~M类：持续低电平
    - B~J类：50ms间隔脉冲
    - A、N、O类：1秒间隔脉冲
  - 异常优先级：温度异常(K~M) > 状态反馈异常(B~J) > 冲突/自检/电源异常(A,N,O)
- **智能异常解除**：基于真值表逻辑的条件检测，确保系统恢复安全状态
- **电源监控保护**：DC_CTRL信号实时监控DC24V外部电源状态，异常时立即触发O类异常
- **性能指标**：
  - 响应时间：<500ms
  - CPU占用：<2%
  - 内存占用：<1KB

---



### 2、继电器驱动逻辑

#### 使能开启第一通道

**触发条件**：
- 当K1_EN检测到电平下降沿后开始间隔50ms连续检测三次均为低电平，则触发中断

**中断动作流程**：

1. **开始判定**：
   - 检测K2_EN、K3_EN是否为高电平
     - 如果是 → 进行下一步判定
     - 如果否 → 产生"A"异常标志
   - 检测K2_1_STA、K2_2_STA、K3_1_STA、K3_2_STA、SW2_STA、SW3_STA是否为低电平
     - 如果是 → 进行下一个判定
     - 如果否 → 哪个信号为高电平，则产生相对应的异常标志

2. **驱动继电器**：
   - 以上两次判定均为是，则K1_1_ON、K1_2_ON同时输出一个时长为500ms的低电平脉冲
   - 用来驱动外部的磁保持继电器吸合

3. **延时500ms后开始判定**：
   - 检测K1_1_STA、K1_2_STA、SW1_STA是否为高电平
     - 如果是 → OLED显示目前切换箱状态为打开（详情参阅OLED显示部分说明）
     - 如果否 → 输出相对应异常标志

---

#### 使能开启第二通道

**触发条件**：
- 当K2_EN检测到电平下降沿后开始间隔50ms连续检测三次均为低电平，则触发中断

**中断动作流程**：

1. **开始判定**：
   - 检测K1_EN、K3_EN是否为高电平
     - 如果是 → 进行下一步判定
     - 如果否 → 产生"A"异常标志
   - 检测K1_1_STA、K1_2_STA、K3_1_STA、K3_2_STA、SW1_STA、SW3_STA是否为低电平
     - 如果是 → 进行下一个判定
     - 如果否 → 哪个信号为高电平，则产生相对应的异常标志

2. **驱动继电器**：
   - 以上两次判定均为是，则K2_1_ON、K2_2_ON同时输出一个时长为500ms的低电平脉冲
   - 用来驱动外部的磁保持继电器吸合

3. **延时500ms后开始判定**：
   - 检测K2_1_STA、K2_2_STA、SW2_STA是否为高电平
     - 如果是 → OLED显示目前切换箱状态为打开（详情参阅OLED显示部分说明）
     - 如果否 → 输出相对应异常标志

---

#### 使能开启第三通道

**触发条件**：
- 当K3_EN检测到电平下降沿后开始间隔50ms连续检测三次均为低电平，则触发中断

**中断动作流程**：

1. **开始判定**：
   - 检测K1_EN、K2_EN是否为高电平
     - 如果是 → 进行下一步判定
     - 如果否 → 产生"A"异常标志
   - 检测K1_1_STA、K1_2_STA、K2_1_STA、K2_2_STA、SW1_STA、SW2_STA是否为低电平
     - 如果是 → 进行下一个判定
     - 如果否 → 哪个信号为高电平，则产生相对应的异常标志

2. **驱动继电器**：
   - 以上两次判定均为是，则K3_1_ON、K3_2_ON同时输出一个时长为500ms的低电平脉冲
   - 用来驱动外部的磁保持继电器吸合

3. **延时500ms后开始判定**：
   - 检测K3_1_STA、K3_2_STA、SW3_STA是否为高电平
     - 如果是 → OLED显示目前切换箱状态为打开（详情参阅OLED显示部分说明）
     - 如果否 → 输出相对应异常标志

---

#### 使能关闭第一通道

**触发条件**：
- 当K1_EN检测到电平上升沿后开始间隔50ms连续检测三次均为高电平，则触发中断

**中断动作流程**：

1. **驱动继电器**：
   - K1_1_OFF、K1_2_OFF同时输出一个时长为500ms的低电平脉冲
   - 用来驱动外部的磁保持继电器断开

2. **延时500ms后开始判定**：
   - 检测K1_1_STA、K1_2_STA、SW1_STA是否为低电平
     - 如果是 → OLED显示目前切换箱状态为关闭（详情参阅OLED显示部分说明）
     - 如果否 → 输出相对应异常标志

---

#### 使能关闭第二通道

**触发条件**：
- 当K2_EN检测到电平上升沿后开始间隔50ms连续检测三次均为高电平，则触发中断

**中断动作流程**：

1. **驱动继电器**：
   - K2_1_OFF、K2_2_OFF同时输出一个时长为500ms的低电平脉冲
   - 用来驱动外部的磁保持继电器断开

2. **延时500ms后开始判定**：
   - 检测K2_1_STA、K2_2_STA、SW2_STA是否为低电平
     - 如果是 → OLED显示目前切换箱状态为关闭（详情参阅OLED显示部分说明）
     - 如果否 → 输出相对应异常标志

---

#### 使能关闭第三通道

**触发条件**：
- 当K3_EN检测到电平上升沿后开始间隔50ms连续检测三次均为高电平，则触发中断

**中断动作流程**：

1. **驱动继电器**：
   - K3_1_OFF、K3_2_OFF同时输出一个时长为500ms的低电平脉冲
   - 用来驱动外部的磁保持继电器断开

2. **延时500ms后开始判定**：
   - 检测K3_1_STA、K3_2_STA、SW3_STA是否为低电平
     - 如果是 → OLED显示目前切换箱状态为关闭（详情参阅OLED显示部分说明）
     - 如果否 → 输出相对应异常标志

---
           
### 3、温度检测逻辑

#### 硬件电路

- **供电电压**：3.3V
- **电路结构**：使用一个10KΩ电阻与一个10K B值3435的NTC热敏电阻串联分压
- **ADC采集**：分压点电压作为ADC采集信号
- **参考数据**：NTC热敏电阻的RT值请参考`DOC/NTC热敏电阻RT值.csv`文件

#### 采集与计算

- **采集方式**：轮流检测ADC1的IN0、IN1、IN2三个通道的电压值
- **温度转换**：将测量到的电压值通过查表法对比RT值后，对应到当前对应的温度值

#### 风扇控制逻辑

| 温度范围 | 状态 | 风扇PWM占空比 | 异常标志 |
|---------|------|--------------|---------|
| < 35℃ | 正常温度 | 50% | 无 |
| ≥ 35℃ | 高温状态 | 95% | 无 |
| ≥ 60℃ | 过温状态 | 95% | 产生K、L、M异常标志 |

**温度回差机制**：
- 温度下降到上一级别的阈值以下时，风扇控制和异常标志恢复到上一级别的状态
- 设置2℃的温度回差（避免频繁切换）

#### 异常标志定义

| 标志 | 说明 |
|------|------|
| K | NTC_1温度异常 |
| L | NTC_2温度异常 |
| M | NTC_3温度异常 |

---

### 4、报警逻辑

#### 报警触发

当检测到任何异常标志位时：
- 将该异常输出到OLED屏幕上
- 同时使能"异常报警输出"和"报警蜂鸣器驱动"

#### 异常报警输出

- **ALARM引脚**（PB4）：无论何种异常标志，均连续输出低电平

#### 报警蜂鸣器驱动

| 异常类型 | 蜂鸣器输出模式 |
|---------|--------------|
| A、N、O类异常 | 时间间隔1秒的脉冲 |
| B~J类异常 | 时间间隔50ms的脉冲 |
| K~M类异常 | 持续低电平 |

---

#### 报警解除

**触发条件真值表**（供B~J、N异常解除条件参考）：

| 状态 | K1_EN | K2_EN | K3_EN | K1_1_STA | K1_2_STA | K2_1_STA | K2_2_STA | K3_1_STA | K3_2_STA | SW1_STA | SW2_STA | SW3_STA |
|------|-------|-------|-------|----------|----------|----------|----------|----------|----------|---------|---------|---------|
| Channel_1打开 | 0 | 1 | 1 | 1 | 1 | 0 | 0 | 0 | 0 | 1 | 0 | 0 |
| Channel_2打开 | 1 | 0 | 1 | 0 | 0 | 1 | 1 | 0 | 0 | 0 | 1 | 0 |
| Channel_3打开 | 1 | 1 | 0 | 0 | 0 | 0 | 0 | 1 | 1 | 0 | 0 | 1 |

**异常解除条件**：

- **A异常解除**：
  - 发生A异常时，在报警同时，持续检测K1_EN、K2_EN、K3_EN三路的状态
  - 确保只能有一路处于低电平或三路均处于高电平时，报警解除

- **B~J、N异常解除**：
  - 当发生此类异常时，在报警同时，需要持续检测K1_EN、K2_EN、K3_EN、K1_1_STA、K1_2_STA、K2_1_STA、K2_2_STA、K3_1_STA、K3_2_STA、SW1_STA、SW2_STA、SW3_STA各点状态
  - 确保符合三路触发后条件后（参考触发条件真值表），警报解除

- **K~M异常解除**：
  - 当温度回降到58℃以下（考虑到2℃回差）后报警解除

---

#### 异常标志详细定义

| 标志 | 说明 |
|------|------|
| A | K1_EN、K2_EN、K3_EN使能冲突（多路同时低电平） |
| B | K1_1_STA工作异常（继电器1第一路状态反馈异常） |
| C | K2_1_STA工作异常（继电器2第一路状态反馈异常） |
| D | K3_1_STA工作异常（继电器3第一路状态反馈异常） |
| E | K1_2_STA工作异常（继电器1第二路状态反馈异常） |
| F | K2_2_STA工作异常（继电器2第二路状态反馈异常） |
| G | K3_2_STA工作异常（继电器3第二路状态反馈异常） |
| H | SW1_STA工作异常（接触器1状态反馈异常） |
| I | SW2_STA工作异常（接触器2状态反馈异常） |
| J | SW3_STA工作异常（接触器3状态反馈异常） |
| K | NTC_1温度异常（传感器1温度≥60℃） |
| L | NTC_2温度异常（传感器2温度≥60℃） |
| M | NTC_3温度异常（传感器3温度≥60℃） |
| N | 自检异常（上电自检过程中发现的异常） |
| O | 系统的电源监控保护功能 |

---

### 5、OLED显示

#### 显示流程

1. **上电LOGO显示**
   - 显示公司LOGO
   - 维持2秒

2. **系统自检界面**
   - 显示自检信息
   - 显示进度条

3. **待机界面**
   - 自检结束后进入待机界面

#### 待机界面布局

待机界面分为三个部分（以横线分割出每个区域）：

1. **异常信息显示区域**（顶部）
   - 显示当前激活的异常标志

2. **三个通道的状态区域**（中部）
   - Channel_1状态：开/关
   - Channel_2状态：开/关
   - Channel_3状态：开/关

3. **温度显示以及风扇转速显示区域**（底部）
   - NTC_1温度值
   - NTC_2温度值
   - NTC_3温度值
   - 风扇转速(RPM)

---





## 开发需求

### 1. 系统启动流程规则

- ✅ 上电显示公司LOGO必须维持2秒
- ✅ 系统自检必须持续3秒并显示进度条
- ✅ 自检必须按以下顺序执行完整检测：
  - **前置安全动作**：通道关断确认（确保所有通道处于安全状态）
  - **输入信号检测**：K1_EN、K2_EN、K3_EN三路使能信号均为高电平（防止误触发）
  - **状态反馈检测**：9路状态信号均为低电平（6路继电器+3路接触器，确保设备关断）
  - **温度安全检测**：3路NTC温度传感器均<60℃（确保热安全）
- ✅ 任一检测失败必须产生"N"异常标志并立即启动安全监控报警机制
- ✅ 自检通过后必须无缝切换到正常运行状态，启动主循环调度
- ✅ 自检过程必须有详细的串口调试输出和OLED进度显示

---

### 2. 继电器控制规则

- ✅ 所有信号检测必须采用50ms间隔连续3次检测机制
- ✅ 继电器驱动必须使用500ms低电平脉冲
- ✅ 必须严格执行通道互锁机制
- ✅ 状态检测必须在脉冲后500ms进行

---

### 3. 温度控制规则

- ✅ 温度<35℃时，风扇必须维持50%PWM
- ✅ 温度≥35℃时，风扇必须提升至95%PWM
- ✅ 温度≥60℃时，必须触发对应的K、L、M异常标志
- ✅ 必须实现2℃温度回差机制

---

### 4. 报警处理规则

- ✅ 异常报警输出（ALARM）必须为持续低电平
- ✅ 蜂鸣器报警规则：
  - **A、N、O类异常**：必须使用1秒间隔脉冲
  - **B~J类异常**：必须使用50ms间隔脉冲
  - **K~M类异常**：必须使用持续低电平

---

### 5. 报警解除规则

- ✅ **A异常**：必须确认只有一路低电平或全高电平
- ✅ **B~J、N异常**：必须符合触发条件真值表
- ✅ **K~M异常**：必须等待温度降至58℃以下

---

### 6. OLED显示规则

- ✅ 显示界面必须严格分为三个区域：
  - 异常信息显示区
  - 三通道状态区
  - 温度和风扇转速区

---

### 7. 安全监控系统规则

- ✅ 必须支持15种异常类型的实时检测和管理
- ✅ 异常标志管理必须使用位图方式，确保高效和准确
- ✅ ALARM引脚必须在任何异常时立即输出低电平
- ✅ 蜂鸣器报警必须按异常类型分级：
  - **K~M类异常**：持续低电平（温度异常优先级最高）
  - **B~J类异常**：50ms间隔脉冲（状态反馈异常）
  - **A、N、O类异常**：1秒间隔脉冲（冲突和自检异常）
- ✅ 异常解除必须严格按照真值表逻辑条件检查
- ✅ 系统响应时间必须<500ms，确保安全保护及时性
- ✅ 必须确保通道互锁有效
- ✅ 必须实现双重继电器控制
- ✅ 必须实现完整的状态反馈检查
- ✅ 必须实现温度保护机制

---

## 项目目录结构

```
Three-channel controller_v4.0/
│
├── 📁 Core/                          # 核心应用代码目录
│   ├── 📁 Inc/                       # 头文件目录
│   │   ├── 🔧 CubeMX自动生成文件组
│   │   │   ├── main.h                # 主程序头文件
│   │   │   ├── stm32f1xx_hal_conf.h  # HAL库配置文件
│   │   │   ├── stm32f1xx_it.h        # 中断服务函数声明
│   │   │   ├── gpio.h                # GPIO配置
│   │   │   ├── adc.h                 # ADC配置
│   │   │   ├── dma.h                 # DMA配置
│   │   │   ├── i2c.h                 # I2C配置
│   │   │   ├── spi.h                 # SPI配置
│   │   │   ├── tim.h                 # 定时器配置
│   │   │   └── usart.h               # 串口配置
│   │   │
│   │   └── 👤 用户功能模块文件组
│   │       ├── common_def.h          # 通用定义（数据类型、宏定义、枚举等）
│   │       ├── relay_control.h       # 继电器控制模块
│   │       ├── safety_monitor.h      # 安全监控系统模块
│   │       ├── temperature.h         # 温度检测控制模块
│   │       ├── ntc_table.h           # NTC温度查表数据
│   │       ├── alarm_output.h        # 报警输出控制模块
│   │       ├── oled_display.h        # OLED显示驱动模块
│   │       └── self_test.h           # 系统自检模块
│   │
│   └── 📁 Src/                       # 源文件目录
│       ├── 🔧 CubeMX自动生成文件组
│       │   ├── main.c                # 主程序文件
│       │   ├── stm32f1xx_it.c        # 中断服务函数实现
│       │   ├── stm32f1xx_hal_msp.c   # HAL MSP初始化
│       │   ├── system_stm32f1xx.c    # 系统时钟配置
│       │   ├── gpio.c                # GPIO初始化
│       │   ├── adc.c                 # ADC初始化
│       │   ├── dma.c                 # DMA初始化
│       │   ├── i2c.c                 # I2C初始化
│       │   ├── spi.c                 # SPI初始化
│       │   ├── tim.c                 # 定时器初始化
│       │   └── usart.c               # 串口初始化
│       │
│       └── 👤 用户功能模块文件组
│           ├── common_def.c          # 通用定义实现
│           ├── relay_control.c       # 继电器控制逻辑（三通道互锁、双路冗余）
│           ├── safety_monitor.c      # 安全监控逻辑（15种异常检测、位图管理）
│           ├── temperature.c         # 温度检测与风扇控制（ADC采集、查表转换）
│           ├── ntc_table.c           # NTC查表数据实现
│           ├── alarm_output.c        # 报警输出逻辑（ALARM/BEEP分级控制）
│           ├── oled_display.c        # OLED显示实现（I2C驱动、界面显示）
│           └── self_test.c           # 系统自检流程（状态识别、纠错检测）
│
├── 📁 Drivers/                       # STM32驱动库目录
│   ├── 📁 STM32F1xx_HAL_Driver/      # STM32F1 HAL库驱动
│   │   ├── 📁 Inc/                   # HAL库头文件
│   │   └── 📁 Src/                   # HAL库源文件
│   │
│   └── 📁 CMSIS/                     # ARM CMSIS标准接口
│       ├── 📁 Include/               # CMSIS核心头文件
│       └── 📁 Device/ST/STM32F1xx/   # STM32F1设备支持包
│           ├── 📁 Include/           # 设备头文件
│           └── 📁 Source/Templates/  # 启动文件模板
│
├── 📁 MDK-ARM/                       # Keil MDK工程文件目录
│   ├── Three-channel controller_v4.0.uvprojx   # Keil工程文件
│   ├── Three-channel controller_v4.0.uvoptx    # Keil工程配置
│   ├── startup_stm32f103xe.s                   # 启动汇编文件
│   ├── 📁 DebugConfig/               # 调试配置文件
│   └── 📁 RTE/                       # 运行时环境配置
│
├── 📁 DOC/                           # 项目文档资料目录
│   ├── 📄 硬件规格书组
│   │   ├── 2.42寸屏-ZJY242-2864ASWPG01.pdf     # OLED显示屏规格书
│   │   ├── SSD1309.pdf                          # OLED控制芯片规格书
│   │   ├── ZJY242I0400WG01.pdf                  # OLED显示屏详细规格
│   │   ├── W25Q128JVSIQ技术文档.pdf             # FLASH存储器规格书
│   │   ├── W25Q128JVSIQ_p1~p7.pdf               # FLASH分册1
│   │   ├── W25Q128JVSIQ_p8.pdf                  # FLASH分册2
│   │   ├── W25Q128JVSIQ_p9.pdf                  # FLASH分册3
│   │   ├── DPC-0000011_EL817-G Series Datasheet rev11.pdf  # 光耦规格书
│   │   ├── NTC热敏电阻10K B值3435规格书.pdf     # NTC传感器规格书
│   │   └── CF1FBF721ECE2AB22ACED5F4B72E76C8.pdf # 其他器件规格
│   │
│   ├── 📊 设计参考数据组
│   │   ├── NTC热敏电阻RT值.csv                  # NTC阻值-温度对照表（CSV格式）
│   │   ├── NTC热敏电阻RT值.xlsx                 # NTC阻值-温度对照表（Excel格式）
│   │   ├── 热敏电阻阻值表.csv                   # 备用热敏电阻数据（CSV）
│   │   └── 热敏电阻阻值表.xlsx                  # 备用热敏电阻数据（Excel）
│   │
│   ├── 📝 开发文档组
│   │   ├── README_beckup.md                     # README备份文件
│   │   ├── 预期测试验证流程.md                  # 测试验证计划文档
│   │   └── 日志记录与输出实现参考.txt           # 调试日志实现参考
│   │
│   └── 🖼️ 资源文件组
│       └── minyer logo.bmp                      # 公司LOGO图片（OLED启动显示）
│
├── 📁 .cursor/                       # Cursor IDE配置目录
│   └── 📁 rules/                     # AI编程规则配置
│
├── 📄 README.md                      # 项目说明文档（主文档）
├── 📄 Three-channel controller_v4.0.ioc  # STM32CubeMX配置文件
├── 📄 .mxproject                     # CubeMX项目元数据
└── 📄 create_readme.vbs              # README生成脚本
```

---

### **目录结构说明**

#### **1. Core/ - 核心应用代码**
项目的主要业务逻辑实现，分为两大类：

**CubeMX自动生成文件（🔧标记）**
- 由STM32CubeMX工具自动生成和维护
- **禁止手动修改USER CODE区域之外的代码**
- 重新生成时会覆盖非保护区域

**用户功能模块文件（👤标记）**
- 完全由开发者创建和维护
- 包含8个核心功能模块：
  1. `common_def` - 全局通用定义
  2. `relay_control` - 继电器驱动与互锁控制
  3. `safety_monitor` - 安全监控与异常管理
  4. `temperature` - 温度采集与风扇控制
  5. `ntc_table` - NTC查表数据
  6. `alarm_output` - 多级报警输出
  7. `oled_display` - 人机界面显示
  8. `self_test` - 系统自检流程

#### **2. Drivers/ - 驱动库**
- STM32官方HAL库驱动（**严禁修改**）
- CMSIS标准接口层（**严禁修改**）

#### **3. MDK-ARM/ - 工程文件**
- Keil开发环境工程配置
- 启动文件和链接脚本
- 编译输出目录

#### **4. DOC/ - 文档资料**
- **硬件规格书**：OLED、FLASH、NTC、光耦等器件datasheet
- **设计数据**：NTC温度-电阻对照表
- **开发文档**：测试计划、日志参考
- **资源文件**：公司LOGO图片

#### **5. 根目录配置文件**
- `README.md` - 项目完整说明文档
- `.ioc` - CubeMX硬件配置文件（**修改硬件配置的唯一入口**）
- `.cursor/` - AI编程助手规则配置

---

### **文件命名规范**

| 文件类型 | 命名规则 | 示例 |
|---------|---------|------|
| 外设配置 | 小写外设名 | `adc.c` `i2c.h` `usart.c` |
| 功能模块 | 下划线分隔描述性名称 | `relay_control.c` `safety_monitor.h` |
| 通用定义 | `common_def` | `common_def.h` |
| 查表数据 | `xxx_table` | `ntc_table.c` |

---

### **代码组织原则**

1. **外设初始化** → `Core/Src/外设名.c` 的 `MX_外设名_Init()` 函数
2. **业务逻辑** → `Core/Src/功能模块.c` 的自定义函数
3. **中断处理** → `Core/Src/stm32f1xx_it.c` 的 `USER CODE` 区域
4. **主循环调度** → `Core/Src/main.c` 的 `while(1)` 中
5. **全局定义** → `Core/Inc/common_def.h` 集中管理

---

### **开发注意事项**

1. **CubeMX重新生成代码时**：
   - 用户代码必须写在 `/* USER CODE BEGIN */` 和 `/* USER CODE END */` 之间
   - 否则重新生成时会被覆盖

2. **添加新的功能模块时**：
   - 在 `Core/Inc` 创建 `.h` 头文件
   - 在 `Core/Src` 创建对应 `.c` 源文件
   - 在 `main.h` 中包含新模块的头文件
   - 禁止创建新的文件夹

3. **修改硬件配置时**：
   - 必须通过 `.ioc` 文件在CubeMX中修改
   - 不要手动修改外设初始化代码

4. **文件编码**：
   - 所有代码和文档文件必须使用 **UTF-8编码**

---

# **项目开发计划**

## **开发原则**

1. **分层开发**：基础层 → 驱动层 → 功能层 → 应用层
2. **依赖优先**：被依赖的模块优先开发
3. **测试驱动**：每个模块开发完成后必须通过串口测试验证
4. **授权推进**：每阶段完成并得到授权后，才进入下一阶段

---

## **开发阶段划分**

### **📊 开发进度概览**

| 阶段 | 模块 | 文件 | 依赖关系 | 预计工作量 |
|-----|------|------|---------|-----------|
| **阶段0** | 环境准备 | CubeMX配置验证 | 无 | 0.5小时 |
| **阶段1** | 基础层 | common_def | 无 | 1小时 |
| **阶段2** | 驱动层-串口 | usart调试输出 | common_def | 1小时 |
| **阶段3** | 驱动层-NTC | ntc_table | common_def | 1.5小时 |
| **阶段4** | 驱动层-温度 | temperature, adc, dma | ntc_table, usart | 2小时 |
| **阶段5** | 驱动层-OLED | oled_display, i2c | common_def, usart | 2.5小时 |
| **阶段6** | 功能层-报警 | alarm_output, tim | common_def, usart | 2小时 |
| **阶段7** | 功能层-继电器 | relay_control | common_def, usart | 2.5小时 |
| **阶段8** | 功能层-安全监控 | safety_monitor | relay_control, alarm_output | 3小时 |
| **阶段9** | 应用层-自检 | self_test | safety_monitor, temperature, oled | 2.5小时 |
| **阶段10** | 应用层-主程序 | main整合 | 所有模块 | 2小时 |
| **阶段11** | 系统测试 | 完整功能验证 | 所有模块 | 3小时 |

**总计**：约 23.5 小时纯开发时间

---

## **详细开发计划**

---

### **🔧 阶段0：开发环境准备与验证**

#### **开发内容**
1. 验证CubeMX配置是否完整
2. 验证Keil工程能否正常编译
3. 确认调试串口USART3基本通信

#### **交付文件**
- 无新增文件（验证现有配置）

#### **测试方法**
```c
// 在main.c的while(1)中测试
HAL_UART_Transmit(&huart3, "System Ready\r\n", 14, 100);
HAL_Delay(1000);
```

#### **验收标准**
- ✅ 工程编译无错误
- ✅ 程序能正常下载到MCU
- ✅ 串口工具能收到"System Ready"消息（每秒1次）

#### **依赖关系**
- 无依赖

---

### **📦 阶段1：基础层 - 通用定义模块**

#### **开发内容**
创建项目全局通用定义，包括：
- 数据类型定义
- 异常标志枚举（15种异常A~O）
- 通道状态枚举
- 继电器状态枚举
- 蜂鸣器模式枚举
- 全局常量定义（温度阈值、时间常量等）
- 调试宏定义

**重要说明**：GPIO引脚宏定义已由CubeMX自动生成在 `Core/Inc/main.h` 中（第60-130行），包括K1_EN_Pin/Port、K2_EN_Pin/Port等所有引脚定义。本阶段只需定义业务逻辑相关的通用类型和常量，直接使用main.h中的引脚宏即可。

#### **交付文件**
- `Core/Inc/common_def.h`
- `Core/Src/common_def.c`

#### **核心内容**
```c
// common_def.h 主要内容
- 异常类型枚举：ErrorType_e (A~O共15种)
- 通道枚举：Channel_e (CHANNEL_1/2/3/NONE)
- 继电器状态枚举：RelayState_e (OFF/ON)
- 蜂鸣器模式枚举：BeepMode_e (CONTINUOUS/PULSE_50MS/PULSE_1S/OFF)
- 异常标志位图：uint16_t ErrorFlags (16位位图)
- 全局常量：温度阈值(35℃/60℃)、脉冲时长(500ms)、防抖次数(3次)等
- 调试宏：DEBUG_PRINTF()、LOG_INFO()、LOG_ERROR()等

// 引脚宏定义使用说明
- 所有GPIO引脚宏已在main.h中由CubeMX生成，无需重复定义
- 直接使用：K1_EN_Pin, K1_EN_GPIO_Port, ALARM_Pin等
- 示例：HAL_GPIO_ReadPin(K1_EN_GPIO_Port, K1_EN_Pin)
```

#### **测试方法**
```c
// 在main.c中测试
#include "common_def.h"

void Test_CommonDef(void)
{
    // 测试枚举定义
    Channel_e ch = CHANNEL_1;
    ErrorType_e err = ERROR_TYPE_A;
    
    // 通过串口输出测试
    printf("Common_Def Test:\r\n");
    printf("Channel: %d\r\n", ch);
    printf("Error Type: %d\r\n", err);
    printf("Test PASS\r\n\r\n");
}
```

#### **验收标准**
- ✅ 文件编译无错误无警告
- ✅ 15种异常类型枚举定义完整
- ✅ 通道、状态枚举定义正确
- ✅ 串口输出测试信息正常

#### **依赖关系**
- 无依赖（基础模块）

---

### **🔌 阶段2：驱动层 - 调试串口增强**

#### **开发内容**
增强串口调试功能，实现：
- printf重定向到USART3
- 格式化输出函数
- 时间戳输出（基于HAL_GetTick()）
- 日志级别输出（INFO/WARNING/ERROR）

#### **交付文件**
- 修改 `Core/Src/usart.c`（在USER CODE区域）
- 修改 `Core/Inc/usart.h`（在USER CODE区域）

#### **核心内容**
```c
// usart.c 增强功能
- int fputc(int ch, FILE *f)  // printf重定向
- void UART_Printf(const char *format, ...)  // 格式化输出
- void UART_Log(const char *level, const char *msg)  // 日志输出
- void UART_PrintTimestamp(void)  // 时间戳输出
```

#### **测试方法**
```c
// 在main.c中测试
void Test_UART(void)
{
    printf("\r\n========== UART Test ==========\r\n");
    printf("[%lu ms] System Start\r\n", HAL_GetTick());
    
    UART_Log("INFO", "UART module initialized");
    UART_Log("WARNING", "This is a warning test");
    UART_Log("ERROR", "This is an error test");
    
    printf("Integer: %d\r\n", 12345);
    printf("Hex: 0x%X\r\n", 0xABCD);
    printf("Float: %.2f\r\n", 3.1415f);
    printf("Test PASS\r\n");
    printf("===============================\r\n\r\n");
}
```

#### **验收标准**
- ✅ printf函数能正常输出到串口
- ✅ 支持%d、%x、%f等格式化输出
- ✅ 时间戳功能正常
- ✅ 日志级别显示正确

#### **依赖关系**
- 依赖：`common_def.h`

---

### **📊 阶段3：驱动层 - NTC查表模块**

#### **开发内容**
实现NTC热敏电阻温度查表功能：
- 导入NTC RT值表（从DOC/NTC热敏电阻RT值.csv）
- 实现ADC值到温度的转换函数
- 实现线性插值算法（提高精度）

#### **交付文件**
- `Core/Inc/ntc_table.h`
- `Core/Src/ntc_table.c`

#### **核心内容**
```c
// ntc_table.h
typedef struct {
    uint16_t adc_value;  // ADC采样值
    int16_t temperature; // 对应温度（0.1℃单位，如250表示25.0℃）
} NTC_Table_t;

// 函数接口
int16_t NTC_GetTemperature(uint16_t adc_value);  // ADC值转温度
void NTC_PrintTable(void);  // 打印查表（调试用）
```

#### **测试方法**
```c
// 在main.c中测试
void Test_NTC_Table(void)
{
    printf("\r\n========== NTC Table Test ==========\r\n");
    
    // 测试已知ADC值
    uint16_t test_adc[] = {500, 1000, 1500, 2000, 2500, 3000, 3500};
    
    for(int i = 0; i < 7; i++)
    {
        int16_t temp = NTC_GetTemperature(test_adc[i]);
        printf("ADC: %4d -> Temp: %3d.%d°C\r\n", 
               test_adc[i], temp/10, abs(temp%10));
    }
    
    printf("Test PASS\r\n");
    printf("====================================\r\n\r\n");
}
```

#### **验收标准**
- ✅ NTC表数据导入正确（至少覆盖-40℃~125℃）
- ✅ ADC值转温度计算正确
- ✅ 插值算法精度达到±0.5℃
- ✅ 边界值处理正常（过低/过高ADC值）

#### **依赖关系**
- 依赖：`common_def.h`

---

### **🌡️ 阶段4：驱动层 - 温度检测与风扇控制**

#### **开发内容**
实现温度采集与风扇PWM控制：
- ADC+DMA连续采集3路NTC
- 温度值滤波处理（移动平均）
- 风扇PWM控制（50%/95%）
- 温度阈值判断（35℃、60℃）
- 温度回差处理（2℃）

#### **交付文件**
- `Core/Inc/temperature.h`
- `Core/Src/temperature.c`
- 修改 `Core/Src/adc.c`（USER CODE区域）
- 修改 `Core/Src/dma.c`（USER CODE区域）
- 修改 `Core/Src/tim.c`（USER CODE区域，FAN_PWM）

#### **核心内容**
```c
// temperature.h
typedef struct {
    uint16_t adc_raw[3];      // 原始ADC值
    int16_t temperature[3];   // 温度值（0.1℃单位）
    uint8_t fan_speed;        // 风扇占空比（0-100）
    uint8_t overheat_flag[3]; // 过温标志
} Temperature_Manager_t;

// 函数接口
void Temperature_Init(void);
void Temperature_Update(void);  // 1秒调用1次
void Temperature_GetValues(int16_t *t1, int16_t *t2, int16_t *t3);
uint8_t Temperature_GetFanSpeed(void);
void Temperature_PrintStatus(void);
```

#### **测试方法**
```c
// 在main.c中测试
void Test_Temperature(void)
{
    Temperature_Init();
    
    printf("\r\n========== Temperature Test ==========\r\n");
    printf("Testing for 10 seconds...\r\n\r\n");
    
    for(int i = 0; i < 10; i++)
    {
        HAL_Delay(1000);
        Temperature_Update();
        
        int16_t t1, t2, t3;
        Temperature_GetValues(&t1, &t2, &t3);
        
        printf("[%2ds] NTC1:%3d.%d°C  NTC2:%3d.%d°C  NTC3:%3d.%d°C  FAN:%3d%%\r\n",
               i+1, t1/10, abs(t1%10), t2/10, abs(t2%10), 
               t3/10, abs(t3%10), Temperature_GetFanSpeed());
    }
    
    printf("\nTest PASS\r\n");
    printf("======================================\r\n\r\n");
}
```

#### **验收标准**
- ✅ ADC+DMA采集正常（3通道）
- ✅ 温度值计算正确
- ✅ 风扇PWM输出正常（可用示波器验证）
- ✅ 温度阈值切换正确（35℃、60℃）
- ✅ 2℃回差机制工作正常

#### **依赖关系**
- 依赖：`ntc_table.h`、`common_def.h`、`usart.h`

---

### **📺 阶段5：驱动层 - OLED显示驱动**

#### **开发内容**
实现OLED显示功能：
- I2C通信驱动（SSD1309芯片）
- OLED初始化序列
- 基础绘图函数（清屏、画点、画线、画矩形）
- 字符显示函数（ASCII 6x8、8x16字体）
- 中文显示函数（16x16字库，常用汉字）
- 区域管理（三区域显示）

#### **交付文件**
- `Core/Inc/oled_display.h`
- `Core/Src/oled_display.c`
- 修改 `Core/Src/i2c.c`（USER CODE区域）

#### **核心内容**
```c
// oled_display.h
// OLED显示区域定义
typedef enum {
    OLED_AREA_ALARM = 0,    // 异常信息区（上）
    OLED_AREA_CHANNEL,      // 通道状态区（中）
    OLED_AREA_TEMP          // 温度风扇区（下）
} OLED_Area_e;

// 函数接口
void OLED_Init(void);
void OLED_Clear(void);
void OLED_ShowString(uint8_t x, uint8_t y, const char *str);
void OLED_ShowChinese(uint8_t x, uint8_t y, const char *cn);
void OLED_DrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);
void OLED_ShowLogo(void);  // 显示公司LOGO
void OLED_ShowProgress(uint8_t percent);  // 进度条
void OLED_Refresh(void);  // 刷新显示
```

#### **测试方法**
```c
// 在main.c中测试
void Test_OLED(void)
{
    printf("\r\n========== OLED Test ==========\r\n");
    
    OLED_Init();
    printf("OLED initialized\r\n");
    
    // 测试1：清屏
    OLED_Clear();
    HAL_Delay(500);
    
    // 测试2：显示字符串
    OLED_ShowString(0, 0, "OLED Test");
    OLED_ShowString(0, 2, "Line 2");
    OLED_ShowString(0, 4, "Line 4");
    OLED_Refresh();
    HAL_Delay(2000);
    
    // 测试3：显示LOGO
    OLED_Clear();
    OLED_ShowLogo();
    HAL_Delay(2000);
    
    // 测试4：进度条
    OLED_Clear();
    for(int i = 0; i <= 100; i += 10)
    {
        OLED_ShowProgress(i);
        printf("Progress: %d%%\r\n", i);
        HAL_Delay(300);
    }
    
    printf("Test PASS\r\n");
    printf("===============================\r\n\r\n");
}
```

#### **验收标准**
- ✅ I2C通信正常
- ✅ OLED能正常初始化并显示
- ✅ 字符显示清晰
- ✅ LOGO显示正常
- ✅ 进度条动画流畅
- ✅ 三区域布局正确

#### **依赖关系**
- 依赖：`common_def.h`、`usart.h`

---

### **🚨 阶段6：功能层 - 报警输出控制**

#### **开发内容**
实现多级报警控制：
- ALARM引脚控制
- 蜂鸣器分级控制（持续/50ms脉冲/1秒脉冲）
- 基于TIM定时器的非阻塞脉冲生成
- 异常标志位图管理（16位）

#### **交付文件**
- `Core/Inc/alarm_output.h`
- `Core/Src/alarm_output.c`
- 修改 `Core/Src/tim.c`（USER CODE区域，配置TIM用于蜂鸣器）

#### **核心内容**
```c
// alarm_output.h
void Alarm_Init(void);
void Alarm_SetError(ErrorType_e error);      // 设置异常
void Alarm_ClearError(ErrorType_e error);    // 清除异常
uint8_t Alarm_HasError(void);                // 是否有异常
uint16_t Alarm_GetErrorFlags(void);          // 获取异常位图
void Alarm_Update(void);  // 定时调用，更新蜂鸣器状态
void Alarm_PrintStatus(void);
```

#### **测试方法**
```c
// 在main.c中测试
void Test_Alarm(void)
{
    Alarm_Init();
    
    printf("\r\n========== Alarm Test ==========\r\n");
    
    // 测试1：A类异常（1秒脉冲）
    printf("Test A-type error (1s pulse)...\r\n");
    Alarm_SetError(ERROR_TYPE_A);
    for(int i = 0; i < 5; i++) {
        HAL_Delay(1000);
        Alarm_Update();
    }
    Alarm_ClearError(ERROR_TYPE_A);
    
    // 测试2：B类异常（50ms脉冲）
    printf("Test B-type error (50ms pulse)...\r\n");
    Alarm_SetError(ERROR_TYPE_B);
    for(int i = 0; i < 20; i++) {
        HAL_Delay(100);
        Alarm_Update();
    }
    Alarm_ClearError(ERROR_TYPE_B);
    
    // 测试3：K类异常（持续）
    printf("Test K-type error (continuous)...\r\n");
    Alarm_SetError(ERROR_TYPE_K);
    for(int i = 0; i < 5; i++) {
        HAL_Delay(1000);
        Alarm_Update();
    }
    Alarm_ClearError(ERROR_TYPE_K);
    
    printf("Test PASS\r\n");
    printf("================================\r\n\r\n");
}
```

#### **验收标准**
- ✅ ALARM引脚输出正确
- ✅ 蜂鸣器三种模式工作正常
- ✅ 异常位图管理正确
- ✅ 多异常同时存在时优先级正确（K>B>A）

#### **依赖关系**
- 依赖：`common_def.h`、`usart.h`

---

### **⚡ 阶段7：功能层 - 继电器控制模块**

#### **开发内容**
实现继电器驱动与互锁控制：
- 三通道互锁逻辑
- 双路继电器控制（ON/OFF）
- 500ms脉冲生成（非阻塞）
- 状态反馈检测（6路继电器+3路接触器）
- 防抖检测（50ms间隔3次检测）

#### **交付文件**
- `Core/Inc/relay_control.h`
- `Core/Src/relay_control.c`
- 修改 `Core/Src/gpio.c`（USER CODE区域）
- 修改 `Core/Src/stm32f1xx_it.c`（USER CODE区域，K1/K2/K3_EN中断）

#### **核心内容**
```c
// relay_control.h
typedef struct {
    Channel_e active_channel;   // 当前激活通道
    uint8_t relay_state[3][2];  // 继电器状态[通道][路]
    uint8_t switch_state[3];    // 接触器状态
} Relay_Manager_t;

// 函数接口
void Relay_Init(void);
void Relay_OpenChannel(Channel_e channel);   // 打开通道
void Relay_CloseChannel(Channel_e channel);  // 关闭通道
uint8_t Relay_CheckState(Channel_e channel); // 检查状态
void Relay_Update(void);  // 状态机更新
void Relay_PrintStatus(void);
```

#### **测试方法**
```c
// 在main.c中测试
void Test_Relay(void)
{
    Relay_Init();
    
    printf("\r\n========== Relay Test ==========\r\n");
    
    // 测试1：打开通道1
    printf("Opening Channel 1...\r\n");
    Relay_OpenChannel(CHANNEL_1);
    HAL_Delay(1000);
    Relay_PrintStatus();
    
    HAL_Delay(2000);
    
    // 测试2：关闭通道1
    printf("Closing Channel 1...\r\n");
    Relay_CloseChannel(CHANNEL_1);
    HAL_Delay(1000);
    Relay_PrintStatus();
    
    HAL_Delay(2000);
    
    // 测试3：切换到通道2
    printf("Opening Channel 2...\r\n");
    Relay_OpenChannel(CHANNEL_2);
    HAL_Delay(1000);
    Relay_PrintStatus();
    
    printf("Test PASS\r\n");
    printf("================================\r\n\r\n");
}
```

#### **验收标准**
- ✅ 继电器控制脉冲正确（500ms）
- ✅ 互锁机制工作正常（不能同时打开多通道）
- ✅ 状态反馈检测正确
- ✅ 防抖机制有效
- ✅ 中断响应正常

#### **依赖关系**
- 依赖：`common_def.h`、`usart.h`

---

### **🛡️ 阶段8：功能层 - 安全监控系统**

#### **开发内容**
实现完整的安全监控功能：
- 15种异常检测逻辑
- 真值表状态判断
- 智能异常解除机制
- 电源监控（DC_CTRL中断）
- 与继电器、温度、报警模块集成

#### **交付文件**
- `Core/Inc/safety_monitor.h`
- `Core/Src/safety_monitor.c`
- 修改 `Core/Src/stm32f1xx_it.c`（USER CODE区域，DC_CTRL中断）

#### **核心内容**
```c
// safety_monitor.h
void Safety_Init(void);
void Safety_Update(void);  // 主循环调用，检测所有异常
void Safety_CheckEnableConflict(void);   // 检测A类异常
void Safety_CheckRelayFeedback(void);    // 检测B~G类异常
void Safety_CheckSwitchFeedback(void);   // 检测H~J类异常
void Safety_CheckTemperature(void);      // 检测K~M类异常
void Safety_CheckPowerSupply(void);      // 检测O类异常
void Safety_TryClearErrors(void);        // 尝试解除异常
void Safety_PrintStatus(void);
```

#### **测试方法**
```c
// 在main.c中测试
void Test_Safety(void)
{
    Safety_Init();
    
    printf("\r\n========== Safety Monitor Test ==========\r\n");
    
    // 测试1：正常状态
    printf("Test 1: Normal state\r\n");
    Safety_Update();
    Safety_PrintStatus();
    HAL_Delay(1000);
    
    // 测试2：模拟使能冲突（通过人为触发）
    printf("\nTest 2: Simulate enable conflict\r\n");
    printf("Please trigger K1_EN and K2_EN simultaneously\r\n");
    printf("Monitoring for 10 seconds...\r\n");
    for(int i = 0; i < 10; i++) {
        HAL_Delay(1000);
        Safety_Update();
        if(Alarm_HasError()) {
            Safety_PrintStatus();
        }
    }
    
    printf("\nTest PASS\r\n");
    printf("=========================================\r\n\r\n");
}
```

#### **验收标准**
- ✅ 15种异常检测逻辑正确
- ✅ 真值表判断准确
- ✅ 异常触发时报警正常
- ✅ 异常解除条件正确
- ✅ 与其他模块集成无冲突

#### **依赖关系**
- 依赖：`relay_control.h`、`temperature.h`、`alarm_output.h`、`common_def.h`

---

### **🔍 阶段9：应用层 - 系统自检模块**

#### **开发内容**
实现完整的系统自检流程：
- LOGO显示（2秒）
- 四步自检流程（3秒，带进度条）
  1. 期望状态识别
  2. 继电器纠错
  3. 接触器检查
  4. 温度安全检测
- 自检结果判定
- 失败时产生N类异常

#### **交付文件**
- `Core/Inc/self_test.h`
- `Core/Src/self_test.c`

#### **核心内容**
```c
// self_test.h
typedef enum {
    SELF_TEST_IDLE = 0,
    SELF_TEST_LOGO,
    SELF_TEST_STEP1_STATE_IDENTIFY,
    SELF_TEST_STEP2_RELAY_CHECK,
    SELF_TEST_STEP3_SWITCH_CHECK,
    SELF_TEST_STEP4_TEMP_CHECK,
    SELF_TEST_PASS,
    SELF_TEST_FAIL
} SelfTest_State_e;

// 函数接口
void SelfTest_Start(void);
uint8_t SelfTest_IsRunning(void);
uint8_t SelfTest_IsPassed(void);
void SelfTest_Update(void);  // 状态机更新
void SelfTest_PrintResult(void);
```

#### **测试方法**
```c
// 在main.c中测试
void Test_SelfTest(void)
{
    printf("\r\n========== Self Test ==========\r\n");
    
    SelfTest_Start();
    
    while(SelfTest_IsRunning())
    {
        SelfTest_Update();
        HAL_Delay(100);
    }
    
    SelfTest_PrintResult();
    
    if(SelfTest_IsPassed()) {
        printf("Self-test PASSED\r\n");
    } else {
        printf("Self-test FAILED\r\n");
    }
    
    printf("===============================\r\n\r\n");
}
```

#### **验收标准**
- ✅ LOGO显示2秒
- ✅ 进度条显示流畅
- ✅ 四步自检逻辑正确
- ✅ 继电器纠错功能正常
- ✅ 自检失败时触发N类异常

#### **依赖关系**
- 依赖：`safety_monitor.h`、`relay_control.h`、`temperature.h`、`oled_display.h`

---

### **🎯 阶段10：应用层 - 主程序集成**

#### **开发内容**
整合所有模块到main.c：
- 系统初始化流程
- 主循环调度（任务调度器）
- 定时任务管理（1ms/10ms/100ms/1s）
- 中断与主循环协调
- 完整的启动流程

#### **交付文件**
- 修改 `Core/Src/main.c`（USER CODE区域）
- 修改 `Core/Inc/main.h`（USER CODE区域）

#### **核心内容**
```c
// main.c 主循环结构
int main(void)
{
    // 硬件初始化
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_DMA_Init();
    MX_ADC1_Init();
    MX_I2C1_Init();
    MX_SPI2_Init();
    MX_TIM3_Init();
    MX_USART3_UART_Init();
    
    // 功能模块初始化
    UART_Printf_Init();
    Temperature_Init();
    OLED_Init();
    Alarm_Init();
    Relay_Init();
    Safety_Init();
    
    // 系统自检
    SelfTest_Start();
    while(SelfTest_IsRunning()) {
        SelfTest_Update();
        HAL_Delay(100);
    }
    
    // 主循环
    uint32_t tick_1ms = 0;
    uint32_t tick_10ms = 0;
    uint32_t tick_100ms = 0;
    uint32_t tick_1s = 0;
    
    while(1)
    {
        uint32_t now = HAL_GetTick();
        
        // 1ms任务
        if(now - tick_1ms >= 1) {
            tick_1ms = now;
            Relay_Update();
            Alarm_Update();
        }
        
        // 10ms任务
        if(now - tick_10ms >= 10) {
            tick_10ms = now;
            // 保留
        }
        
        // 100ms任务
        if(now - tick_100ms >= 100) {
            tick_100ms = now;
            Safety_Update();
        }
        
        // 1s任务
        if(now - tick_1s >= 1000) {
            tick_1s = now;
            Temperature_Update();
            // OLED_UpdateDisplay();
        }
    }
}
```

#### **测试方法**
```c
// 完整系统运行测试
1. 上电观察LOGO显示
2. 观察自检进度条
3. 观察三区域界面显示
4. 手动触发K1_EN，观察通道1打开
5. 观察温度实时更新
6. 模拟异常，观察报警输出
7. 连续运行24小时稳定性测试
```

#### **验收标准**
- ✅ 系统启动流程完整
- ✅ 所有模块正常工作
- ✅ 任务调度时序正确
- ✅ CPU占用率<30%
- ✅ 无内存泄漏
- ✅ 长时间运行稳定

#### **依赖关系**
- 依赖：所有模块

---

### **✅ 阶段11：系统集成测试**

#### **测试内容**

**功能测试清单**：
1. 系统启动自检流程
2. 三通道切换功能
3. 互锁保护机制
4. 双路继电器控制
5. 状态反馈检测
6. 温度检测与风扇控制
7. 15种异常检测
8. 多级报警输出
9. 异常解除机制
10. OLED三区域显示
11. 串口调试输出
12. 电源监控保护

**性能测试**：
- 响应时间：<500ms
- CPU占用率：<30%
- 内存占用：<10KB
- 稳定性：连续运行24小时无故障

**边界测试**：
- 极端温度测试（-10℃~80℃）
- 快速切换测试（100次连续切换）
- 异常恢复测试
- 断电重启测试

#### **验收标准**
- ✅ 所有功能测试通过
- ✅ 性能指标达标
- ✅ 边界测试通过
- ✅ 无已知BUG

---

## **开发流程规范**

### **每个阶段的标准流程**

```
1. 需求确认
   ↓
2. 代码开发（严格遵守USER CODE区域）
   ↓
3. 编译验证（无错误无警告）
   ↓
4. 单元测试（串口输出测试结果）
   ↓
5. 提交测试报告（等待授权）
   ↓
6. 获得授权后进入下一阶段
```

### **测试报告格式**

每个阶段完成后，提供如下格式的测试报告：

```
===============================
阶段X：模块名称 - 测试报告
===============================

【开发内容】
- 已完成文件：xxx.c, xxx.h
- 代码行数：xxx行

【编译结果】
- ✅ 编译通过，0错误，0警告

【测试结果】
- ✅ 测试1：描述 - PASS
- ✅ 测试2：描述 - PASS
- ✅ 测试3：描述 - PASS

【串口输出日志】
（粘贴实际测试的串口输出）

【存在问题】
- 无 / 列出问题

【下一步计划】
- 进入阶段X+1：模块名称

【等待授权】
请确认是否继续下一阶段开发
===============================
```

---

## **开发时间估算**

| 阶段类型 | 阶段数 | 预计总时长 |
|---------|-------|-----------|
| 环境准备 | 1 | 0.5小时 |
| 基础层 | 1 | 1小时 |
| 驱动层 | 4 | 7小时 |
| 功能层 | 3 | 7.5小时 |
| 应用层 | 2 | 4.5小时 |
| 系统测试 | 1 | 3小时 |
| **总计** | **12** | **23.5小时** |

*注：以上为纯开发时间，不包括测试、调试、修改的时间*

---

## **风险提示与应对**

| 风险 | 应对措施 |
|------|---------|
| CubeMX配置错误 | 阶段0验证，必要时重新配置 |
| 硬件调试问题 | 每阶段充分测试，及时发现 |
| 模块集成冲突 | 严格依赖管理，先测试后集成 |
| 时间延期 | 分阶段授权，灵活调整 |

---

## **当前开发状态**

- ✅ 开发计划已制定
- ⏸️ 等待用户授权开始阶段0：开发环境准备与验证