SDK文件目录结构说明：
app：应用固件代码，包含照明、传感、电工Demo例程代码。
doc：SDK的相关文档介绍，包含datasheet，zigbeeSPEC以及非常重要的开发手册，注意开发手册资料非常重要请咨询阅读。
include：SDK包含的头文件。
lib：SDK编译的库文件。
sdk: SDK相关文件，开发者不需要操作。
tools:编译使用的相关脚本文件，开发者不需要操作。


app文件目录结构说明：
目录下分别有switch、light_rgb simple_contact_sensro的demo工程文件。

demo文件目录结构说明：
documents:demo设备软件设计说明，非常重要建议开发之前仔细阅读。
EFR32MG21A020F768：工程文件目录，编译生成文件在build中，IAR工程直接用demo打开，不能						通过IAR直接添加。固件位置在./bulid/exe/xx_QIO_.1.0.0.37 
include：demo应用头文件
src：demo中的.c文件。
pre-build-iar.py：脚本文件，开发者不需要操作。


注意：demo工程目录不要随意调整，bin文件与.37文件生产与工程路径相关。