#include "basic.h"
#include<iostream>
#include<cmath>
#include<vector>


// Remove this line if you konw C++, and don't want a dirty namespace
using namespace teamstyle16;
using std::vector;
using std::cout;
using std::endl;

#define INF 1000
const char * GetTeamName()
{
    return "teamstyle16";  // Name of your team
}

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

void init();//��ʼ������������ĺ���
void enemy_init();//����Info()��ʼ����ʱ�ĵ���
void Supply_Repair(int i);/////�ò���ȼ�ϻ��ߵ�ҩ��ʱ�򣬻���Ѫ��̫����Ҫ�ػ���ά��
Position minus(Position pos1,Position pos2,int fire_range);//����pos1��pos2ǡ�����fire_range��һ��λ��
int if_in(int i,int *a,int len);//�ж�ĳ�����Ƿ���������
int GetBase(int team);//��ü�����Է����ص�����
int distance(Position pos1, Position pos2);//����֮��ľ���
int DisToBase(Position pos1, Position pos2);  ////���ص����صľ��룬 pos2��ʾ�������Ͻ�
int if_alive(int operand);//�ж�ĳ��λ�Ƿ��Դ��
int max(int x, int y);//���ֵ
int min(int x, int y);//��Сֵ
int GetNear(Position pos, ElementType type);//��þ��λ�������ĳ���͵�λ������
int if_command(int i,CommandType type,ElementType target = kElementTypes);//�ж϶Ըó�Ա�ĸ������Ƿ�����´����Ѿ��´���߼���ָ���򲻿ɣ�
void Cargo_Supply(int index);//���ڻغϿ�ʼʱ���Ϊ1�ĵ�λ���в���
void MoveCargo(int i);//���ڳ��������Ȼû����������䴬��ȥ��Դ���ռ������߻ػ��ز��������ߴ���ǰ��
void Attack(int index);//Ѱ������ڵĻ��ػ�ݵ㣬��ѡ��Ѫ�����ٵĵ�λ����,�������˶��������ǡ����̵ĵط�
void BaseAct(int index);//���ص�ά�ޣ�����
void BaseProduce(int index);///���ص�����
void Forward(int index);////Ѱ������ĵо�ǰ������������䴬
void Difference(int index);//�жϸձ�����������ʱ��λ�Ĵ���


const GameInfo *INFO = Info();
int enemy_num,DISTANCE;
State * enemy_element;
int (*command)[2];  //���غϷ�����ָ�һ��Ч��ָ�һ���ƶ�ָ��
double parameter[4];
int FormerElement[100];  //�ϻغϱ�����λ��index����
int cargo_index[2];  //�����������ص�     ��ʼ��Ϊ-2��-1��ʾ������
int resource_defender[2];  //������Դ��Ǳͧ����ʼ��Ϊ-2��-1��ʾ������
int base_defender[2];      //���ػ��ص�Ǳͧ����ʼ��Ϊ -2��-1��ʾ������
Position origin_target[5];  //����·����+Ǳͧ+�ɻ�*2+����,����ɻ�*2+����  ��Ŀ�ĵ�
int produce_state[2];   //��һ����ʾ·����·�����ϣ����£��ϣ��·ֱ�Ϊ0-4
						//�ڶ�����ʾ�������������˳��ΪǱͧ�����𽢣��ɻ����ɻ�������
						//��ʼ��Ϊ-1��0
int unit_union[4];   //��������������Ǳͧ�����𽢣��ɻ������������ļ��ţ���ʼ��Ϊ0 �����ڷɻ�����ʼ��Ϊ1����Ϊ-1����Ϊ2,��Ϊ-2��
vector<int>Enemy_Indexes;


void AIMain()
{  
	TryUpdate();
	//�غϿ�ʼʱ�ĳ�ʼ��
	init(); 
	for(int i=0;i<INFO->element_num;i++)
		if(GetState(INFO->elements[i])->team == INFO->team_num)
		{
			const State *Element = GetState(INFO->elements[i]);
			Supply_Repair(i); //�ò�����ά�޾�ȥ������ά��
			Difference(i);    //�жϸճ��ֵĵ�λ�Ĵ���

			//TryUpdate();
			//enemy_init();
			Attack(i);        //����

			if(Element->type == BASE)
				BaseAct(i);     //����ά�ޣ�����������
			else if(Element->type == CARGO)
			{
				Cargo_Supply(i);   //���佢����
				MoveCargo(i);      //���佢�˶�
			}	
			else ;

			Forward(i);       //ǰ��
		}

	for(int i=0;i<Info()->element_num;i++)   //��¼һ�غϽ���ʱ�ĵ�λ��������һ�غ�ȷ�������������ĵ�λ
		if(GetState(Info()->elements[i])->team == Info()->team_num)
			FormerElement[i] = Info()->elements[i];
	delete []enemy_element;	
}

//��ʼ������������ĺ���
void init()
{
	if(INFO->round == 1)
	{
		parameter[0] = 0.3; //Ѫ��
		parameter[1] = 0.3; //����
		parameter[2] = 0.3; //ȼ��
		parameter[3] = 0.4; //��ҩ

		cargo_index[0] = -2;  //�������ص����佢
		cargo_index[1] = -2;

		resource_defender[0] = -2;   //������Դ�ķɻ�
		resource_defender[1] = -2;

		base_defender[0] = -2;  //���ػ��ص�Ǳͧ
		base_defender[1] = -2;

		produce_state[0] = -1;   //�������
		produce_state[1] = 0;

		unit_union[0] = 0;   //����������ս����λ�����ļ���
		unit_union[1] = 0;
		unit_union[2] = 0;
		unit_union[3] = 0;

		//��ʼĿ�ĵ�
		origin_target[0].x = INFO->x_max/2;   //��·Ŀ�ĵ�
		origin_target[0].y = INFO->y_max/2;
		origin_target[0].z = 1;

		for(int i = 0; i <= 1; i++)        //���ϣ�����·Ŀ�ĵ�
		{
			origin_target[1 + i].x = INFO->x_max/2 + (2 * i - 1)*INFO->x_max/4; 
			origin_target[1 + i].y = INFO->y_max/2 - (2 * i - 1)*INFO->y_max/4; 
			origin_target[1 + i].z = 1;
		}

		for(int i = 0; i <= 1; i++)  //����·Ŀ�ĵ�
		{
			origin_target[3 + i].x = min(INFO->x_max/2 + (2 * i - 1)*INFO->x_max/2, INFO->x_max - 1); 
			origin_target[3 + i].y = min(INFO->y_max/2 - (2 * i - 1)*INFO->y_max/2, INFO->y_max - 1); 
			origin_target[3 + i].z = 1;
		}
	}
	command = new int[INFO->element_num][2];    //�����б��ʼ��
	for(int i = 0; i<INFO->element_num; i++)
		for(int j=0;j<2;j++)
			command[i][j] = -1; 
	enemy_init();
}

//����Info()��ʼ����ʱ�ĵ���
void enemy_init()
{
	//�з���λ���������з���λ�������ݵ㣬�������󳡺�����
	enemy_num =0;
	for(int i=0;i<Info()->element_num;i++)
		if(GetState(Info()->elements[i])->team != Info()->team_num  
			&& GetState(Info()->elements[i])->type != MINE && GetState(Info()->elements[i])->type != OILFIELD)
		{
			enemy_num += 1;
			Enemy_Indexes.push_back(i);
		}	
	enemy_element = new State[enemy_num];
	for(int i=0; i<enemy_num; i++)
		{
			enemy_element[i] = *GetState(Info()->elements[Enemy_Indexes[i]]);
		}
	Enemy_Indexes.clear();
}

/////�ò���ȼ�ϻ��ߵ�ҩ��ʱ�򣬻���Ѫ��̫����Ҫ�ػ���ά��
void Supply_Repair(int i)
{
	const State *Element = GetState(INFO->elements[i]);
	if(Element->health < parameter[0] * kElementInfos[Element->type].health_max 
		&& Element->health != -1
		&& if_command(i, RETURNBASE))
	{
		ChangeDest(Element->index, GetState(INFO->elements[GetBase(INFO->team_num)])->pos);
		command[i][1] = RETURNBASE;
	}
	else if((Element->ammo <  parameter[1]*kElementInfos[Element->type].ammo_max && Element->ammo != -1) ||   //��������
			Element->fuel < parameter[2]*kElementInfos[Element->type].fuel_max)          //ȼ�ϲ���
	{
		int cargo_index = GetNear(Element->pos, CARGO);
		if(cargo_index >= 0)
		{
			if(if_command(cargo_index, CARGOSUPPLY))
			{
				ChangeDest(INFO->elements[cargo_index], Element->pos);
				command[cargo_index][1] = CARGOSUPPLY;	
			}
			else
			{
				ChangeDest(Element->index, GetState(INFO->elements[cargo_index])->pos);
				command[i][1] = CARGOSUPPLY;
			}
		}
		else
		{
			ChangeDest(Element->index, GetState(INFO->elements[GetBase(INFO->team_num)])->pos);
			command[i][1] = RETURNBASE;
		}
		
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
	int i;
	for(i=0; i<INFO->element_num; i++)
		if(GetState(INFO->elements[i])->type == BASE && GetState(INFO->elements[i])->team == team)
			return i; 
	return -1;
}

//����֮��ľ���
int distance(Position pos1, Position pos2)
{
	return abs(pos1.x - pos2.x) + abs(pos1.y - pos2.y);
}

//���ص����صľ���
int DisToBase(Position pos1, Position pos2)  //pos2��ʾ�������Ͻ�
{
	int min = 1000;
	if(pos1.x < pos2.x)//��Ϊ�ſ�
	{
		if(pos1.y > pos2.y)
			return abs(pos1.x - pos2.x) + abs(pos1.y - pos2.y);
		else if(pos1.y < pos2.y - 2)
			return abs(pos1.x - pos2.x) + abs(pos1.y - pos2.y + 2);
		else 
			return abs(pos1.x - pos2.x);
	}
	else if(pos1.x > pos2.x +2)
	{
		if(pos1.y > pos2.y)
			return abs(pos1.x - pos2.x - 2) + abs(pos1.y - pos2.y);
		else if(pos1.y < pos2.y - 2)
			return abs(pos1.x - pos2.x - 2) + abs(pos1.y - pos2.y + 2);
		else 
			return abs(pos1.x - pos2.x - 2);
	}
	else
	{
		if(pos1.y > pos2.y)
			return abs(pos1.y - pos2.y);
		else if(pos1.y < pos2.y -2)
			return abs(pos1.y - pos2.y +2);
		else 
			return 0;
	}
}

//�ж�ĳ��λ�Ƿ��Դ��
int if_alive(int operand)
{
	for(int i=0;i<INFO->element_num;i++)
		if(INFO->elements[i] == operand)
			return 1;
	return 0;
}

//���ֵ
int max(int x, int y)
{
	return x>y?x:y;
}

//��Сֵ
int min(int x, int y)
{
	return x<y?x:y;
}

//��þ��λ�������ĳ���͵�λ������
int GetNear(Position pos, ElementType type)
{
	int i,near_index = -1,min_distance = 10000;
	for(i=0;i<INFO->element_num;i++)
	{
		const State *target = GetState(INFO->elements[i]);
		if(target->type == type &&  distance(target->pos, pos)<min_distance)
		{
			if( type == OILFIELD && target->fuel >= kElementInfos[CARGO].fuel_max) //���ﻹ��ʯ��
			{
				near_index = i;
				min_distance = distance(target->pos, pos);
			}
			else if(type == MINE  && target->metal >= kElementInfos[CARGO].metal_max)//��֤�󳡻��н���
			{
				near_index = i;
				min_distance = distance(target->pos, pos);
			}
			else if(target->team == INFO->team_num && type != OILFIELD && type!=MINE)
			{
				near_index = i;
				min_distance = distance(target->pos, pos);
			}
			else ;
		}
	}
	return near_index;
}

//�ж϶Ըó�Ա�ĸ������Ƿ�����´����Ѿ��´���߼���ָ���򲻿ɣ�
int if_command(int i,CommandType type,ElementType target)
{
	const State *element = GetState(INFO->elements[i]);
	if(type < FORWARD && type > command[i][0])  //���ƶ�ָ��
	{
		if(type == PRODUCE || type == FIX)     //������ά�޲���
		{
			if(element->type == BASE) return 1;
			return 0;
		}
		else if(type <= SUPPLYUNIT)      //��������
		{
			if(element->type >= FIGHTER 
				|| element->type == DESTROYER 
				|| element->type == SUBMARINE)
				return 0;
			return 1;
		}
		else if(target == kElementTypes)
			return 1;
		else if((kElementInfos[element->type].attacks[0] <= kElementInfos[target].defences[0] 
					|| kElementInfos[target].defences[0] == -1)
			&& (kElementInfos[element->type].attacks[1] <= kElementInfos[target].defences[1]
					|| kElementInfos[target].defences[1] == -1))
			return 0;
		else ;
		return 1;
	}
	if(type >= FORWARD && type > command[i][1])  //�ƶ�ָ��
	{   
		if(element->type <= FORT)
			return 0;//���غ;ݵ㲻���ƶ�
		return 1;
	}
	return 0;
}


//���ڻغϿ�ʼʱ���Ϊ1�ĵ�λ���в���
void Cargo_Supply(int index)                           
{
	const State *element, *target;
	element = GetState(INFO->elements[index]);
	for(int j=0;j<INFO->element_num;j++)
	{
		target = GetState(INFO->elements[j]);
		if(distance(element->pos,GetState(INFO->elements[j])->pos) <= 1 
			&& index!=j 
			&& target->team == INFO->team_num
			&& target->type != CARGO)
		{
			if(target->type > OILFIELD)command[index][0] = SUPPLYUNIT;
			else if(target->type == FORT)command[index][0] = SUPPLYFORT;
			else if(target->type == BASE)command[index][0] = SUPPLYBASE;
			else continue;
			Supply(INFO->elements[index], INFO->elements[j],INF,INF,INF);
			return;
		}
	}	
}

//���ڳ��������Ȼû����������䴬��ȥ��Դ���ռ������߻ػ��ز��������ߴ���ǰ��
void MoveCargo(int i)        
{
	const State *Element = GetState(INFO->elements[i]);
	if(command[i][1] == -1 )
	{
		if(Element->fuel < 0.5 * kElementInfos[CARGO].fuel_max)          //ȼ�ϲ���
		{   
			ChangeDest(Element->index,GetState(INFO->elements[GetNear(Element->pos,OILFIELD)])->pos);
			command[i][1] = CARGOSUPPLY;
		}
		else if(Element->metal < 0.5 * kElementInfos[CARGO].metal_max)     //��������
		{	
			ChangeDest(Element->index,GetState(INFO->elements[GetNear(Element->pos,MINE)])->pos);
			command[i][1] = CARGOSUPPLY;
		}
		else if(if_in(Element->index, cargo_index, 2))         //�ǲ������صĴ�
		{	
			ChangeDest(Element->index,GetState(INFO->elements[GetBase(INFO->team_num)])->pos);   
			command[i][1] = RETURNBASE;
		}	
		else                   //�ǲ���ǰ�ߵĴ�
			Forward(i);
	}
}

//Ѱ������ڵĻ��ػ�ݵ㣬��ѡ��Ѫ�����ٵĵ�λ����,�������˶��������ǡ����̵ĵط�
void Attack(int index)
{
	const State	*Element = GetState(INFO->elements[index]);
	int health = 1000,enemy_index = -1;
	if(!if_command(index,ATTACKBASE))
		return;
	for(int i=0;i<enemy_num;i++)
	{
		State *enemy = &enemy_element[i];
		cout<<Element->type<<" "<<enemy->type<<" "<<kElementInfos[enemy->type].level<<endl;
		cout<<distance(Element->pos, enemy->pos)<<" "<<kElementInfos[Element->type].fire_ranges[kElementInfos[enemy->type].level]<<endl;
		cout<<endl;
		if(enemy->type == BASE && enemy->team == 1-INFO->team_num
			&& DisToBase(Element->pos, enemy->pos) <= kElementInfos[Element->type].fire_ranges[SURFACE]
			&& if_command(index, ATTACKBASE, BASE))
		{												//�з�����
			AttackUnit(Element->index,enemy->index);
			command[index][0] = ATTACKBASE;
			if(if_command(index, FORWARD))
			{
				ChangeDest(Element->index,
					minus(Element->pos,enemy->pos,kElementInfos[Element->type].fire_ranges[SURFACE]));
				command[index][1] = FORWARD;
			}
			return;
		}
		if(distance(Element->pos, enemy->pos) <= kElementInfos[Element->type].fire_ranges[kElementInfos[enemy->type].level])
		{	
			if(enemy->type == FORT
				&& if_command(index,ATTACKFORT,FORT))                                     //�ݵ�
			{
				AttackUnit(Element->index,enemy->index);
				command[index][0] = ATTACKFORT;
				if(if_command(index,FORWARD))
				{
					ChangeDest(Element->index,
						minus(Element->pos,enemy->pos,kElementInfos[Element->type].fire_ranges[SURFACE]));
					command[index][1] = FORWARD;
				}
				return;
			}
			if(health > enemy->health 
				&& (enemy_index == -1 
					|| (enemy_index >=0 && 
					if_command(index, ATTACKUNIT, ElementType(enemy_element[enemy_index].type)))))   
			{
				health=enemy->health;					//Ѱ��Ѫ�����ٵĵз���λ
				enemy_index=i;
			}
		}
	}
	if(enemy_index != -1)
	{
		AttackUnit(Element->index, enemy_element[enemy_index].index);
		command[index][0] = ATTACKUNIT;
		if(if_command(index,FORWARD))
		{
			ChangeDest(Element->index, minus(Element->pos,enemy_element[enemy_index].pos,
				kElementInfos[Element->type].fire_ranges[kElementInfos[enemy_element[enemy_index].type].level]));
			command[index][1] = FORWARD;
		}
	}
}

//���ص�ά�ޣ�����
void BaseAct(int index)
{
	const State *target;
	for(int i=0;i<INFO->element_num;i++)
	{
		target = GetState(INFO->elements[i]);
		if(DisToBase(target->pos, GetState(INFO->elements[index])->pos)<=1 
			&& target->index != INFO->elements[index]
			&& target->team == INFO->team_num)
		{
			if(target->health < parameter[0]*kElementInfos[target->type].health_max
				&& target->health != -1)
			{
				if(if_command(index, FIX))
				{	
					Fix(INFO->elements[index], INFO->elements[i]);
					command[index][0] = FIX;
					return;
				}
			}
			else if(target->fuel < parameter[2]*kElementInfos[target->type].fuel_max
					|| target->ammo < parameter[3]*kElementInfos[target->type].ammo_max)
			{
				if(if_command(index, SUPPLYUNIT))
				{	
					Supply(INFO->elements[index], INFO->elements[i], INF, INF, INF);
					command[index][0] = SUPPLYUNIT;
					return;
				}
			}
		}
	}
	BaseProduce(index);   //��������
}

///���ص�����
void BaseProduce(int index)
{
	State Element = *GetState(INFO->elements[index]);
	if(!if_command(index, PRODUCE))return;
	for(int i=0; i<2;i++)        //�жϲ������ص��������Ƿ���������ֱ���������佢
	{
		if(cargo_index[i] == -2 || (cargo_index[i]>=0 && !if_alive(cargo_index[i])))
		{
			Produce(CARGO);
			command[index][0] = PRODUCE;
			cargo_index[i] = -1;
			return;
		}
	} 

	//�м���·������
	if(produce_state[0]<3 && produce_state[1]<= 4)         
		switch(produce_state[1])
		{
			case 0: Produce(SUBMARINE);
					command[index][0] = PRODUCE;
					produce_state[1] += 1;
					return;
			case 1: Produce(DESTROYER);
					command[index][0] = PRODUCE;
					produce_state[1] += 1;
					return;
			case 2:
			case 3: Produce(FIGHTER);
					command[index][0] = PRODUCE;
					produce_state[1] += 1;
					return;
			case 4: Produce(SCOUT);
					command[index][0] = PRODUCE;
					produce_state[0] += 1; 
					return;
			default:;
		}

	 //����·������
	if(produce_state[0]>= 3 && produce_state[0]<5)        
		switch(produce_state[1])
		{
			case 0: 
			case 1:
			case 2:
			case 3: Produce(FIGHTER);
					command[index][0] = PRODUCE;
					produce_state[1] += 1;
					return;
			case 4: Produce(SCOUT);
					command[index][0] = PRODUCE;
					produce_state[0] = produce_state[0] == 4 ? 0 :4;
					produce_state[1] = 0;
					return;
			default:;
		}

	 //����·������·������������һ�����䴬
	if((produce_state[0] == 1 || produce_state[0] == 3 )&& produce_state[1] == 5)    
	{
		Produce(CARGO);
		command[index][0] = PRODUCE;
		produce_state[0] += 1;
		produce_state[1] = 0;
		return;
	}

	//������·������·�����������ս����ȥ������Դ
	if((produce_state[0] == 2 || produce_state[0] == 4)&& produce_state[1] == 5 
		&& (resource_defender[1] == -2     //������Դ�ĵ�λû�л����Ѿ�����
			|| (resource_defender[1] >= 0
			&& ! if_alive(resource_defender[1]))))
	{															
		int i = resource_defender[0] == -2 ? 0 : 1;
		Produce(FIGHTER);
		command[index][0] = PRODUCE;
		resource_defender[i] = -1;
		produce_state[0] += 1;
		if(produce_state[0] == 5)produce_state[0] = 0;
		produce_state[1] = 0;
		return;
	}

	//����Ǳͧ���ػ���
	if(INFO->round > 5 && produce_state[0] == 0 && produce_state[1] == 5
		&& (base_defender[1] == -2     //������Դ�ĵ�λû�л����Ѿ�����
			|| (base_defender[1] >= 0
			&& ! if_alive(base_defender[1]))))
	{
		int i = base_defender[0] == -2 ? 0 : 1;
		Produce(SUBMARINE);
		command[index][0] = PRODUCE;
		base_defender[i] = -1;
		produce_state[0] += 1;
		produce_state[1] = 0;
	}

}

////Ѱ������ĵо�ǰ������������䴬�ʹ�
void Forward(int index)
{
	State Element = *GetState(INFO->elements[index]);
	int Distance=1000; 
	Position target;
	if(!if_command(index, FORWARD))return;
	if(Element.type == BASE || Element.type == FORT)
		return;
	else if(Element.type == CARGO && ! if_in(Element.index, cargo_index, 2))
	{
		for(int i=0;i<INFO->element_num;i++)
			if(distance(Element.pos,GetState(INFO->elements[i])->pos) < Distance 
				&& index!=i 
				&& GetState(INFO->elements[i])->team == INFO->team_num)
			{
				Distance =  distance(Element.pos,GetState(INFO->elements[i])->pos);
				target = GetState(INFO->elements[i])->pos;
			}
	}
	else
	{
		for(int i=0;i<enemy_num;i++)
			if(distance(Element.pos, enemy_element[i].pos) < Distance)
			{
				Distance = distance(Element.pos,enemy_element[i].pos);
				target = enemy_element[i].pos;
			}
	}
	if(Distance != 1000)
	{
		ChangeDest(Element.index, target);
		command[index][1] = FORWARD;	
	}
}

//�жϸձ�����������ʱ��λ�Ĵ���
void Difference(int index)
{
	State Element = *GetState(INFO->elements[index]);
	for(int i=0;i<INFO->element_num - enemy_num;i++)  //�ж����Ƿ���ԭ���ʹ��ڵ�
		if(index == FormerElement[i])
			return ;
	switch(Element.type)
	{
	case BASE:break;
	case FORT:break;
	case CARGO:
		for(int i=0;i<=1;i++)     //�ж��Ƿ�Ϊ�����������Ĳ������ص����䴬
			if(cargo_index[i] == -1 && DisToBase(Element.pos,GetState(INFO->elements[GetBase(INFO->team_num)])->pos) <= 1)
			{
				cargo_index[i] = Element.index;
				return;
			}
		return;
	case SUBMARINE:
		for(int i=0;i<2;i++)
			if(base_defender[i] == -1 && DisToBase(Element.pos,GetState(INFO->elements[GetBase(INFO->team_num)])->pos) <= 1)
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