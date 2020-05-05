# MIPS-OS

## 实验环境
- 操作系统：Linux 虚拟机，Ubuntu
- 硬件模拟器：GXemul
- 编译器：mips_4KC-gcc (GCC) 4.0.0 (DENX ELDK 4.1 4.0.0)


### 安装交叉编译工具
在linux环境中安装ELDK构建交叉编译环境：
```bash
# 下载ISO镜像
wget http://ftp.denx.de/pub/eldk/4.1/mips-linux-x86/iso/mips-2007-01-21.iso
```
安装方式1：
```bash
# 安装32位运行库（仅在64位系统上需要执行此步骤）
# sudo apt install ia32-libs
sudo apt install lib32ncurses5 lib32z1

# 建立一个用于挂载ISO镜像的目录
sudo mkdir /mnt/mipsiso

# 挂载iso文件
sudo mount -o loop mips-2007-01-21.iso /mnt/mipsiso

# 切换到/mnt/mipsiso 文件夹中
cd /mnt/mipsiso

# 运行安装脚本
sudo ./install -d /opt/eldk
```
挂载iso文件时可能遇到错误，可用方法2：
```bash
sudo apt install p7zip-full p7zip-rar # Ubuntu下载7z
sudo 7z x mips-2007-01-21.iso -o/mnt/mipsiso # -o指定路径
cd /mnt/mipsiso
sudo chmod a+x install
sudo ./install -d /opt/eldk
```
### 安装仿真器
```bash
# 下载gxemul
wget http://gavare.se/gxemul/src/gxemul-0.6.2.tar.gz

# 解压安装
tar -zxvf gxemul-0.6.2.tar.gz
cd gxemul-0.6.2/
./configure
make
sudo make install
```

## 项目运行
```bash
# 运行项目
make start

# 以调试模式运行项目
make debug
```