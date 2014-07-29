# -*- coding: UTF-8 -*-
# basic.py

# 缩进: 4空格
# 命名: (参考 Google style)
# 类名: CapWords
# 模块名: lower_with_under
# 函数或方法名: firstLowerCapWords()
# 常量名: CAPS_WITH_UNDER
# 其他变量,实例及函数形参: lower_with_under 

# 变量名开头加一个下划线(_)能对保护模块(protected)中的变量及函数提供一些支持(不会被 import * from 导入)
# 在实例的变量和方法开头加两个下划线(__)能有效地帮助把该变量或方法变成类的私有内容(using name mangling)

# 以下所有数据暂时并无理论依据...

# 基础参数限制
ROUND_MAX = 1000    # 最大回合数
MAP_SIZE_MAX = 200  # 地图最大边长
BASE_NUM_MAX = 1    # 每方基地最大数量
FORT_NUM_MAX = 6    # 据点最大数量
MINE_NUM_MAX = 8    # 矿场最大数量
OILFIELD_NUM_MAX = 8    # 油田最大数量
MOVEABLE_UNIT_NUM_MAX = 100      # 每方最大可移动单位数
COMMAND_NUM_MAX = BASE_NUM_MAX + FORT_NUM_MAX + MOVEABLE_UNIT_NUM_MAX    # 每方单回合最大总指令数
INFINITY = float('inf')     # 正无穷, 大于任何有限数


# 维修代价
METAL_PER_HEALTH = 0.2    # 恢复1点生命所需金属


# 地图分层
UNDERWATER = 0  # 水下
SURFACE = 1     # 水面 or 地面
AIR = 2         # 空中


# 伤害类型
""" 潜艇只能造成和接受鱼雷伤害
        陆地建筑只能造成和受到火力伤害
        飞机不能受到鱼雷伤害 """
FIRE = 0        # 火力伤害
TORPEDO = 1     # 鱼雷伤害


# 地形
OCEAN = 0       # 海洋
LAND = 1        # 陆地


# 建筑
BASE = 0        # 基地
FORT = 1        # 据点


# 资源点
MINE = 0        # 矿场
OILFIELD = 1    # 油田


# 可移动单位
SUBMARINE = 0   # 潜艇
DESTROYER = 1   # 驱逐舰
CRUISER = 2     # 巡洋舰
BATTLESHIP = 3  # 战舰
CARRIER = 4     # 航母
CARGO = 5       # 运输舰
FORMATION = 6   # 飞机编队(机群)


# 飞机编队内飞机种类
FIGHTER = 0     # 战斗机
TORPEDOER = 1   # 鱼雷机
BOMBER = 2      # 轰炸机
SCOUT = 3       # 侦察机


# 建筑属性
""" building_property = (sight_ranges, fire_ranges, 
                                                 health_max, fuel_max, ammo_max, metal_max, 
                                                 attacks, defences)
        其中 sight_ranges = [UNDERWATER, SURFACE, AIR]
                 fire_ranges = [UNDERWATER, SURFACE, AIR]
                 attacks = [FIRE, TORPEDO]
                 defences = [FIRE, TORPEDO] """
BUILDINGS = [([4, 10, 8], [0, 7, 5], 
              2000, 1000, INFINITY, 200, 
              [50, 0], [30, INFINITY]),     # 基地
             ([3, 8, 6], [0, 5, 4], 
              800, 200, 300, 0, 
              [30, 0], [12, INFINITY])]      # 据点


# 可移动单位(除飞机)属性
""" moveable_unit_except_formation_property = (sight_ranges, fire_ranges,
                                                                                             health_max, fuel_max, ammo_max, metal_max, 
                                                                                             speed,
                                                                                             attacks, defences) """
# 数据不可信...
MOVEABLE_UNITS_EXCEPT_FORMATION = [([6, 5, 3], [5, 5, 0],
                                    50, 120, 20, 0,
                                    5,
                                    [0, 18], [INFINITY, 10]),    # 潜艇
                                   ([5, 10, 8], [4, 8, 6],
                                    80, 150, 30, 0,
                                    8,
                                    [12, 20], [10, 10]),          # 驱逐舰
                                   ([5, 10, 8], [4, 8, 6],
                                    85, 300, 30, 0,
                                    7,
                                    [20, 12], [10, 8]),          # 巡洋舰
                                   ([5, 10, 8], [4, 8, 6],
                                    200, 200, 50, 0,
                                    5,
                                    [30, 12], [16, 16]),          # 战舰
                                   ([5, 10, 8], [4, 8, 6],
                                    600, 400, 70, 80,
                                    6,
                                    [15, 12], [20, 15]),          # 航母
                                   ([5, 10, 8], [4, 8, 6],
                                    170, 300, 40, 20,
                                    7,
                                    [0, 0], [10, 8])]          # 运输舰


# 飞机常量属性
FORMATION_SIGHT_RANGES_WITH_SCOUT = [2, 12, 16]
FORMATION_SIGHT_RANGES_WITHOUT_SCOUT = [0, 8, 10]
FORMATION_FIRE_RANGES = [0, 8, 10]
FORMATION_SPEED = 12
FORMATION_TOTAL_PLANES = 10     # 一个机群最多10架飞机


# 各机种参数
""" plane_property = (health_max, fuel_max, ammo_max,
                                            attacks, defences) """
PLANES = [(7, 10, 5, 
           [2, 0], [1, INFINITY]),    # 单架战斗机
          (6, 10, 4,
           [0, 2], [1, INFINITY]),    # 单架鱼雷机
          (8, 10, 4,
           [3, 0], [2, INFINITY]),    # 单架轰炸机
          (5, 15, 2,
           [1, 0], [0, INFINITY])]    # 单架侦察机


# 对象

class Position(object):
    """三维坐标"""
    def __init__(self, x, y, z):
        super(Position, self).__init__()
        self.x = x
        self.y = y
        self.z = z

    def __eq__(self, other):
        """判断两Position实例相等"""
        if isinstance(other, Position):
            return (self.x == other.x) and (self.y == other.y) and (self.z == other.z)
        else:
            return False

    def distance(self, target_pos):
        """返回该位置到target_pos的距离"""
        return abs(target_pos.x - self.x) + abs(target_pos.y - self.y)

    def region(self, level, range):     
        """返回距离该位置range以内区域点集list"""
        region_points = []
        for y in xrange(self.y - range, self.y + range + 1):
            for x in xrange(self.x - range, self.x + range + 1):
                if self.distance(Position(x, y, level)) <= range:
                    region_points.append(Position(x, y, level))
        # 在一矩形范围内寻找符合条件的点
        return region_points

    def display(self):
        print '(', self.x, ',', self.y, ',', self.z, ')',

class Rectangle(object):
    """平面矩形区域, 由左上及右下两顶点坐标确定"""
    def __init__(self, upper_left, lower_right):
        super(Rectangle, self).__init__()
        self.upper_left = upper_left
        self.lower_right = lower_right
        self.level = upper_left.z   # 转化为upper_left所在平面
        self.lower_right.z = self.level

    def bound(self):
        """返回矩形区域的边界点集list"""
        bound_points = []
        for x in xrange(self.upper_left.x, self.lower_right.x + 1):
            bound_points.append(Position(x, self.upper_left.y, self.level))
            bound_points.append(Position(x, self.lower_right.y, self.level))
        for y in xrange(self.upper_left.y + 1, self.lower_right.y):
            bound_points.append(Position(self.upper_left.x, y, self.level))
            bound_points.append(Position(self.lower_right.x, y, self.level))
        return bound_points

    def region(self, level, range = 0):
        """返回矩形区域向外延伸range范围的区域点集list"""
        region_points = []
        for y in xrange(self.upper_left.y, self.lower_right.y + 1):
            for x in xrange(self.upper_left.x, self.lower_right.x + 1):
                region_points.append(Position(x, y, level))    # region_points = 矩形区域内所有点构成的list
        if range == 0:
            return region_points
        else:
            for point in self.bound():
                for pos in point.region(level, range):
                    if pos not in region_points:
                        region_points.append(pos)
            return region_points


class UnitBase(object):
    """单位抽象, 派生出建筑类以及可移动单位类"""
    def __init__(self, team, pos, sight_ranges, fire_ranges, health, fuel, ammo, 
                 attacks, defences):
        super(UnitBase, self).__init__()
        self.team = team
        self.pos = pos                      # pos可以是一个点(Position类型), 
                                            # 也可以是矩形(Rectangle类型)
        self.sight_ranges = sight_ranges
        self.fire_ranges = fire_ranges
        self.health = health
        self.fuel = fuel
        self.ammo = ammo
        self.attacks = attacks
        self.defences = defences

    def availableRegion(self, option = 'sight'):
        """返回三维视野区域或可攻击区域(option == 'fire')"""
        if option == 'sight':
            ranges = self.sight_ranges
        elif option == 'fire':
            ranges = self.fire_ranges
        else:
            return -1   # unknown option
        available_region = []
        for z in xrange(3):
            available_region += pos.region(z, ranges[z])
        return available_region

    def view(self, target_pos):
        """查看目标点的状态"""
        if target_pos not in self.availableRegion('sight'):
            return -1   # 不在视野范围内, 不可见
        else:
            pass
            ## 返回单位信息...

    def attack(self, target_pos, attack_type = FIRE):
        """攻击(默认火力攻击)某三维坐标位置"""
        if target_pos not in self.pos.region(target_pos.z, fire_ranges[target_pos.z]):
            return -1   # 不在攻击范围内
        elif self.ammo <= 0:
            return -2   # 无弹药
        else:
            self.ammo -= 1  # 弹药-1
            pass
            ## 计算伤害...

def repairToNew(repairer, broken, new):   # 维修者(基地), 损坏单位, 新单位
    provides = [0, 0, 0]    # 维修者可提供的燃料, 弹药, 金属
    provides[0] = max(repairer.fuel, new.fuel - broken.fuel)
    provides[1] = max(repairer.ammo, new.ammo - broken.ammo)
    provides[2] = max(repairer.metal, (new.health - broken.health) * METAL_PER_HEALTH)
    repairer.fuel -= provides[0]
    repairer.ammo -= provides[1]
    repairer.metal -= provides[2]
    broken.fuel += provides[0]



class Base(UnitBase):
    """建筑类, 继承自UnitBase"""
    def __init__(self, team, rectangle, metal):
        super(Base, self).__init__(team, rectangle, 
                                   *(BUILDINGS[BASE][:5] + BUILDINGS[BASE][-2:]))
                                   # 从元组解析出数据后传入UnitBase.__init__()
        self.kind = 'BASE'  # 以字符串储存单位种类
        self.metal = BUILDINGS[BASE][6]

    def supply(self, ):
        pass

    def repair(self, our_unit, plane_nums = [3, 3, 3, 1]):  # 提供默认编队配置
        """维修"""
        if not self.team == our_unit.team:
            return -1   # 非友军
        elif our_unit.kind == 'FORMATION':  
            if not our_unit.pos in self.pos.region(level = AIR):   # range = 0
                return -2   # 不在范围内
            else:
                new_formation = Formation(...)      # 维修后理想状态
                repairToNew(self, our_unit, new_formation)
