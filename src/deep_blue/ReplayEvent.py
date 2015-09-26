#!/usr/bin/env python
# -*- coding: UTF-8 -*-
#回放定义

from lib.PaintEvent import *
import sys, time, copy
from deep_blue import *
import qrc_source

class ReplayThread(QThread):
	def __init__(self, scene, parent = None):
		super(ReplayThread, self).__init__(parent)
		
		self.started = False
		self.froglist = []
		self.used = 0
		self.scene = scene

	def Initialize(self, index, fileinfo, MapX, MapY):
		self.resetFrog()
		width = 25
		height = 25
		vision_list = fileinfo.vision(index)
		for k in range(3):
			for i in range(MapX, width + MapX):
				for j in range(MapY, height + MapY):
					if basic.Position(i, j, k) not in vision_list[k]:
						new_frog = self.froglist[self.used]
						self.scene.addItem(new_frog)
						new_frog.setPos(i - MapX, j - MapY, k)
						self.used += 1

	def run(self):
		self.started = True
		for i in range(10000):
			self.froglist.append(FrogUnit(0,0,0))

	def resetFrog(self):
		for i in range(self.used):
			if self.froglist[i].scene() == self.scene:
				self.scene.removeItem(self.froglist[i])
		self.used = 0


class Replay(QGraphicsView):
	endGame = pyqtSignal()
	Unselected = pyqtSignal()
	EscPressed = pyqtSignal()
	APressed = pyqtSignal()
	MPressed = pyqtSignal()
	FPressed = pyqtSignal()
	SPressed = pyqtSignal()
	moveAnimEnd = pyqtSignal()
	unitSelected = pyqtSignal()
	mapSelected = pyqtSignal()
	Scaled = pyqtSignal()
	def __init__(self, scene, parent = None):
		super(Replay, self).__init__(parent)

		self.scene = scene
		self.setScene(self.scene)
		self.setFixedSize(QSize(750,750))

		self.mouseUnit = MouseIndUnit(0, 0, 0, 1)
		self.mouseUnit.setVisible(False)
		self.scene.addItem(self.mouseUnit)

		self.setVerticalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
		self.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOff)

		self.frogIndex = -1
		self.key_pressed = 0
		self.changed = False
		self.battle = None
		self.now_state = None
		self.animation = None
		self.run = False
		self.HUMAN_REPLAY = -1 #4代表ai对战，0代表左人右机，1相反，2代表全人，3代表回放
		self.TIME_PER_STEP = 100
		self.nowRound = 0
		self.MapInfo = None
		self.moveani = None
		self.scale_num = 1.0

		self.map_size = 0
		self.height = 0
		self.width = 0
		self.SelectedIndex = None

		self.unitIndexList = [{},{},{}]
		self.UnitBase = [[],[],[]]
		self.MapList = []
		self.FrogList = []
		self.CreateList = []
		self.MoveList = []
		self.animationItem = []
		self.command_list = []
		self.attack_list = {}

		#状态机
		'''
		self.stateMachine = QStateMachine(self)
		self.State_Run = QState(self.stateMachine)
		self.State_Selected = QState(self.State_Run)
		self.State_Nothing = QState(self.State_Selected)
		self.State_A_Pressed = QState(self.State_Selected)
		self.State_F_Pressed = QState(self.State_Selected)
		self.State_M_Pressed = QState(self.State_Selected)
		self.State_S_Pressed = QState(self.State_Selected)
		self.State_Unselected = QState(self.State_Run)
		self.stateMachine.setInitialState(self.State_Run)
		self.State_Run.setInitialState(self.State_Unselected)
		self.State_Selected.setInitialState(self.State_Nothing)
		self.State_Final = QFinalState(self.stateMachine)

		self.statelist = [self.State_Unselected, self.State_Selected, self.State_A_Pressed, self.State_F_Pressed, self.State_M_Pressed, self.State_S_Pressed]

		self.State_Unselected.addTransition(self, SIGNAL("unitSelected()"), self.State_Selected)
		self.State_Selected.addTransition(self, SIGNAL("mapSelected()"), self.State_Unselected)
		self.State_Run.addTransition(self, SIGNAL("endGame()"), self.State_Final)
		self.State_Selected.addTransition(self, SIGNAL("Unselected()"), self.State_Unselected)
		self.State_Nothing.addTransition(self, SIGNAL("APressed()"), self.State_A_Pressed)
		self.State_Nothing.addTransition(self, SIGNAL("FPressed()"), self.State_F_Pressed)
		self.State_Nothing.addTransition(self, SIGNAL("MPressed()"), self.State_M_Pressed)
		self.State_Nothing.addTransition(self, SIGNAL("SPressed()"), self.State_S_Pressed)
		self.State_A_Pressed.addTransition(self, SIGNAL("EscPressed()"), self.State_Nothing)
		self.State_F_Pressed.addTransition(self, SIGNAL("EscPressed()"), self.State_Nothing)
		self.State_M_Pressed.addTransition(self, SIGNAL("EscPressed()"), self.State_Nothing)
		self.State_S_Pressed.addTransition(self, SIGNAL("EscPressed()"), self.State_Nothing)

		for state in self.statelist:
			self.connect(state, SIGNAL("entered()"), self.on_Entered)
		'''

	'''
	人机已经实现的功能:
		左键单击选中单位：
			键盘点击A，左键单击敌方单位————攻击单位
			键盘点击A，左键单击地图元素————攻击地点
			键盘点击M                  ————移动
			键盘点击F，左键点击本方单位————维修
			键盘点击S，左键点击本方单位————补给
			右键单击敌方单位           ————攻击单位
			右键单击地图元素           ————攻击地点
			右键单击本方船只、飞机     ————维修
			右键单击本方基地、据点     ————补给
		Esc键：
			如果左键选中有单位且键盘有指令————取消键盘命令
			如果左键选中有单位且键盘无指令————取消选中
	'''

	def mousePressEvent(self, event):
		if not self.run:
			QGraphicsView.mousePressEvent(self, event)
			return

		if event.button() == Qt.LeftButton:
			pos = event.pos()
			items = self.items(pos)
			focus = None
			temp = None
			if not items:
				return
			if self.key_pressed == 0:
				for it in items:
					if isinstance(it, SoldierUnit):
						if not temp:
							temp = it.obj
							self.emit(SIGNAL("unitSelected"),it.obj,self.HUMAN_REPLAY,self.battle)
							self.SelectedIndex = it.obj #此变量的作用是识别被选定单位的队伍，以判定是否可以发出攻击指令，待修改
							focus = it
						else:
							if it.obj.kind:
								self.emit(SIGNAL("unitSelected"),it.obj,self.HUMAN_REPLAY,self.battle)
								self.SelectedIndex = it.obj #此变量的作用是识别被选定单位的队伍，以判定是否可以发出攻击指令，待修改
								focus = it
						break
					elif isinstance(it, MapUnit):
						self.emit(SIGNAL("mapSelected"), it.obj, it.corX, it.corY)
				if focus:
					if self.moveani:
						self.moveani.stop()
						self.moveani.deleteLater()
						self.moveani = None
					if not self.mouseUnit.isVisible():
						self.mouseUnit.setVisible(True)
					self.mouseUnit.size = focus.obj.size[0]
					self.mouseUnit.setPos(focus.corX, focus.corY, focus.corZ)
				else:
					self.SelectedIndex = None
			#点击键盘后的左键指令只判断接收方是否符合要求，对发起方的要求在keyPressEvent中判断
			elif self.key_pressed == 1:
				flag1 = False
				flag2 = False
				pos = None
				for it in items:
					if isinstance(it, SoldierUnit) and it.obj.team != self.SelectedIndex.team:
						#self.emit(SIGNAL("attackTargetSelected"), it.obj)
						new_command = command.AttackUnit(self.SelectedIndex.index, it.obj.index)
						self.command_list.append(new_command)
						self.attack_list[self.SelectedIndex.index] = new_command
						self.key_pressed = 0
						flag1 = True
					if isinstance(it, FrogUnit):
						flag2 = True
						pos = it.pos
						#self.emit(SIGNAL("attackPosSelected"), it.pos)
					if flag2 and not flag1:
						new_command = command.AttackPos(self.SelectedIndex.index, basic.Position(pos[0], pos[1], pos[2]))
						self.command_list.append(new_command)
						self.attack_list[self.SelectedIndex.index] = new_command
				self.key_pressed = 0
				cursor = QCursor(QPixmap(":cursor.png").scaled(30,30), 0, 0)
				self.setCursor(cursor)
			elif self.key_pressed == 2:
				for it in items:
					if isinstance(it, SoldierUnit) and it.obj.team == self.SelectedIndex.team and it.obj.kind in [4, 5, 6, 7, 8, 9]:
						#self.emit(SIGNAL("fixTargetSelected"), it.obj)
						new_command = command.Fix(self.SelectedIndex.index, it.obj.index)
						if self.SelectedIndex.index in self.attack_list.keys():
							del self.attack_list[self.SelectedIndex.index]
						self.command_list.append(new_command)
				self.key_pressed = 0
				cursor = QCursor(QPixmap(":cursor.png").scaled(30,30), 0, 0)
				self.setCursor(cursor)
			elif self.key_pressed == 3:
				for it in items:
					if isinstance(it, MapUnit):
						new_command = command.ChangeDest(self.SelectedIndex.index, basic.Position(it.pos[0], it.pos[1], it.pos[2]))
						if self.SelectedIndex.index in self.attack_list.keys():
							del self.attack_list[self.SelectedIndex.index]
						self.command_list.append(new_command)
				self.key_pressed = 0
				cursor = QCursor(QPixmap(":cursor.png").scaled(30,30), 0, 0)
				self.setCursor(cursor)
			elif self.key_pressed == 4:
				for it in items:
					if isinstance(it, SoldierUnit) and it.obj.team == self.SelectedIndex.team: #认为除了矿井和和油田外的对象均能被补给
						#self.emit(SINGAL("supplyTargetSelected"), it.obj)
						new_command = command.Supply(self.SelectedIndex.index, it.obj.index)
						if self.SelectedIndex.index in self.attack_list.keys():
							del self.attack_list[self.SelectedIndex.index]
						self.command_list.append(new_command)
				self.key_pressed = 0
				cursor = QCursor(QPixmap(":cursor.png").scaled(30,30), 0, 0)
				self.setCursor(cursor)
			
		

		elif event.button() == Qt.RightButton:
			if not self.HUMAN_REPLAY in [0, 1, 2]:
				return
			if not self.SelectedIndex:
				return
			if self.HUMAN_REPLAY in [0, 1]:
				if self.SelectedIndex.team != self.HUMAN_REPLAY:
					return
			pos = event.pos()
			items = self.items(pos)
			if not items:
				return
			maprecord = False
			get_unit = False
			for it in items:
				if isinstance(it, SoldierUnit) and it.obj.team != self.SelectedIndex.team:  #如果点击对象是单位，并且点击对象和被选中的单位不属于同一队伍，则攻击
					get_unit = True
					new_command = command.AttackUnit(self.SelectedIndex.index, it.obj.index)
					self.command_list.append(new_command)
					self.attack_list[self.SelectedIndex.index] = new_command
				elif isinstance(it, SoldierUnit) and it.obj.team == self.SelectedIndex.team:
					if it.obj.kind in [4, 5, 6, 7, 8, 9] and self.SelectedIndex.kind == 0:
						get_unit = True
						#如果之前选中了基地，再右击可被维修的单位，则发出维修指令
						new_command = command.Fix(self.SelectedIndex.index, it.obj.index)
						if self.SelectedIndex.index in self.attack_list.keys():
							del self.attack_list[self.SelectedIndex.index]
						self.command_list.append(new_command)
					elif it.obj.kind in [0, 1, 4, 5, 6, 7, 8, 9] and self.SelectedIndex.kind in [1,6,7]:
						get_unit = True
						#如果之前选中航母或运输舰，在右击基地或据点，则补给
						new_command = command.Supply(self.SelectedIndex.index, it.obj.index)
						if self.SelectedIndex.index in self.attack_list.keys():
							del self.attack_list[self.SelectedIndex.index]
						self.command_list.append(new_command)
				#由于修理和补给指令的双方种类可能完全相同，所以只定义了右键点击可以修理以及单位向建筑物补给，对单位补给指令只能通过"s+左键"发出
				elif isinstance(it, MapUnit):
					maprecord = True
			if not get_unit and maprecord:
				if self.SelectedIndex.kind == 0 or self.SelectedIndex.kind == 1:
						new_command = command.AttackPos(self.SelectedIndex.index, basic.Position(it.pos[0], it.pos[1], it.pos[2]))
						self.command_list.append(new_command)
						self.attack_list[self.SelectedIndex.index] = new_command
				elif self.SelectedIndex.kind >= 4:
					new_command = command.ChangeDest(self.SelectedIndex.index, basic.Position(it.pos[0], it.pos[1], it.pos[2]))
					if self.SelectedIndex.index in self.attack_list.keys():
						del self.attack_list[self.SelectedIndex.index]
					self.command_list.append(new_command)
	'''
	def mouseMoveEvent(self, event):
		if not self.run:
			QGraphicsView.mouseMoveEvent(self, event)
			return

		if not self.mouseUnit.isVisible():
			self.mouseUnit.setVisible(True)

		pos = event.pos()
		items = self.items(pos)
		if not items:
			return
		if self.now_state == self.State_A_Pressed:
			for it in items:
				if isinstance(it, SoldierUnit):
					"""修改指针样式"""
					return
	'''

	def wheelEvent(self, event):
		if self.run:
		    factor = 1.414213562373 ** (event.delta() / 240.0)
		    self.scale(factor, factor)
		    self.scale_num *= factor
		    self.emit(SIGNAL("Scaled"), factor)

	def keyPressEvent(self, event):
		if not self.run:
			QGraphicsView.keyPressEvent(self, event)
			return

		key_list = [Qt.Key_1, Qt.Key_2, Qt.Key_3, Qt.Key_4, Qt.Key_5, Qt.Key_6]

		if event.key() in key_list:
			new_command = command.Produce(event.key() - Qt.Key_1 + 4)
			self.command_list.append(new_command)
			return

		if not self.SelectedIndex or self.SelectedIndex.kind in [2, 3]:
			return

		if event.key() == Qt.Key_Escape:
			'''
			if self.now_state in [self.State_A_Pressed, self.State_C_Pressed, self.State_F_Pressed, self.State_S_Pressed]:
				self.EscPressed.emit()
			elif self.now_state == self.State_Selected:
				self.Unselected.emit()
			else:
				return
			'''
			if self.key_pressed:
				self.key_pressed = 0
			elif self.SelectedIndex:
				self.SelectedIndex = None
			else:
				return

		elif event.key() == Qt.Key_A:
			if not self.HUMAN_REPLAY in [0, 1, 2]: 
				return
			if not self.SelectedIndex:
				return
			if self.HUMAN_REPLAY in [0, 1] and self.SelectedIndex.team != self.HUMAN_REPLAY: 
				return
			'''
			if self.now_state == self.State_Unselected:
				return
			'''
			if self.SelectedIndex.kind in [0, 1, 4, 5, 6, 8, 9]:          #不知道能不能实现，不能的话交由平台管？
				cursor = QCursor(QPixmap(":cursor2.png").scaled(30,30), 0, 0)
				self.setCursor(cursor)
				self.key_pressed = 1
			else:
				return

		elif event.key() == Qt.Key_F:
			if not self.HUMAN_REPLAY in [0, 1, 2]:
				return
			if not self.SelectedIndex:
				return
			if self.HUMAN_REPLAY in [0, 1] and self.SelectedIndex.team != self.HUMAN_REPLAY:
				return
			if self.SelectedIndex.kind == 0:
				cursor = QCursor(QPixmap(":cursor2.png").scaled(30,30), 0, 0)
				self.setCursor(cursor)    
				self.key_pressed = 2
			else:
				return

		elif event.key() == Qt.Key_M:
			if not self.HUMAN_REPLAY in [0, 1, 2]:
				return
			if not self.SelectedIndex:
				return
			if self.HUMAN_REPLAY in [0, 1] and self.SelectedIndex.team != self.HUMAN_REPLAY:
				return
			if self.SelectedIndex.kind >= 4:
				cursor = QCursor(QPixmap(":cursor2.png").scaled(30,30), 0, 0)
				self.setCursor(cursor)
				print
				print
				print
				print
				print "we set cursor"
				print
				print
				self.key_pressed = 3
			else:
				return

		elif event.key() == Qt.Key_S:
			if not self.HUMAN_REPLAY in [0, 1, 2]:
				return
			if not self.SelectedIndex:
				return
			if self.HUMAN_REPLAY in [0, 1] and self.SelectedIndex.team != self.HUMAN_REPLAY:
				return
			if self.SelectedIndex.kind in [0, 1, 6, 7]:
				cursor = QCursor(QPixmap(":cursor2.png").scaled(30,30), 0, 0)
				print
				print
				print
				print
				print "we set cursor"
				print
				print
				self.setCursor(cursor)
				self.key_pressed = 4
			else:
				return

		else:
			return
	
	'''
	def on_Entered(self):
		now_state = self.sender()
		if not isinstance(now_state, QState):
			return
		self.now_state = now_state
	'''

	def resetMap(self):
		for item in self.MapList:
			if item.scene() == self.scene:
				self.scene.removeItem(item)
				item.deleteLater()
		self.MapList = []

	def setMapArea(self, area):
		self.centerOn(QPointF((area.x()*30 + (area.x() + 25)*30)/2, (area.y()*30 + (area.y() + 25)*30)/2))

	def setMap(self):
		self.resetMap()
		for i in range(self.MapInfo.x_max):
			for j in range(self.MapInfo.y_max):
				new_map = MapUnit(i, j, 2, self.Map_Info[i][j])
				self.scene.addItem(new_map)
				new_map.setPos(i, j, 2)
				self.MapList.append(new_map)

	def resetFrog(self):
		for item in self.FrogList:
			if item.scene() == self.scene:
				self.scene.removeItem(item)
				item.deleteLater()
		self.FrogList = []
		#self.setUnit()

	def setFrog(self, index):
		self.resetFrog()
		frog = self.battle.vision(index)
		for k in range(3):
			for i in range(self.MapInfo.x_max):
				for j in range(self.MapInfo.y_max):
					if basic.Position(i, j, k) not in frog[k]:
						new_frog = FrogUnit(i, j, k)
						self.scene.addItem(new_frog)
						new_frog.setPos(i, j, k)
						self.FrogList.append(new_frog)

	def resetUnit(self):
		for item in self.UnitBase[0]:
			if item.scene() == self.scene:
				self.scene.removeItem(item)
				item.deleteLater()
		for item in self.UnitBase[1]:
			if item.scene() == self.scene:
				self.scene.removeItem(item)
				item.deleteLater()
		for item in self.UnitBase[2]:
			if item.scene() == self.scene:
				self.scene.removeItem(item)
				item.deleteLater()
		self.UnitBase = [[],[],[]]

	def setUnit(self):
		self.resetUnit()
		for i in range(3):
			for j in self.Unit_Info[i].keys():
				new_unit = SoldierUnit(self.Unit_Info[i][j])
				self.UnitBase[i].append(new_unit)
				self.scene.addItem(new_unit)
				if self.Unit_Info[i][j].kind:
					new_unit.setPos(self.Unit_Info[i][j].position.x, self.Unit_Info[i][j].position.y, self.Unit_Info[i][j].position.z)
				else:
					new_unit.setPos(self.Unit_Info[i][j].position.x - 1, self.Unit_Info[i][j].position.y - 1, self.Unit_Info[i][j].position.z)

	def Initialize(self, battle):
		self.MapInfo = battle.map_info()
		self.map_size = self.MapInfo.x_max
		Elements = [battle.elements(0), battle.elements(1), battle.elements(2)]
		self.Unit_Info = [{},{},{}]
		for i in range(3):
			for element in Elements[i].values():
				self.unitIndexList[i][element.index] = element.position
				self.Unit_Info[i][element.position] = element
		for element in self.MapInfo.elements.values():
			if isinstance(element, basic.UnitBase):
				pass
			else:
				self.unitIndexList[2][element.index] = element.position
				self.Unit_Info[2][element.position] = element
		self.Map_Info = [[0 for y in range(self.MapInfo.y_max)] for x in range(self.MapInfo.x_max)]
		for i in range(self.MapInfo.x_max):
			for j in range(self.MapInfo.y_max):
				self.Map_Info[i][j] = self.MapInfo.map_type(i, j)
		rect = QRectF(0,0,self.MapInfo.x_max*30, self.MapInfo.y_max*30)
		self.setSceneRect(rect)
		self.setMap()
		self.setUnit()
		self.run = True

		'''
		if not self.stateMachine.isRunning():
			print "try to open stateMachine, now it is ", self.stateMachine.isRunning()
			self.stateMachine.start()
			print "after open, it is", self.stateMachine.isRunning()
		print "Initialize! stateMachine running? ", self.stateMachine.isRunning() 
		'''

	def attackAnimation(self, move_unit_index, attack_target_index, damage):
		ATTACK_TIME = 20 * self.TIME_PER_STEP
		TOTAL_TIME = 60 * self.TIME_PER_STEP
		move_unit = None
		attack_target = None
		for i in range(2):
			for soldier in self.UnitBase[i]:
				if soldier.obj.index == move_unit_index:
					move_unit = soldier
					break			
		for i in range(3):
			for soldier in self.UnitBase[i]:
				if soldier.obj.index == attack_target_index:
					attack_target = soldier
					break
		if not move_unit or not attack_target:
			return None, []
		showAtkAnim = QParallelAnimationGroup()
		move_effect = AttackEffectUnit(move_unit.corX, move_unit.corY, move_unit.corZ)
		move_effect.setPos(move_unit.corX, move_unit.corY, move_unit.corZ)
		self.scene.addItem(move_effect)

#炮弹飞行效果
		ani = QPropertyAnimation(move_effect, "pos")
		ani.setDuration(ATTACK_TIME)
		if move_unit.obj.kind:
			ani.setStartValue(QPointF((move_unit.corX)* 30 + 15, (move_unit.corY)* 30 + 5 + 10 * (2 - move_unit.corZ)))
		else:
			ani.setStartValue(QPointF((move_unit.corX + 1)* 30 + 15, (move_unit.corY + 1)* 30 + 5 + 10 * (2 - move_unit.corZ)))
		if isinstance(attack_target, basic.Position):
			ani.setEndValue(QPointF((attack_target.x)* 30 + 15, (attack_target.y)* 30 + 5 + 10 * (2 - attack_target.z)))
		else:
			if attack_target.obj.kind:
				ani.setEndValue(QPointF((attack_target.corX)* 30 + 15, (attack_target.corY)* 30 + 5 + 10 * (2 - attack_target.corZ)))
			else:
				ani.setEndValue(QPointF((attack_target.corX + 1)* 30 + 15, (attack_target.corY + 1)* 30 + 5 + 10 * (2 - attack_target.corZ)))
		showAtkAnim.addAnimation(ani)


#炮弹渐隐渐显
		ani = QPropertyAnimation(move_effect, "opacity")
		ani.setDuration(ATTACK_TIME)
		ani.setStartValue(0)
		ani.setKeyValueAt(0.05,1)
		ani.setKeyValueAt(0.95,1)
		ani.setEndValue(0)
		showAtkAnim.addAnimation(ani)

#攻击效果提示
		label = EffectIndUnit("-%d" %(damage))
		label.setOpacity(0)
		label.setScale(0.8)
		self.scene.addItem(label)
		if isinstance(attack_target,basic.Position):
			label.setPos(QPointF((attack_target.x)* 30, (attack_target.y)* 30 + 5 + 10 * (2 - attack_target.z)) + QPointF(0, -10))
		else:
			if attack_target.obj.kind:
				label.setPos(QPointF((attack_target.corX)* 30, (attack_target.corY)* 30 + 5 + 10 * (2 - attack_target.corZ)) + QPointF(0, -10))
			else:
				label.setPos(QPointF((attack_target.corX + 1)* 30, (attack_target.corY + 1)* 30 + 5 + 10 * (2 - attack_target.corZ)) + QPointF(0, -10))
		ani = QPropertyAnimation(label, "opacity")
		ani.setDuration(TOTAL_TIME)
		ani.setStartValue(0)
		ani.setKeyValueAt(0.375, 0)
		ani.setKeyValueAt(0.4, 1)
		ani.setKeyValueAt(0.5, 0)
		ani.setEndValue(0)
		showAtkAnim.addAnimation(ani)

		if isinstance(attack_target, basic.Position):
			boom = BoomEffectUnit(attack_target.x, attack_target.y, attack_target.z)
			boom.setPos(attack_target.x, attack_target.y, attack_target.z)
		else:
			if attack_target.obj.kind:
				boom = BoomEffectUnit(attack_target.corX, attack_target.corY, attack_target.corZ)
				boom.setPos(attack_target.corX, attack_target.corY, attack_target.corZ)
			else:
				boom = BoomEffectUnit(attack_target.corX + 1, attack_target.corY + 1, attack_target.corZ)
				boom.setPos(attack_target.corX + 1, attack_target.corY + 1, attack_target.corZ)
		ani = QPropertyAnimation(boom, "opacity")
		ani.setDuration(TOTAL_TIME)
		ani.setStartValue(0)
		ani.setKeyValueAt(0.375, 0)
		ani.setKeyValueAt(0.4, 1)
		ani.setKeyValueAt(0.5, 0)
		ani.setEndValue(0)
		showAtkAnim.addAnimation(ani)
		self.scene.addItem(boom)

		ani = QPropertyAnimation(label, "pos")
		ani.setDuration(TOTAL_TIME)
		pos1 = label.pos()
		ani.setStartValue(pos1)
		ani.setKeyValueAt(0.375, pos1)
		ani.setKeyValueAt(0.5, pos1 + QPointF(0, -15))
		ani.setEndValue(pos1 + QPointF(0, -15))
		ani.setEasingCurve(QEasingCurve.OutCubic)
		showAtkAnim.addAnimation(ani)

		item = [move_effect, label, boom]
		return showAtkAnim, item

	def moveAnimation(self, move_unit_index, nodes, total_distance):		
		moveAnim_mouse = None
		flag2 = False
		flag = False
		for i in range(2):
			for soldier in self.UnitBase[i]:
				if soldier.obj.index == move_unit_index:
					move_effect = soldier
					flag = True
					break
		if not flag:
			return None, None, []
		if self.SelectedIndex:
			if self.SelectedIndex.index == move_unit_index:
				move_mouse_effect = self.mouseUnit
				flag2 = True
		inflection = len(nodes) - 1
		moveAnim = QPropertyAnimation(move_effect, "pos")
		moveAnim.setDuration(self.TIME_PER_STEP * 60)
		moveAnim.setStartValue(QPointF((nodes[0].x)* 30, (nodes[0].y)* 30 + 10 * (2 - nodes[0].z)))
		moveAnim.setKeyValueAt(0.5, QPointF((nodes[0].x)* 30, (nodes[0].y)* 30 + 10 * (2 - nodes[0].z)))
		if flag2:
			moveAnim_mouse = QPropertyAnimation(move_mouse_effect, "pos")
			moveAnim_mouse.setDuration(self.TIME_PER_STEP * 60)
			moveAnim_mouse.setStartValue(QPointF((nodes[0].x)* 30, (nodes[0].y) * 30 + 10 * (2 - nodes[0].z)))
			moveAnim_mouse.setKeyValueAt(0.5, QPointF((nodes[0].x)* 30, (nodes[0].y)* 30 + 10 * (2 - nodes[0].z)))

		move_distance = []
		for i in range(inflection):
			unit_distance = (abs(nodes[i + 1].x - nodes[i].x) + abs(nodes[i + 1].y - nodes[i].y) + abs(nodes[i + 1].z - nodes[i].z))
			move_distance.append(unit_distance)
			#print move_distance[i]

		tmp = 0.5
		for i in range(inflection):
			pos = QPointF((nodes[i + 1].x)* 30, (nodes[i + 1].y)* 30 + 10 *(2 - nodes[i + 1].z))
			tmp = tmp + float(move_distance[i]) / float(total_distance) * 0.5
			if tmp >= 1:
				break
			moveAnim.setKeyValueAt(tmp, pos)
			if flag2:
				moveAnim_mouse.setKeyValueAt(tmp, pos)
		if total_distance:
			moveAnim.setEndValue(QPointF((nodes[-1].x)* 30, (nodes[-1].y)* 30 + 10 *(2 - nodes[-1].z)))
			move_effect.corX = nodes[-1].x
			move_effect.corY = nodes[-1].y
			move_effect.corZ = nodes[-1].z
			if flag2:
				#move_mouse_effect.setPos(nodes[-1].x, nodes[-1].y, nodes[-1].z)
				moveAnim_mouse.setEndValue(QPointF((nodes[-1].x)* 30, (nodes[-1].y)* 30 + 10 *(2 - nodes[-1].z)))
		else:
			moveAnim.setEndValue(QPointF((move_unit.corX)* 30, (move_unit.corY)* 30 + 10 * (2 - move_unit.corZ)))
			if flag2:
				#move_mouse_effect.setPos(nodes[-1].x, nodes[-1].y, nodes[-1].z)
				moveAnim_mouse.setEndValue(QPointF((move_unit.corX)* 30, (move_unit.corY)* 30 + 10 * (2 - move_unit.corZ)))

		return moveAnim, moveAnim_mouse, []

	def supplyAnimation(self, supply_unit_index, supply_target_index, fuel_supply, ammo_supply, metal_supply):
		TOTAL_TIME=60 * self.TIME_PER_STEP
		supply_unit = None
		supply_target = None

		for i in range(2):
			for soldier in self.UnitBase[i]:
				if soldier.obj.index == supply_unit_index:
					supply_unit = soldier
					break			
		for i in range(2):
			for soldier in self.UnitBase[i]:
				if soldier.obj.index == supply_target_index:
					supply_target = soldier
					break
		if not supply_target or not supply_unit:
			return None, []
		
		showSplAnim = QParallelAnimationGroup()
		if supply_unit.obj.kind and supply_target.obj.kind:
			move_unit = SupEffectUnit(supply_unit.corX, supply_unit.corY, supply_unit.corZ, supply_target.corX, supply_target.corY, supply_target.corZ)
		elif supply_unit.obj.kind == 0:
			move_unit = SupEffectUnit(supply_unit.corX + 1, supply_unit.corY + 1, supply_unit.corZ, supply_target.corX, supply_target.corY, supply_target.corZ)
		else:
			move_unit = SupEffectUnit(supply_unit.corX, supply_unit.corY, supply_unit.corZ, supply_target.corX + 1, supply_target.corY + 1, supply_target.corZ)
		#move_unit.setPos(supply_unit.corX, supply_unit.corY, supply_unit.corZ)
		move_unit.setOpacity(0)
		self.scene.addItem(move_unit)
		
		ani = QPropertyAnimation(move_unit, "opacity")
		ani.setDuration(TOTAL_TIME) 
		ani.setStartValue(0)
		ani.setKeyValueAt(0.5,0)
		ani.setKeyValueAt(0.6, 1)
		ani.setKeyValueAt(0.9, 1)
		ani.setEndValue(0)
		showSplAnim.addAnimation(ani)
		
		label = EffectIndUnit("F +%d A +%d M +%d" %(fuel_supply, ammo_supply, metal_supply))
		label.setColor(Qt.green)
		label.setOpacity(0)
		self.scene.addItem(label)
		if supply_target.obj.kind:
			label.setPos(QPointF((supply_target.corX)* 30 - 10, (supply_target.corY)* 30 + (2 - supply_target.corZ) * 10 - 15))
		else:
			label.setPos(QPointF((supply_target.corX + 1)* 30 - 10, (supply_target.corY + 1)* 30 + (2 - supply_target.corZ) * 10 - 15))
		ani = QPropertyAnimation(label, "opacity")
		ani.setDuration(TOTAL_TIME)
		ani.setStartValue(0)
		ani.setKeyValueAt(0.5,0)
		ani.setKeyValueAt(0.6, 0.8)
		ani.setKeyValueAt(0.75, 1)
		ani.setKeyValueAt(0.95, 0.8)
		ani.setEndValue(0)
		showSplAnim.addAnimation(ani)
		
		ani = QPropertyAnimation(label, "pos")
		ani.setDuration(TOTAL_TIME)
		pos1 = label.pos()
		ani.setStartValue(pos1)
		ani.setKeyValueAt(0.5, pos1)
		ani.setEndValue(pos1 + QPointF(0, 20))
		ani.setEasingCurve(QEasingCurve.OutCubic)
		showSplAnim.addAnimation(ani)
		
		item = [move_unit,label]
		return showSplAnim,item

	def fixAnimation(self, fix_unit_index, fix_target_index, metal_consumption, health_increase):
		TOTAL_TIME = 60 * self.TIME_PER_STEP
		fix_target = None
		fix_unit = None
		for i in range(2):
			for soldier in self.UnitBase[i]:
				if soldier.obj.index == fix_unit_index:
					fix_unit = soldier
					break			
		for i in range(2):
			for soldier in self.UnitBase[i]:
				if soldier.obj.index == fix_target_index:
					fix_target = soldier
					break
		if not fix_target or not fix_unit:
			return None, []
		showFixAnim = QParallelAnimationGroup()
		
		#位置可能不对，再调
		effect_unit = FixEffectUnit(fix_target.corX, fix_target.corY, fix_target.corZ)
		effect_unit.setPos(fix_target.corX, fix_target.corY, fix_target.corZ)
		effect_unit.setOpacity(0)
		self.scene.addItem(effect_unit)
		
		ani = QPropertyAnimation(effect_unit, "rotation")
		ani.setDuration(TOTAL_TIME)
		ani.setStartValue(0)
		ani.setKeyValueAt(0.5,0)
		ani.setKeyValueAt(0.65,60)
		ani.setKeyValueAt(0.8, -60)
		ani.setKeyValueAt(0.95,60)
		ani.setEndValue(0)
		showFixAnim.addAnimation(ani)

		ani = QPropertyAnimation(effect_unit, "opacity")
		ani.setDuration(TOTAL_TIME)
		ani.setStartValue(0)
		ani.setKeyValueAt(0.5,0)
		ani.setKeyValueAt(0.6,1)
		ani.setKeyValueAt(0.9,1)
		ani.setKeyValueAt(0.95,0)
		ani.setEndValue(0)
		showFixAnim.addAnimation(ani)


		fix_label = EffectIndUnit("Metal-%d" %(metal_consumption))
		fix_label.setColor(Qt.red)
		fix_label.setOpacity(0)
		self.scene.addItem(fix_label)
		if fix_unit.obj.kind:
			fix_label.setPos(QPointF((fix_unit.corX)* 30, (fix_unit.corY)* 30))
		else:
			fix_label.setPos(QPointF((fix_unit.corX + 1)* 30, (fix_unit.corY + 1)* 30))
		ani = QPropertyAnimation(fix_label, "opacity")
		ani.setDuration(TOTAL_TIME)
		ani.setStartValue(0)
		ani.setKeyValueAt(0.5,0)
		ani.setKeyValueAt(0.6,0.8)
		ani.setKeyValueAt(0.75, 1)
		ani.setKeyValueAt(0.95,0.8)
		ani.setEndValue(0)
		showFixAnim.addAnimation(ani)
		
		ani = QPropertyAnimation(fix_label, "pos")
		ani.setDuration(TOTAL_TIME)
		pos1 = fix_label.pos()
		ani.setStartValue(pos1)
		ani.setKeyValueAt(0.75, pos1)
		ani.setEndValue(pos1 + QPointF(0, -10))
		ani.setEasingCurve(QEasingCurve.OutCubic)
		showFixAnim.addAnimation(ani)
		
		effect_label = EffectIndUnit("Health+%d" %(health_increase))
		effect_label.setDefaultTextColor(Qt.green)
		effect_label.setOpacity = (0)
		self.scene.addItem(effect_label)
		effect_label.setPos(QPointF((fix_target.corX)* 30, (fix_target.corY)* 30 + (2 - fix_target.corZ) * 10 - 10))
		ani = QPropertyAnimation(effect_label, "pos")
		ani.setDuration(TOTAL_TIME)
		ani.setStartValue(effect_label.pos())
		ani.setKeyValueAt(0.2,effect_label.pos())
		ani.setEndValue(effect_label.pos() + QPointF(0, 10))
		showFixAnim.addAnimation(ani)

		ani = QPropertyAnimation(effect_label, "opacity")
		ani.setDuration(TOTAL_TIME)
		ani.setStartValue(0)
		ani.setKeyValueAt(0.5,0)
		ani.setKeyValueAt(0.6,1)
		ani.setKeyValueAt(0.9,1)
		ani.setEndValue(0)

		showFixAnim.addAnimation(ani)
		
		return showFixAnim,[effect_unit,fix_label,effect_label]

	def collectAnimation(self, collect_unit_index, collect_target_index, fuel_collect, metal_collect):
		TOTAL_TIME = 60 * self.TIME_PER_STEP
		collect_target = None
		collect_unit = None
		for i in range(2):
			for soldier in self.UnitBase[i]:
				if soldier.obj.index == collect_unit_index:
					collect_unit = soldier
					break			
		for soldier in self.UnitBase[2]:
			if soldier.obj.index == collect_target_index:
				collect_target = soldier
				break			
		if not collect_target or not collect_unit:
			return None, []
		showColAnim = QParallelAnimationGroup()

		effect_unit = ColEffectUnit(collect_target.corX, collect_target.corY , collect_target.corZ)
		effect_unit.setPos(collect_target.corX, collect_target.corY , collect_target.corZ)
		effect_unit.setOpacity(0)
		self.scene.addItem(effect_unit)
		
		ani = QPropertyAnimation(effect_unit, "opacity")
		ani.setDuration(TOTAL_TIME)
		ani.setStartValue(0)
		ani.setKeyValueAt(0.6,0)
		ani.setKeyValueAt(0.75, 1)
		ani.setKeyValueAt(0.95,0.8)
		ani.setEndValue(0)
		showColAnim.addAnimation(ani)
		
		effect_label = EffectIndUnit("Fuel+%d\n Metal+%d" %(fuel_collect, metal_collect))
		effect_label.setColor(Qt.blue)
		effect_label.setOpacity(0)
		self.scene.addItem(effect_label)
		effect_label.setPos(QPointF((collect_unit.corX)* 30, (collect_unit.corY) * 30))
		ani = QPropertyAnimation(effect_label, "opacity")
		ani.setDuration(TOTAL_TIME)
		ani.setStartValue(0)
		ani.setKeyValueAt(0.6,0)
		ani.setKeyValueAt(0.75, 1)
		ani.setKeyValueAt(0.95,0.8)
		ani.setEndValue(0)
		showColAnim.addAnimation(ani)
		
		return showColAnim,[effect_unit,effect_label]


	def addProductionEntryAnimation(self, team, kind):
		
		TOTAL_TIME = 30 * self.TIME_PER_STEP

		aPE_effect = EffectIndUnit("Team: %d\nKind: %d\n" %(team, kind))
		aPE_effect.setPos(QPointF(300, 300))

		aPEAnim = QPropertyAnimation(aPE_effect, "opacity")
		aPEAnim.setDuration(TOTAL_TIME)
		aPEAnim.setStartValue(0)
		aPEAnim.setKeyValueAt(0.5, 1)
		aPEAnim.setEndValue(0)

	'''生产单位'''
	def createAnimation(self, created_unit_index):
		#要不要画个什么效果
		
		TOTAL_TIME = 60 * self.TIME_PER_STEP
		
		created_unit = self.battle.map_info().elements[created_unit_index]
		created_effect = SoldierUnit(created_unit)
		created_effect.setOpacity(0)
		self.scene.addItem(created_effect)
		self.UnitBase[created_unit.team].append(created_effect)
		created_effect.setPos(created_unit.position.x, created_unit.position.y, created_unit.position.z)			
		createAnim = QPropertyAnimation(created_effect, "opacity")
		createAnim.setDuration(TOTAL_TIME)
		createAnim.setStartValue(0)
		createAnim.setKeyValueAt(0.6, 0)
		createAnim.setEndValue(1)
			
		return createAnim,[]

	def destroyAnimation(self, destroyed_unit_index):
		#要不要画个什么效果
		destroyed_effect = None
		for i in range(2):
			for soldier in self.UnitBase[i]:
				if soldier.obj.index == destroyed_unit_index:
					destroyed_effect = soldier
					break		
		if not destroyed_effect:
			return None, []
		TOTAL_TIME = 60 * self.TIME_PER_STEP
		
		destroyAnim = QPropertyAnimation(destroyed_effect, "opacity")
		destroyAnim.setDuration(TOTAL_TIME)
		destroyAnim.setStartValue(1)
		destroyAnim.setKeyValueAt(0.5, 1)
		destroyAnim.setKeyValueAt(0.75, 0)
		destroyAnim.setEndValue(0)
		self.UnitBase[destroyed_effect.obj.team].remove(destroyed_effect)
		
		return destroyAnim,[]

	def captureAnimation(self, target_unit_index, team):		
		TOTAL_TIME=60 * self.TIME_PER_STEP
		target_unit = None
		for i in range(3):
			for soldier in self.UnitBase[i]:
				if soldier.obj.index == target_unit_index:
					team_ = i
					target_unit = soldier
					break
		if not target_unit:
			return None, []
		self.UnitBase[team_].remove(target_unit)
		self.scene.removeItem(target_unit)
		new_target_unit = self.battle.map_info().elements[target_unit_index]
		capture_effect = SoldierUnit(new_target_unit)
		capture_effect.setOpacity(0)
		capture_effect.setPos(new_target_unit.position.x, new_target_unit.position.y, new_target_unit.position.z)
		self.scene.addItem(capture_effect)
		self.UnitBase[new_target_unit.team].append(capture_effect)
		
		ani = QParallelAnimationGroup()

		label = EffectIndUnit("Team %d has gained possession." %team)
		label.setOpacity(0)
		self.scene.addItem(label)
		label.setPos(QPointF((new_target_unit.position.x)* 30, (new_target_unit.position.y)* 30))
		Wani = QPropertyAnimation(label, "opacity")
		Wani.setDuration(TOTAL_TIME)
		Wani.setStartValue(0)
		Wani.setKeyValueAt(0.5,0)
		Wani.setKeyValueAt(0.55,0.8)
		Wani.setKeyValueAt(0.75, 1)
		Wani.setKeyValueAt(0.95,0.8)
		Wani.setEndValue(0)
		ani.addAnimation(Wani)

		createAnim = QPropertyAnimation(capture_effect, "opacity")
		createAnim.setDuration(TOTAL_TIME)
		createAnim.setStartValue(0)
		createAnim.setKeyValueAt(0.6, 0)
		createAnim.setEndValue(1)
		ani.addAnimation(createAnim)

		
		return ani,[label]

	def attackMissAnimation(self, move_unit_index, attack_target):
		ATTACK_TIME = 20 * self.TIME_PER_STEP
		TOTAL_TIME = 60 * self.TIME_PER_STEP
		ani = None
		move_effect = None
		move_unit = None
		label = None
		for i in range(2):
			for soldier in self.UnitBase[i]:
				if soldier.obj.index == move_unit_index:
					move_unit = soldier
					break
		if not move_unit:
			return None, []
		showAtkAnim = QParallelAnimationGroup()
		move_effect = AttackEffectUnit(move_unit.corX, move_unit.corY, move_unit.corZ)
		move_effect.setPos(move_unit.corX, move_unit.corY, move_unit.corZ)
		self.scene.addItem(move_effect)
		ani = QPropertyAnimation(move_effect, "pos")
		ani.setDuration(ATTACK_TIME)
		ani.setStartValue(QPointF((move_unit.corX)* 30 + 15, ((move_unit.corY)* 30 + 5 + (2 - move_unit.corZ) * 10)))
		ani.setEndValue(QPointF((attack_target.x)* 30 + 15, ((attack_target.y)* 30 + 5 + (2 - attack_target.z) * 10)))
		showAtkAnim.addAnimation(ani)

		ani = QPropertyAnimation(move_effect, "opacity")
		ani.setDuration(ATTACK_TIME)
		ani.setStartValue(0)
		ani.setKeyValueAt(0.3,1)
		ani.setKeyValueAt(0.7,1)
		ani.setEndValue(0)
		showAtkAnim.addAnimation(ani)
		label = EffectIndUnit("Miss")
		label.setColor(Qt.red)
		label.setOpacity(0)
		self.scene.addItem(label)
		label.setPos(QPointF((attack_target.x)* 30, (attack_target.y)* 30 + (2 - attack_target.z) * 10))
		label.setScale(0.8)
		ani = QPropertyAnimation(label, "opacity")
		ani.setDuration(TOTAL_TIME)
		ani.setStartValue(0)
		ani.setKeyValueAt(0.375, 0)
		ani.setKeyValueAt(0.4, 1)
		ani.setKeyValueAt(0.5, 0)
		ani.setEndValue(0)
		showAtkAnim.addAnimation(ani)

		boom = BoomEffectUnit(attack_target.x, attack_target.y, attack_target.z)
		boom.setPos(attack_target.x, attack_target.y, attack_target.z)
		ani = QPropertyAnimation(boom, "opacity")
		ani.setDuration(TOTAL_TIME)
		ani.setStartValue(0)
		ani.setKeyValueAt(0.375, 0)
		ani.setKeyValueAt(0.4, 1)
		ani.setKeyValueAt(0.5, 0)
		ani.setEndValue(0)
		showAtkAnim.addAnimation(ani)
		self.scene.addItem(boom)

		ani = QPropertyAnimation(label, "pos")
		ani.setDuration(TOTAL_TIME)
		pos1 = label.pos()
		ani.setStartValue(pos1)
		ani.setKeyValueAt(0.375, pos1)
		ani.setKeyValueAt(0.5, pos1 + QPointF(0,-20))
		ani.setEndValue(pos1 + QPointF(0,-20))
		ani.setEasingCurve(QEasingCurve.OutCubic)
		showAtkAnim.addAnimation(ani)
		item = [move_effect, label, boom]
		return showAtkAnim, item

	def Play(self, battle):
		eventgroup = None
		self.battle = battle
		if self.HUMAN_REPLAY in [0, 1]:
			for command in self.command_list:
				self.battle.add_command(command)
			self.command_list = []
			for command in self.attack_list.values():
				self.command_list.append(command)
		if self.HUMAN_REPLAY in [0, 1, 4]:
			battle.feed_ai_commands()
		eventgroup = self.battle.next_round()
		#print "len:",len(eventgroup)
		self.nowRound += 1
		self.animation = QParallelAnimationGroup()
		if eventgroup:
			for events in eventgroup:
				if isinstance(events, event.AttackUnit):
					ani, item = self.attackAnimation(events.index, events.target, events.damage)
					if ani:
						self.animation.addAnimation(ani)
					self.animationItem.extend(item)
				if isinstance(events, event.Supply):
					ani, item = self.supplyAnimation(events.index, events.target, events.fuel, events.ammo, events.metal)
					if ani:
						self.animation.addAnimation(ani)
					self.animationItem.extend(item)
				if isinstance(events, event.Fix):
					ani, item = self.fixAnimation(events.index, events.target, events.metal, events.health_increase)
					if ani:
						self.animation.addAnimation(ani)
					self.animationItem.extend(item)
				if isinstance(events, event.Collect):
					ani, item = self.collectAnimation(events.index, events.target, events.fuel, events.metal)
					if ani:
						self.animation.addAnimation(ani)
					self.animationItem.extend(item)
				if isinstance(events, event.Move):
					ani, ani2, item = self.moveAnimation(events.index, events.nodes, events.steps)
					if ani:
						self.animation.addAnimation(ani)
					if ani2:
						self.moveani = ani2
						self.animation.addAnimation(ani2)
					self.MoveList.extend(item)
				if isinstance(events, event.Create):
					ani, item = self.createAnimation(events.index)
					if ani:
						self.animation.addAnimation(ani)
					self.CreateList.extend(item)
				if isinstance(events, event.Destroy):
					ani, item = self.destroyAnimation(events.index)
					if ani:
						self.animation.addAnimation(ani)
					self.animationItem.extend(item)
				if isinstance(events, event.Capture):
					ani, item = self.captureAnimation(events.index, events.team)
					if ani:
						self.animation.addAnimation(ani)
					self.animationItem.extend(item)
				if isinstance(events, event.AttackMiss):
					ani, item = self.attackMissAnimation(events.index, events.target_pos)
					if ani:
						self.animation.addAnimation(ani)
					self.animationItem.extend(item)
		self.animation.addAnimation(QPauseAnimation(30))
		self.connect(self.animation, SIGNAL("finished()"), self.moveAnimEnd)
		#self.connect(self.animation, SIGNAL("finished()"), self.animation, SLOT("deleteLater()"))
		self.animation.start()
		eventgroup = None
		if self.frogIndex == 0:
			self.setFrog(0)
		elif self.frogIndex == 1:
			self.setFrog(1)
		#self.moveAnimEnd.emit()
			
	def TerminateAni(self):
		if self.animation:
			self.animation.stop()
			self.animation.deleteLater()
			self.animation = None
		for item in self.animationItem:
			if item.scene() == self.scene:
				self.scene.removeItem(item)
				item.deleteLater()
		for it in self.CreateList:
			if it.scene() == self.scene:
				self.scene.removeItem(it)
				it.deleteLater()
		for it in self.MoveList:
			if it.scene() == self.scene:
				self.scene.removeItem(it)
				it.deleteLater()
		self.animationItem = []
		self.MoveList = []
		self.CreateList = []
		self.moveani = None

	def reset(self):
		self.TerminateAni()
		self.resetFrog()
		self.resetUnit()
		self.resetMap()
		self.emit(SIGNAL("endGame()"))
		self.frogIndex = -1
		self.key_pressed = 0
		self.changed = False
		self.battle = None
		self.now_state = None
		self.animation = None
		self.run = False
		self.HUMAN_REPLAY = -1 #4代表ai对战，0代表左人右机，1相反，2代表全人，3代表回放
		self.TIME_PER_STEP = 100
		self.nowRound = 0
		self.MapInfo = None
		self.moveani = None

		self.map_size = 0
		self.height = 0
		self.width = 0
		self.scale_num = 1.0
		self.SelectedIndex = None
		self.mouseUnit.setVisible(False)
		self.unitIndexList = [{},{},{}]
		self.UnitBase = [[],[],[]]
		self.MapList = []
		self.FrogList = []
		self.CreateList = []
		self.MoveList = []
		self.animationItem = []
		self.command_list = []
		self.attack_list = {}
