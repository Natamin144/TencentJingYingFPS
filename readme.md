# 程嘉毅-北京航空航天大学-腾讯游戏客户端菁英班大作业

## 项目简介

![项目截图](./ScreenShots/GameScene.png "项目场景")

Github地址：https://github.com/Natamin144/TencentJingYingFPS

本项目基于Unreal Engine自带的FPS为模板进行修改。模板中只包含了基本的单人单机游戏支持，没有任何网络同步等功能，这些地方需要进行修改。

游戏选用的网络模式为：服务器-客户端模式，在运行时，采用1个服务器+1个客户端的模式进行运营。

#### 游戏规则：

和UE5提供的模板游戏规则相同。服务端玩家和客户端玩家各处于2个队伍，一方玩家杀死另一方玩家5次，则该方获胜。玩家头顶上有计分板，显示当前队伍的得分。玩家的Ui同时包括了玩家的生命值和弹药数。

场地上提供了模板的3种武器：手枪、自动步枪、榴弹发射器。它们可以被玩家拾取，拾取后玩家点击发射。

## 项目加载和运行

加载项目后，将网络模式设置为服务器-客户端模式（以监听服务器运行）。经过测试，在玩家数量为2-4时，可以正常进行游戏。

## 游戏规则完善

模板起初给的ShooterGameMode只有玩家队和人机队。只有基础的死亡计分逻辑。

![得分](./ScreenShots/ScoreAndWinning.png "击杀敌人得分，满5分胜利")

在项目中，我添加了ShooterGameState类，用于存储游戏状态，其中包括了2队伍玩家的得分。

ShooterGameMode和ShooterUI也进行了更进一步的完善。击杀计分规则从模板给的死亡队伍计1分，改为在发生击杀事件时，击杀者队伍记1分。同时设置最大分数，当一方得分达到5分时，游戏获胜。玩家的UI界面会直接弹出 "You Win!"或者"You Lose!"的提示。

## 添加的网络同步

对于网络同步，一部分Actor直接设置为Replicated实时同步。例如玩家本体、玩家身上的枪、场景中发射的子弹。就像图中客户端玩家2使用榴弹发射器攻击客户端玩家1一样。榴弹爆炸冲击波、玩家、玩家手中的武器设置为Replicated，因此客户端1可以看到它们。

![项目截图](./ScreenShots/Playing.png "游玩场景，客户端2使用榴弹发射器攻击客户端1")

一部分使用ReplicatedUsing=OnRep_...。例如玩家生命值的同步。当玩家执行OnHealthUpdate更新当前生命值时，会触发OnRep_CurrentHealth更新所有客户端的生命值。

一部分则使用RPC多播进行执行。例如ChangeIntoWeapon - 玩家切换武器时，需要通知服务器，进而通知所有玩家。

客户端调用ChangeIntoWeapon时，会调用ServerChangeIntoWeapon，通知服务器切换玩家的武器。服务器执行ServerChangeIntoWeapon后，通过MulticastChangeIntoWeapon广播所有客户端执行DoChangeIntoWeapon，也就是切换武器的主体逻辑（收起旧武器，拿起新武器）。

还有很多的RPC，比如请求武器开火为客户端→服务端；玩家捡起武器、玩家角色死亡，则为服务端→客户端

## 遇到的困难与项目不足之处

这个项目对于我来说，还有很多地方需要改进。这些缺点成为了我的经历和教训，也是我未来在游戏等项目程序设计中将会避免的地方。

项目在类的设计上，起初没有考虑好继承结构。导致AShooterCharacter类将游戏角色和玩家操控者身份高度耦合。当后面想要需要添加AI角色时，AI角色不能复用很多作为射击者角色能够调用的代码，如CurrentWeapon、TeamByte等。

项目在源码编译上没有找到快速便捷的编译工具。使用VS进行编译时，将近3小时编译时间难以接受，且编译报错多样不好解决，因此只在从模板生成项目时进行过1次编译。UE5的Live Coding在第一次编译时同样需要相当长的编译时间，在编译时可能会出现Live Coding崩溃，导致需要重新加载整个UE5项目的情况。这曾经给我带来过困难。创建ShooterGameState.cpp时，由于只使用Live Coding，没有进行编译。导致每次进入UE5都会因为ShooterGameState.cpp不存在，有一部分蓝图绑定由于刚加载时未编译而缺失，需要重新绑定。这两条连接包括：

* BP_ShooterGameMode中，游戏状态类设置为ShooterGameState(C++类)，否则击杀计分不能运行
* BP_ShooterPlayerController中，Shooter UiClass设置为UI_Shooter，否则玩家头顶的计分板、血条等UI无法显示