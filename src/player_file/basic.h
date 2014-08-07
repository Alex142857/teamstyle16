#ifndef  BASIC_H
#define  BASIC_H

// 地图参数
const int RoundMax = 1000;           // 最大回合数
const int MapSizeMax = 200;          // 地图最大边长
const int FortNumMax = 6;            // 据点最大数
const int MineNumMax = 8;            // 矿场最大数
const int OilfieldNumMax = 8;        // 油田最大数
const int UnitNumMax = 100;          // 每方最大单位数
const int CommandNumMax = 1 + FortNumMax + UnitNumMax; // 每方单回合最大总指令数，其中1为基地数 

// 层次
enum Level { UNDERWATER, SURFACE, AIR };
// 攻击类型
enum AttackType { FIRE, TORPEDO };
// 潜艇只能造成和接受鱼雷伤害
// 陆地建筑只能造成和接受火力伤害
// 飞机不能接受鱼雷伤害
// Attack[0],[1]分别代表火力攻击和鱼雷攻击，
// Defence[0],[1]分别代表对火力防御和对鱼雷防御

// 地形
enum MapType { OCEAN, LAND };       // 海洋，陆地
// 建筑    
enum BuildingType { BASE, FORT };       // 基地和据点为陆地上的建筑
// 资源
enum ResourceType { MINE, OILFIELD };   // 资源点所在地形也均为陆地
// 单位
enum UnitType
{
    SUBMARINE,   // 潜艇
    DESTROYER,   // 驱逐舰
    CRUISER,     // 巡洋舰
    BATTLESHIP,  // 战舰
    CARRIER,     // 航母
    CARGO,       // 运输舰
    FORMATION    // 机群（飞机编队）
};

// 机种
enum PlaneType
{
    FIGHTER,    // 战斗机
    TORPEDOER,  // 鱼雷机
    BOMBER,     // 轰炸机
    SCOUT       // 侦察机
};

///////////////////////////////////////////////////////////////////

struct BuildingProperty
{
    int sight_range[3];
    int fire_range[3];

    int health_max;
    int fuel_max;
    int ammo_max;
    int metal_max;

    int attack[2];
    int defence[2];
};
const BuildingProperty BuildingInfo[2] = {};

struct ResourceProperty
{
    int total;
};
const ResourceProperty ResourceInfo[2] = {};

struct UnitProperty
{
    int sight_range[3];
    int fire_range[3];

    int health_max;
    int fuel_max;
    int ammo_max;

    int speed;
    int attack[2];
    int defence[2];
};
const UnitProperty UnitInfo[6] = {};
// 飞机除外（飞机编队的这些属性不是常量，在飞机结构体中定义）

const int ScoutSightRange[3] = {1, 3, 4};        // 编队内有侦察机时的视野范围
const int NonScoutSightRange[3] = {0, 1, 2};     // 编队内无侦察机时的视野范围

const int FormationFireRange[3] = {};   // 飞机攻击范围为常量
const int FormationSpeed = 5;


struct PlaneProperty
{
    int health_max;
    int fuel_max;
    int ammo_max;

    int attack[2];
    int denfence[2];
};
const PlaneProperty PlaneInfo[4] = {};  // 各机种参数

////////////////////////////////////////////////////////////////

const int IslandScoreTime = 5;   // 连续占有岛屿触发积分奖励的回合数
                                 // @Vone 每回合奖励的分数？

////////////////////////////////////////////


struct Position  // 位置结构体，包含xyz坐标
{
    int x;
    int y;
    int z;
};

// 基地和据点占据不止1个格子，为矩形区域
struct Rectangle
{
    Position upper_left;
    Position lower_right;
    // 两顶点的第三维坐标需相同
};

struct Base  // 基地
{
    Rectangle area;             // 占据的区域
    int health;
    int metal;
    int fuel;
    // 基地弹药无限
};

struct Fort  // 据点
{
    Rectangle area;
    int team;   // team == -1 则无主

    int health;
    int fuel;
    int ammo;
};

struct SubmarineBasic  // 潜艇结构体
{
    Position pos;

    int health;
    int fuel;
    int ammo;
};

struct ShipBasic  // 船舰结构体
{
    Position pos;

    int health;
    int fuel;
    int ammo;
    int metal;
};

struct FormationBasic
{
    Position pos;
    int num[4];             // 各机种数量
    int sight_range[3];
    int fuel_max;
    int ammo_max;  // 燃料上限，弹药上限，与当前num有关
    int fuel;
    int ammo;
    int attack[2];
    int defence[2];
};

struct UnitBasic  // 作战单位基本信息抽象
{
    Position pos;
    UnitType unit_type;
    int team;

    int sight_range[3];
    int fire_range[3];
    int health;
    int fuel;
    int ammo;
    int speed;
    int attack[2];
    int defence[2];
    int metal;  // 运输舰or航母
    int plane_num[4];   // 飞机
};

struct Resource  // 资源：油田、矿场
{
    Position pos;
    int remain;
};

struct MapInfo
{
    int row, col;
    int mine_num, oilfield_num, fort_num;
    int type[MapSizeMax][MapSizeMax];    // 实际地图大小为row * col，数组元素的值与Maptype枚举类型对应
    int unit[MapSizeMax][MapSizeMax][3]; // 地图各处的unit情况，此处unit为广义unit，包含Building和Resource
};

struct GameInfo  // 游戏信息结构体，每回合选手从中获取必要的信息
{
    int team_number;                                // 队伍号(0或1)
    MapInfo map_info;                               // 地图信息
    Resource resource[OilfieldNumMax + MineNumMax]; // 资源点
    Base base[2];                                   // 基地
    Fort fort[FortNumMax];                          // 据点
    int score[2];                                   // 两队当前积分
    int round;                                      // 当前总回合数
    SubmarineBasic submarine[UnitNumMax];           // 己方潜艇信息
    ShipBasic ship[UnitNumMax];                     // 己方船舰信息
    FormationBasic formation[UnitNumMax];           // 己方飞机信息
    int field_of_vision[MapSizeMax][MapSizeMax][3]; // 选手的三层视野范围，-2表示不可见，-1表示无单位，若有单位则为该单位ID
    UnitBasic unit_in_sight[UnitNumMax];            // 己方视野范围内的所有单位的数组
};

enum Order { ATTACK, SUPPLY, FIX, BUILD，EXPLODE };   // 攻击，补给，维修，生产，自爆

struct Command
{  
    Position move_destination;  // 要移动的目的地   移动目的可以为原地，即原地等待
    Order order;                // 攻击，补给，维修，生产，自爆
    Position order_target;      // order操作目标的坐标
} command[CommandNumMax];       // 选手操作,每回合传给逻辑给己方所有单位的指令


#endif
