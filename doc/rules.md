# 游戏规则说明

## 地图元素
地图采用矩阵棋盘式地图，方格有水下（下/0）、水面（中/1）、空中（上/2）三个层次。

同一格子的同一层次同时只能被一个元素占据。

元素属性见逻辑组API

### 地形
包括海洋和陆地。

* 海洋：相当于空地，上中下层分别可停留一个单位。
* 陆地：建筑和资源点均在陆地地形上，陆地地形只有空中可停留单位。

### 建筑
包括基地和据点。

#### 基地 Base

* 生命值无法恢复
* 弹药无限

#### 据点 Fort

* 游戏开始时为无主状态，无资源储备。
* 双方可以攻击据点，当据点生命值降为0时，其所有权归于使其生命力降为0的攻击发出方，生命值立即回满，剩余的资源保留。
* 占领方可以控制据点进行攻击、补给或向据点补给
* 可将金属暂时储存在据点，但金属只有在基地才能发挥作用！

### 资源点
包括油田和矿场，均在陆地地形上，不可被攻占。 @逻辑组：为什么油田一定在陆地上？

#### 油田 Oilfield

* 油储备总量fuel_max
* 油田储备的燃料只能由运输舰收集获得。
* 双方基地附近各有一小油田，地图的中间区域可设大油田

#### 矿场 Mine

* 金属储备总量metal_max
* 储备的金属只能由运输舰收集获得
* 双方基地附近各有一小矿场，地图的中间区域可设大矿场

### 单位
单位包括：

* 潜艇 Submarine
* 战列舰 Battleship
* 运输舰 Cargo
* 航空母舰 Carrier
* 巡洋舰 Cruiser
* 驱逐舰 Destroyer
* 机群 Formation
    - 战斗机 Fighter
    - 鱼雷机 Torpedoer
    - 轰炸机 Bomber
    - 侦察机 Scout

#### 潜艇
潜艇能提供成吨的伤害，并且不会被人察觉，神不知鬼不觉的解决掉看起来很叼的航母和战舰。但是本身异常脆弱，适合突袭斩首。

注意： 潜艇只能造成和接受鱼雷伤害，因而无法攻击据点。

#### 船舰
* 驱逐舰：各方面能力均衡，主要反潜和侦查护航突袭，航速高，火力较低
* 巡洋舰：稍微牺牲了移动速度，换来了攻击力提升，对空水平高超
* 战舰：移动速度较慢，但是攻击防御都不逊色，就是干！小秘密：无反潜能力
* 航空母舰：移动速度最慢，兼备攻击和补给能力，有一定防御力但受到鱼雷攻击损伤巨大，无反潜能力
* 运输舰：无攻击能力，运输资源

#### 机群
##### 编队
地图上一个格子内的飞机并不是一架飞机，而是一个飞机编队，每个飞机编队飞机数为常量。
生产飞机时，玩家可自由安排编队内机种配置方案。编队内共四个机种：战斗机、鱼雷机、
轰炸机、侦察机。不同的配置方案会对一个飞机编队（在地图上则呈现为一个飞机单位）
的攻击、侦查范围、生命值产生影响。飞机编队不可合并。

##### 简介
机群可有很高的机动性，但是容易在劣势条件下有很大伤亡，特别是只能攻击所在位置的舰船。需要有航母
或据点的支援才能很好地续航，不然攻击次数少的可怜。愿意战略轰炸的亲们可以尝试绕远偷袭潜艇、
航母、据点和运输船（挂掉不管哦~）

战斗机可以提供大幅制空权值，鱼雷机和轰炸机提供不同种类的输出，侦察机能提供视野，以及对附近
发生的战斗产生有利作用.

##### 能力值
由于编队的具体配置方案是在生产时由玩家自主决定的，故生命值上限、视野范围、攻击力、燃料上限、
弹药上限、各机种初始数量的值只有在游戏过程中才能确定，除了初始数量，其他值还与当前生命值有关。
其中视野范围只决定于编队是否有侦察机。

##### 伤害说明
如果飞机编队受到攻击，则按侦察机、鱼雷机、轰炸机、战斗机的顺序依次扣除


## 游戏模式

### 指令

* 每回合前选手可以为己方单位设定一系列指令。
* 无效的指令将被剔除。
* 若为同一单位多次设定不能同时存在的指令，则保留较后的指令

指令包括：

* 攻击
* 改变目的地
* 收集
* 维修
* 生产
* 补给
* 取消
@逻辑：自爆是否保留？哪些单位收到伤害？

|    单位    | 移动 | 攻击 |      补给     | 收集 | 维修 | 生产 |
|------------|------|------|---------------|------|------|------|
| ***建筑*** |      |      |               |      |      |      |
| 基地       |      | √    | √             |      | √    | √    |
| 据点       |      | √    | √             |      |      |      |
| ***单位*** |      |      |               |      |      |      |
| 航母       | √    | √    | √             |      |      |      |
| 运输舰     | √    |      | √（不含飞机） | √    |      |      |
| 其他       | √    | √    |               |      |      |      |

#### 移动

* 只能在同一层次内移动，不能穿越同层其他单位
* 每移动一格消耗一单位燃料，燃料<=0则不能继续移动。
  特别地，若飞机编队燃料降至零，则坠毁，并对当前坐标的1层（水面/地面）单位造成一定伤害。
* 若飞机编队未发生实际移动，视为原地盘旋一回合，消耗一单位燃料。
* 当某两个同层单位单回合目的地为同一坐标时，速度大的单位移动成功，移动未成功的单位停留在
  前一格不能继续移动，若两单位速度相同，则采取随机的方式决定哪个单位移动成功。

#### 攻击
|   单位   | 可造成伤害 | 可接受伤害 |
|----------|------------|------------|
| **建筑** | 火力       | 火力       |
| **单位** |            |            |
| 潜艇     | 鱼雷       | 鱼雷       |
| 船舰     | ？         | 火力/鱼雷  |
| 机群     | ？         | 火力       |

* 攻击时需指定攻击的操作对象索引号。
* 弹药量小于单次攻击所需弹药时不能发动攻击
* 闪避概率与距离有关

#### 补给
运输舰、航母、岛屿、基地特有指令，

* 航母、岛屿、基地可以对任意单位补给，运输舰不能补给飞机。
* 每次补给补满对方容量，若剩余量不足以补满对方，则全部补充给对方。（警告：运输舰、航母需要保留
一些燃料供自己使用，否则会导致抛锚）（这个不用选手操心，补给时会有限定）
* 补给距离为：0/1（对水下），1（对水面/地面），0（对空）

#### 收集
运输舰特有指令，从岛屿或资源点获取资源

* 每次收集补满运输舰容量
* 收集距离为1

#### 维修
基地特有指令

* 对空维修距离为0，其余维修距离为1
* 维修补满生命值，消耗金属

#### 生产
基地特有指令，给出编码后的单位类型号，生产该类型单位

* 生产后自动附加补给操作
* 生产出的单位在到基地距离为1的对应层次随机出生，若无处可放则滞留在生产列表中
* 生产消耗金属（后续补给操作该消耗什么消耗什么= =），也需要一定的回合数。
* 生产过程并不影响基地的其他操作。

### 结算顺序
结算顺序如下

|   结算事件   |                      更新的值                      |
|--------------|----------------------------------------------------|
| 增加生产条目 | 生产列表，基地金属                                 |
| 攻击         | 攻击方弹药，被攻击方生命值[，据点归属，据点生命值] |
| 飞机坠落     | 地面/水面单位生命值[，据点归属，据点生命值]        |
| 补给         | 补给方资源，被补给方资源                           |
| 维修         | 基地金属，被维修单位生命值和资源（和类型）         |
| 收集         | 资源剩余量，所有请求资源单位的资源                 |
| 改变目的地   | 单位目的地                                         |
| 移动         | 单位坐标，燃料                                     |
| 更新生产列表 | 生产列表中剩余回合，新单位出现在地图中             |










### 距离
    abs(X1-X2)+ abs(Y1-Y2)

### 积分
    总积分 = 对敌伤害积分 + 占领岛屿积分（每回合奖励）+ 收集资源积分

### 初始条件
双方所有单位（包括基地）资源充满，满血。岛屿满血无主。

### 胜利条件
摧毁对方基地，或回合达到回合数上限时积分较高者获胜

### 视野范围
地形全可见（海洋、基地、岛屿、油田、矿场），每个单位均有三层视野范围，己方单位共享视野范围

### 天气
天气weather为在游戏开始就确定的数据，该数据影响所有单位的视野，即

    实际视野 = 固有视野 + weather。

如果出现单位视野受影响至0或负，则该单位只能看到同位置不同层次的敌人。


## 伤害计算
伤害计算计算两次，分别计算火力和鱼雷伤害，弹药消耗为ammo_once
### 伤害计算公式
距离对攻方有一定影响

    修正系数 = 1 – (distance – 1) / (fire_range + 1)  //可能大于1，近距离伤害奖励
    伤害 = （攻击 – 防御）    //空对空伤害另算….
    攻击 = 基本攻击力 * 距离修正 * 攻击方侦察机补正
    防御 = 基本防御力 * 防御方侦察机补正

侦察机补正为常数（暂不确定）

### 闪避
    accuracy = 1 -( (distance) / (fire_range + 1))^2

### 空对空伤害计算
    基本攻击力 = （机体的雷装/火力）* 比例系数 * √搭载数

另外，空对空伤害与双方制空权值air_mastery相关。（制空权的存在是为了~~加速机群的阵亡~~
更好的还原历史）

    air_mastery = 基本攻击力 + 距离为1的援助海军对空攻击力

#### 特别说明：
空军没有护甲（好吧= =），空战双方除非出现完全丧失制空权，否则都会出现损伤，但先发制人好得多~~。

### 制空权
制空权值攻击方与防守方之比决定制空权：

\>3完全制空 攻击加成1/2 攻击方不损失飞机 防守方飞机额外损失结算后的1/3

\>3/2制空权优势 攻击加成1/2 攻击方损失应损失飞机的2/3

1/2~3/2无补正

<1/2制空权劣势 攻击减少1/3 

<1/3完全丧失制空权 进攻方现有飞机再损失结算后的1/3 防守方不损失飞机 可以继续对舰攻击
