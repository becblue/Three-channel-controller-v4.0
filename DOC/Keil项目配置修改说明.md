# Keil项目配置修改说明

## 需要手动操作的步骤

由于删除了`ntc_table.c`和`ntc_table.h`文件，需要从Keil项目中移除引用。

### 操作步骤：

1. **打开Keil项目**
   - 打开 `MDK-ARM\Three-channel controller_v4.0.uvprojx`

2. **移除ntc_table.c**
   - 在左侧项目树中，找到 `Application/User/Core` 组
   - 右键点击 `ntc_table.c`
   - 选择 **"Remove File from Group"**

3. **保存项目**
   - 点击 File → Save All

4. **重新编译**
   - 点击编译按钮或按 F7
   - 应该显示：**0 Error, 0 Warning**

### 原因说明

`ntc_table.c/h`已被删除，因为：
- `temperature.c`已经包含完整的查表功能（166个点）
- 避免代码冗余和混淆
- 简化项目结构

### 验证

编译成功后，预期代码大小应略微减小（约5-6KB）。

---

**注意**：此操作已提交到Git，但Keil项目文件(.uvprojx)需要手动修改。
