## 项目加载

在Unreal Engine 加载后，先执行Live Coding编译代码

有一部分蓝图绑定由于刚加载时未编译而缺失，需要重新绑定。

具体位置包括：

* BP_ShooterGameMode中，游戏状态类设置为ShooterGameState(C++类)，否则击杀计分不能运行
* BP_ShooterPlayerController中，Shooter UiClass设置为UI_Shooter，否则玩家头顶的计分板、血条等UI无法显示