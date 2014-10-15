#include "basic.h"
#include<iostream>
#include<cmath>
#include<vector>
#include<iterator>
#include<algorithm>
#include<functional>

// Remove this line if you konw C++, and don't want a dirty namespace
using namespace teamstyle16;

const char * GetTeamName()
{
    return "��ƿ����";  // Name of your team
}

const GameInfo *INFO = Info();
int enemy_num,DISTANCE;
State * enemy_element;
int (*command)[2]= new int[INFO->element_num][2];
double parameter[10];
int cargo_index[2];  //�����������ص�     ��ʼ��Ϊ-2��-1��ʾ������
int resource_defender[2];  //������Դ��Ǳͧ����ʼ��Ϊ-2��-1��ʾ������
int base_defender[2];      //���ػ��ص�Ǳͧ����ʼ��Ϊ -2��-1��ʾ������
Position origin_target[5];  //����·����+Ǳͧ+�ɻ�*2+����,����ɻ�*2+����  ��Ŀ�ĵ�
int produce_state[2];   //��һ����ʾ·����·�����ϣ����£��ϣ��·ֱ�Ϊ0-4
						//�ڶ�����ʾ�������������˳��ΪǱͧ�����𽢣��ɻ����ɻ�������
						//��ʼ��Ϊ-1��0
int unit_union[4];   //��������������Ǳͧ�����𽢣��ɻ������������ļ��ţ���ʼ��Ϊ0 �����ڷɻ�����ʼ��Ϊ1����Ϊ-1����Ϊ2,��Ϊ-2��

enum CommandType
{
	//Produce
	PRODUCE,
	//Suppply
	SUPPLYFORT,
	SUPPLYBASE,
	SUPPLYUNIT,
	//Attack
	ATTACKUNIT,
	ATTACKFORT,
	ATTACKBASE,
	//Fix
    FIX,
	//ChangeDest
	FORWARD,
	CARGOSUPPLY,
	RETURNBASE,
	//Cancel
	CANCEL,
    kCommandTypes
};
void AIMain()
{  
init();
for(int i=0;i<INFO->element_num;i++)
	if(INFO->elements[i].team == INFO->team_num)
	{
		State Element = INFO->elements[i];
		Supply_Repair(i); //�ò�����ά�޾�ȥ������ά��
		Difference(i);    //�жϸճ��ֵĵ�λ�Ĵ���
		Attack(i);        //����
		if(Element.type == BASE)
			BaseAct();     //����ά�޼�����
		else if(Element.type == CARGO)
			Cargo_Supply(i);   //���佢����
		else ;
		Forward(i);       //ǰ��
	}
MoveCargo();      //���佢�˶�
delete []enemy_element;
}

//��ʼ������������ĺ���
void init()
{
	for(int i = 0; i<INFO->element_num; i++)
		for(int j=0;j<2;j++)
			command[i][j] = -1;           //дһ����ʼ���ĺ���
	parameter[0] = 0.1; parameter[0] = 0.3; parameter[0] = 0.3; parameter[0] = 0.2; parameter[0] = 0.5; 
	if(INFO->round == 1)
	{
		cargo_index[0] = -2; cargo_index[1] = -2;
		resource_defender[0] = -2; resource_defender[1] = -2;
		base_defender[0] = -2; base_defender[1] = -2;
		produce_state[0] = -1; produce_state[1] = 0;
		unit_union[0] = 0; unit_union[1] = 0; unit_union[2] = 0; unit_union[3] = 0;
		for(int i=0;i<5;i++)
		{	
			origin_target[0].x = 0;
			origin_target[0].x = 0;
			origin_target[0].x = 0;
		}

		//�з���λ����
		enemy_num =0;
		for(int i=0;i<Info()->element_num;i++)
			if(Info()->elements[i].team != Info()->team_num)
				enemy_num += 1;
		enemy_element = new State[enemy_num];
		for(int i=0;i<Info()->element_num;i++)
			if(Info()->elements[i].team == 1-Info()->team_num ||
				(Info()->elements[i].type == FORT && Info()->elements[i].team != Info()->team_num))   //�����ݵ�
				enemy_element[i] = Info()->elements[i];
		
	}
}

/////�ò���ȼ�ϻ��ߵ�ҩ��ʱ�򣬻���Ѫ��̫����Ҫ�ػ���ά��
void Supply_Repair(int i)
{
	State Element = INFO->elements[i];
	if(Element.health < parameter[0] * kElementInfos[Element.type].health_max && if_command(i, RETURNBASE))
	{
		ChangeDest(Element.index, INFO->elements[GetBase(INFO->team_num)].pos);
		command[i][1] = max(command[i][2], RETURNBASE);
	}
	else if((Element.ammo <  parameter[1]*kElementInfos[Element.type].ammo_max ||   //��������
		Element.fuel < parameter[2]*kElementInfos[Element.type].fuel_max)          //ȼ�ϲ���
		&& if_command(i, CARGOSUPPLY))                                           //if_command
	    {
			int cargo_index = GetNear(Element.pos,CARGO);
			ChangeDest(INFO->elements[cargo_index].index, Element.pos);
			command[i][1] = CARGOSUPPLY;
		}
	else;
}

//����pos1��pos2ǡ�����fire_range��һ��λ��
Position minus(Position pos1,Position pos2,int fire_range)
{
	Position target;
	target.x = pos1.x > pos2.x ? pos1.x+(fire_range - distance(pos1,pos2))/2 : pos1.x -(fire_range - distance(pos1,pos2))/2;
	target.y = pos1.y > pos2.y ? pos1.y+((fire_range - distance(pos1,pos2))+1)/2 : pos1.y -((fire_range - distance(pos1,pos2))+1)/2;
	target.z = pos1.z;
	return target;

}

//���㹥�����˺�
int damage(State victim,State attacker)
{
	int fire_range = kElementInfos[attacker.type].fire_ranges[kElementInfos[victim.type].level];
	double coefficient = 1 - (distance(victim.pos,attacker.pos) - fire_range / 2) / (fire_range + 1) ;
	return (int)((kElementInfos[attacker.type].attacks[0] + kElementInfos[attacker.type].attacks[1])*coefficient 
		- (kElementInfos[attacker.type].defences[0] + kElementInfos[attacker.type].defences[1]));
}

//�ж�ĳ�����Ƿ���������
int if_in(int i,int *a,int len)
{
	for(int j=0;j<len;j++)
		if(i==a[j])
			return 1;
	return 0;
}

//��ü�����Է����ص�����
int GetBase(int team)
{
	const GameInfo *INFO = Info();
	int i;
	for(i=0; i<INFO->element_num; i++)
		if(INFO->elements[i].type == BASE && INFO->elements[i].team == team)
			return i; 
}

//����֮��ľ���
int distance(Position pos1, Position pos2)
{
	return abs(pos1.x - pos2.x) + abs(pos1.y - pos2.y);
}

//�ж�ĳ��λ�Ƿ��Դ��
int if_alive(int operand)
{
	for(int i=0;i<INFO->element_num;i++)
		if(INFO->elements[i].index == operand)
			return 1;
	return 0;
}

//���ֵ
int max(int x, int y)
{
	return x>y?x:y;
}

//��þ��λ�������ĳ���͵�λ������
int GetNear(Position pos, ElementType type)
{
	int i,near_index = -1,min_distance = 10000;
	for(i=0;i<INFO->element_num;i++)
		{
		if(INFO->elements[i].team == INFO->team_num && INFO->elements[i].type == type &&  distance(INFO->elements[i].pos, pos)<min_distance 
			&& type != OILFIELD && type!=MINE)
			{
				near_index = i;
				min_distance = distance(INFO->elements[i].pos, pos);
			}
		else if(INFO->elements[i].type == type &&  distance(INFO->elements[i].pos, pos)<min_distance   //��֤��������
			&& type == OILFIELD && INFO->elements[i].fuel >= kElementInfos[CARGO].fuel_max)
		{
			near_index = i;
			min_distance = distance(INFO->elements[i].pos, pos);
		}
		else if(INFO->elements[i].type == type &&  distance(INFO->elements[i].pos, pos)<min_distance //��֤�󳡻��н���
			&& type == MINE  && INFO->elements[i].metal >= kElementInfos[CARGO].metal_max)
		{
			near_index = i;
			min_distance = distance(INFO->elements[i].pos, pos);
		}
		}
	return near_index;
}

//�ж϶Ըó�Ա�ĸ������Ƿ�����´����Ѿ��´���߼���ָ���򲻿ɣ�
int if_command(int i,CommandType type,ElementType target = kElementTypes)
{
	if(INFO->elements[i].type != BASE && (type == PRODUCE || type == FIX))return 0;      //������ά�޲���
	if((INFO->elements[i].type >= FIGHTER || INFO->elements[i].type == SUBMARINE) && type <= SUPPLYUNIT)return 0;    //��������
	if(type < FORWARD && type > command[i][0])  //���ƶ�ָ��
	{
		if((target == BASE || target == FORT || target == FIGHTER || target == SCOUT) && INFO->elements[i].type == SUBMARINE)
			return 0;                       // Ǳͧ����أ��ݵ㣬�ɻ�����
		if((INFO->elements[i].type == BASE || INFO->elements[i].type == FORT ) && target == SUBMARINE)
			return 0;                               //���أ��ݵ��Ǳͧ����
		if(INFO->elements[i].type == CARGO && type < ATTACKUNIT) return 0; //���佢û�й�����
		return 1;
	}
	if(type >= FORWARD && type > command[i][1])  //�ƶ�ָ��
	{   
		if(INFO->elements[i].type == BASE || INFO->elements[i].type == FORT)return 0;//���غ;ݵ㲻���ƶ�
		return 1;
	}
	return 0;
}


//���ڻغϿ�ʼʱ���Ϊ1�ĵ�λ���в���
void Cargo_Supply(int index)                           
{
	for(int j=0;j<INFO->element_num;j++)
		if(distance(INFO->elements[index].pos,INFO->elements[j].pos) <= 1 && index!=j && INFO->elements[index].team == INFO->team_num)
		{
			if(INFO->elements[index].type > OILFIELD)command[index][0] = SUPPLYUNIT;
			else if(INFO->elements[index].type == FORT)command[index][0] = SUPPLYFORT;
			else if(INFO->elements[index].type == BASE)command[index][0] = SUPPLYBASE;
			else continue;
			Supply(INFO->elements[index].index,INFO->elements[j].index,-1,-1,-1);
			break;
		}
}

//���ڳ��������Ȼû����������䴬��ȥ��Դ���ռ������߻ػ��ز��������ߴ���ǰ��
void MoveCargo()        
{
	for(int i=0;i<INFO->element_num;i++)
		if(INFO->elements[i].type == CARGO && INFO->elements[i].team == INFO->team_num)
		{
		State Element = INFO->elements[i];
		if(command[i][1] == -1 )
			{
				if(Element.fuel < 0.5 * kElementInfos[CARGO].fuel_max)          //ȼ�ϲ���
				{    ChangeDest(Element.index,INFO->elements[GetNear(Element.pos,OILFIELD)].pos);
					 command[i][1] = CARGOSUPPLY;
				}
				else if(Element.metal < 0.5 * kElementInfos[CARGO].metal_max)     //��������
				{	ChangeDest(Element.index,INFO->elements[GetNear(Element.pos,MINE)].pos);
					command[i][1] = CARGOSUPPLY;
				}
				else if(if_in(Element.index, cargo_index,2))         //�ǲ������صĴ�
				{	ChangeDest(Element.index,INFO->elements[GetBase(INFO->team_num)].pos);   
					command[i][1] = RETURNBASE;
				}	
				else                   //�ǲ���ǰ�ߵĴ�
				{
					Forward(i);
				}
			}
		}
}

//Ѱ������ڵĻ��ػ�ݵ㣬��ѡ��Ѫ�����ٵĵ�λ����,�������˶��������ǡ����̵ĵط�
void Attack(int index)    
{
	State	Element = INFO->elements[index];
	int health = 1000,enemy_index = -1;
	if(!if_command(index,ATTACKBASE))return;
	for(int i=0;i<enemy_num;i++)
	{
		State enemy = INFO->elements[i];
		if(distance(Element.pos, enemy.pos) <= kElementInfos[Element.type].fire_ranges[kElementInfos[enemy.type].level])
		{	
			if((enemy.type == BASE || (enemy.type == FORT && enemy.team == 1-INFO->team_num)) && 
				if_command(index,CommandType(ATTACKBASE -(enemy.type-BASE)),ElementType(enemy.type)))                      //�Է��ǻ��ػ�ݵ�
			{
				AttackUnit(Element.index,enemy.index);
				command[index][0] = CommandType(ATTACKBASE -(enemy.type-BASE));
				if(if_command(index,FORWARD))
				{
					ChangeDest(Element.index,minus(Element.pos,enemy.pos,kElementInfos[Element.type].fire_ranges[kElementInfos[Element.type].level]));
					command[index][1] = FORWARD;
				}
				return;
			}
			if(enemy.type == FORT && enemy.team != 1-INFO->team_num  && enemy.health <= parameter[4]*kElementInfos[Element.type].attacks[0] 
				&& if_command(index,ATTACKFORT,FORT))                                     //�����ݵ�
			{
				AttackUnit(Element.index,enemy.index);
				command[index][0] = ATTACKFORT;
				if(if_command(index,FORWARD))
				{
					ChangeDest(Element.index,minus(Element.pos,enemy.pos,kElementInfos[Element.type].fire_ranges[kElementInfos[Element.type].level]));
					command[index][1] = FORWARD;
				}
				return;
			}
			if(health>enemy.health)   
			{
				health=enemy.health;
				enemy_index=i;
			}
		}
	}
	if(enemy_index != -1 && if_command(index,ATTACKUNIT,ElementType(INFO->elements[enemy_index].type)))
	{
		AttackUnit(Element.index,INFO->elements[enemy_index].index);
		command[index][0] = ATTACKUNIT;
		if(if_command(index,FORWARD))
		{
			ChangeDest(Element.index,minus(Element.pos,INFO->elements[enemy_index].pos,kElementInfos[Element.type].fire_ranges[kElementInfos[Element.type].level]));
			command[index][1] = FORWARD;
		}
	}
}

//���ص�ά�ޣ�����
void BaseAct()
{
	int index,health=1000;
	for(int i=0;i<INFO->element_num;i++)
	{
		if(distance(INFO->elements[i].pos,INFO->elements[GetBase(INFO->team_num)].pos)<=1 && 
			INFO->elements[i].health < parameter[5]*kElementInfos[INFO->elements[i].type].health_max
			&& health>INFO->elements[i].health && INFO->elements[i].team == INFO->team_num)
		{
			index = i;
			health = INFO->elements[i].health;
		}
	}
	if(if_command(GetBase(INFO->team_num),FIX) && health != 1000)
	{	
		Fix(INFO->elements[GetBase(INFO->team_num)].index,index);
		command[GetBase(INFO->team_num)][0] = FIX;
		return;
	}
		BaseProduce();   //��������
}

///���ص�����
void BaseProduce()
{
	int index = GetBase(INFO->team_num);
	State Element =INFO->elements[index];
	if(!if_command(index,PRODUCE))return;
	for(int i=0; i<2;i++)        //�жϲ������ص��������Ƿ���������ֱ���������佢
	{
		if(cargo_index[i] == -2 || (cargo_index[i]>=0 && !if_alive(cargo_index[i])))
		{
			Produce(Element.index,CARGO);
			command[index][0] = PRODUCE;
			cargo_index[i] = -1;
			return;
		}
	} 

	//�м���·������
	if(produce_state[0]<3 && produce_state[1]<= 4)         
		switch(produce_state[1])
		{
			case 0:Produce(Element.index,SUBMARINE);command[index][0] = PRODUCE;produce_state[1] += 1;return;
			case 1: Produce(Element.index,DESTROYER);command[index][0] = PRODUCE;produce_state[1] += 1;return;
			case 2:
			case 3: Produce(Element.index,FIGHTER);command[index][0] = PRODUCE;produce_state[1] += 1;return;
			case 4: Produce(Element.index,SCOUT);command[index][0] = PRODUCE;produce_state[0] += 1; return;
			default:;
		}

	 //����·������
	if(produce_state[0]>= 3 && produce_state[0]<5 && produce_state[1] <= 2)        
		switch(produce_state[1])
		{
			case 0:
			case 1: Produce(Element.index,FIGHTER);command[index][0] = PRODUCE;produce_state[1] += 1;return;
			case 2: Produce(Element.index,SCOUT);command[index][0] = PRODUCE;produce_state[1] += 1;return;
			default:;
		}

	 //����·������·������������һ�����䴬
	if((produce_state[0] == 1 || produce_state[0] == 3 )&& produce_state[1] == 5)    
	{
		Produce(Element.index,CARGO);
		command[index][0] = PRODUCE;
		produce_state[0] += 1;
		produce_state[1] = 0;
		return;
	}

	//������·������·�����������ս����ȥ������Դ
	if((produce_state[0] == 2 || produce_state[0] == 4)&& produce_state[1] == 5 && resource_defender[1] == -2 )
	{															
		int i = resource_defender[0] == -2 ? 0 : 1;
		Produce(Element.index,FIGHTER);
		command[index][0] = PRODUCE;
		resource_defender[i] = -1;
		produce_state[0] += 1;
		if(produce_state[0] == 5)produce_state[0] = 1;
		produce_state[1] = 0;
		return;
	}

	//����Ǳͧ���ػ���
	if(INFO->round > 5 && produce_state[0] == 0 && produce_state[1] == 5 && base_defender[1] == -2) 
	{
		int i = base_defender[0] == -2 ? 0 : 1;
		Produce(Element.index,SUBMARINE);
		command[index][0] = PRODUCE;
		resource_defender[i] = -1;
		produce_state[0] += 1;
	}

}

////Ѱ������ĵо�ǰ������������䴬
void Forward(int index)
{
	State Element = INFO->elements[index];
	int Distance=1000; 
	Position target;
	if(!if_command(index,FORWARD))return;
	if(Element.type == BASE || Element.type == FORT)
		return;
	else if(Element.type == CARGO)
	{
		for(int i=0;i<INFO->element_num;i++)
			if(distance(Element.pos,INFO->elements[i].pos) < Distance && index!=i && INFO->elements[index].team == INFO->team_num)
			{
				Distance = distance(Element.pos,INFO->elements[i].pos) < Distance;
				target = INFO->elements[i].pos;
			}
	}
	else
	{
		for(int i=0;i<enemy_num;i++)
			if(distance(Element.pos,enemy_element[i].pos) < Distance && index!=i)
			{
				Distance = distance(Element.pos,enemy_element[i].pos) < Distance;
				target = enemy_element[i].pos;
			}
	}
	ChangeDest(Element.index,target);
	command[index][1] = FORWARD;
}

//�жϸձ�����������ʱ��λ�Ĵ���
void Difference(int index)
{
	State Element = INFO->elements[index];
	switch(Element.type)
	{
	case BASE:break;
	case FORT:break;
	case CARGO:
		for(int i=0;i<=1;i++)     //�ж��Ƿ�Ϊ�����������Ĳ������ص����䴬
			if(cargo_index[i] == -1 && distance(Element.pos,INFO->elements[GetBase(INFO->team_num)].pos) <= 1)
			{
				cargo_index[i] = Element.index;
				return;
			}
		return;
	case SUBMARINE:
		for(int i=0;i<2;i++)
			if(base_defender[i] == -1 && distance(Element.pos,INFO->elements[GetBase(INFO->team_num)].pos) <= 1)
			{
				base_defender[i] = Element.index;
				return;
			}
		ChangeDest(Element.index,origin_target[unit_union[0]]);
		if(unit_union[0] == 4)
			unit_union[0] = 0;
		else unit_union[0] += 1;
		return;
	case DESTROYER:
		ChangeDest(Element.index,origin_target[unit_union[1]]);
		if(unit_union[1] == 4)
			unit_union[1] = 0;
		else unit_union[1] += 1;
		return;
	case FIGHTER:
		ChangeDest(Element.index,origin_target[abs(unit_union[1])-1]);
		if(unit_union[2] > 0)
			unit_union[2] = -unit_union[2];
		else if(unit_union[2] == -5) 
			unit_union[2] = 1;
		else 
			unit_union[2] = -unit_union[2] + 1;
		return;
	case SCOUT:
		ChangeDest(Element.index,origin_target[abs(unit_union[1])-1]);
		if(unit_union[3] == 4)
			unit_union[3] = 0;
		else unit_union[3] += 1;
		return;
	}

}