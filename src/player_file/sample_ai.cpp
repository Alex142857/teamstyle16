#include "basic.h"
#include<iostream>
#include<cmath>


// Remove this line if you konw C++, and don't want a dirty namespace
using namespace teamstyle16;

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
void Supply_Repair(int i);/////�ò���ȼ�ϻ��ߵ�ҩ��ʱ�򣬻���Ѫ��̫����Ҫ�ػ���ά��
Position minus(Position pos1,Position pos2,int fire_range);//����pos1��pos2ǡ�����fire_range��һ��λ��
int damage(State victim,State attacker);//���㹥�����˺�
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
void MoveCargo();//���ڳ��������Ȼû����������䴬��ȥ��Դ���ռ������߻ػ��ز��������ߴ���ǰ��
void Attack(int index);//Ѱ������ڵĻ��ػ�ݵ㣬��ѡ��Ѫ�����ٵĵ�λ����,�������˶��������ǡ����̵ĵط�
void BaseAct();//���ص�ά�ޣ�����
void BaseProduce();///���ص�����
void Forward(int index);////Ѱ������ĵо�ǰ������������䴬
void Difference(int index);//�жϸձ�����������ʱ��λ�Ĵ���

const GameInfo *INFO = Info();
int enemy_num,DISTANCE;
State * enemy_element;
int (*command)[2];  //���غϷ�����ָ�һ��Ч��ָ�һ���ƶ�ָ��
double parameter[10];
int FormerElement[100];  //�ϻغϱ�����λ��index����
int cargo_index[2];  //�����������ص�     ��ʼ��Ϊ-2��-1��ʾ������
int resource_defender[2];  //������Դ��Ǳͧ����ʼ��Ϊ-2��-1��ʾ������
int base_defender[2];      //���ػ��ص�Ǳͧ����ʼ��Ϊ -2��-1��ʾ������
Position origin_target[5];  //����·����+Ǳͧ+�ɻ�*2+����,����ɻ�*2+����  ��Ŀ�ĵ�
int produce_state[2];   //��һ����ʾ·����·�����ϣ����£��ϣ��·ֱ�Ϊ0-4
						//�ڶ�����ʾ�������������˳��ΪǱͧ�����𽢣��ɻ����ɻ�������
						//��ʼ��Ϊ-1��0
int unit_union[4];   //��������������Ǳͧ�����𽢣��ɻ������������ļ��ţ���ʼ��Ϊ0 �����ڷɻ�����ʼ��Ϊ1����Ϊ-1����Ϊ2,��Ϊ-2��


void AIMain()
{  
	//�غϿ�ʼʱ�ĳ�ʼ��
		init(); 
	for(int i=0;i<INFO->element_num;i++)
		if(GetState(INFO->elements[i])->team == INFO->team_num)
		{
			State Element = *GetState(INFO->elements[i]);
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
	for(int i=0;i<Info()->element_num;i++)
		if(GetState(Info()->elements[i])->team == Info()->team_num)
			FormerElement[i] = Info()->elements[i];
	delete []enemy_element;
}

//��ʼ������������ĺ���
void init()
{
	
	if(INFO->round == 1)
	{
		parameter[0] = 0.1; parameter[1] = 0.3; parameter[2] = 0.3; parameter[3] = 0.2; parameter[4] = 0.5; 
		cargo_index[0] = -2; cargo_index[1] = -2;
		resource_defender[0] = -2; resource_defender[1] = -2;
		base_defender[0] = -2; base_defender[1] = -2;
		produce_state[0] = -1; produce_state[1] = 0;
		unit_union[0] = 0; unit_union[1] = 0; unit_union[2] = 0; unit_union[3] = 0;

		//��ʼĿ�ĵ�
		origin_target[0].x = INFO->x_max/2; origin_target[0].y = INFO->y_max/2; origin_target[0].x = 1;
		for(int i = 0; i <= 1; i++)
		{
			origin_target[1 + i].x = INFO->x_max/2 + (2 * i - 1)*INFO->x_max/4; 
			origin_target[1 + i].y = INFO->y_max/2 - (2 * i - 1)*INFO->y_max/4; 
			origin_target[1 + i].z = 1;
		}
		for(int i = 0; i <= 1; i++)
		{
			origin_target[3 + i].x = min(INFO->x_max/2 + (2 * i - 1)*INFO->x_max/2, INFO->x_max - 1); 
			origin_target[3 + i].y = min(INFO->y_max/2 - (2 * i - 1)*INFO->y_max/2, INFO->y_max - 1); 
			origin_target[3 + i].z = 1;
		}
		for(int i=0;i<Info()->element_num;i++)
			if(GetState(Info()->elements[i])->team == Info()->team_num)
				FormerElement[i] = Info()->elements[i];
	}
	command = new int[INFO->element_num][2];    //�����б��ʼ��
	for(int i = 0; i<INFO->element_num; i++)
		for(int j=0;j<2;j++)
			command[i][j] = -1; 
	//�з���λ���������з���λ�������ݵ㣬�������󳡺�����
	enemy_num =0;
	for(int i=0;i<Info()->element_num;i++)
		if(GetState(Info()->elements[i])->team != Info()->team_num  && GetState(Info()->elements[i])->type != MINE && GetState(Info()->elements[i])->type != OILFIELD)
			enemy_num += 1;
	enemy_element = new State[enemy_num];
	for(int i=0,j=0;i<Info()->element_num;i++)
		if(GetState(Info()->elements[i])->team == 1-Info()->team_num ||
			(GetState(Info()->elements[i])->type == FORT && GetState(Info()->elements[i])->team != Info()->team_num))   //�����ݵ�
		{
			enemy_element[j] = *GetState(Info()->elements[i]);
			j++;
		}
}

/////�ò���ȼ�ϻ��ߵ�ҩ��ʱ�򣬻���Ѫ��̫����Ҫ�ػ���ά��
void Supply_Repair(int i)
{
	State Element = *GetState(INFO->elements[i]);
	if(Element.health < parameter[0] * kElementInfos[Element.type].health_max && if_command(i, RETURNBASE))
	{
		ChangeDest(Element.index, GetState(INFO->elements[GetBase(INFO->team_num)])->pos);
		command[i][1] = max(command[i][2], RETURNBASE);
	}
	else if((Element.ammo <  parameter[1]*kElementInfos[Element.type].ammo_max ||   //��������
		Element.fuel < parameter[2]*kElementInfos[Element.type].fuel_max)          //ȼ�ϲ���
		&& if_command(i, CARGOSUPPLY))                                           //if_command
	    {
			int cargo_index = GetNear(Element.pos,CARGO);
			ChangeDest(INFO->elements[cargo_index], Element.pos);
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
			return abs(pos1.x - pos2.x + 2) + abs(pos1.y - pos2.y);
		else if(pos1.y < pos2.y - 2)
			return abs(pos1.x - pos2.x + 2) + abs(pos1.y - pos2.y + 2);
		else 
			return abs(pos1.x - pos2.x + 2);
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
		if(GetState(INFO->elements[i])->team == INFO->team_num && GetState(INFO->elements[i])->type == type &&  distance(GetState(INFO->elements[i])->pos, pos)<min_distance 
			&& type != OILFIELD && type!=MINE)
			{
				near_index = i;
				min_distance = distance(GetState(INFO->elements[i])->pos, pos);
			}
		else if(GetState(INFO->elements[i])->type == type &&  distance(GetState(INFO->elements[i])->pos, pos)<min_distance   //��֤��������
			&& type == OILFIELD && GetState(INFO->elements[i])->fuel >= kElementInfos[CARGO].fuel_max)
		{
			near_index = i;
			min_distance = distance(GetState(INFO->elements[i])->pos, pos);
		}
		else if(GetState(INFO->elements[i])->type == type &&  distance(GetState(INFO->elements[i])->pos, pos)<min_distance //��֤�󳡻��н���
			&& type == MINE  && GetState(INFO->elements[i])->metal >= kElementInfos[CARGO].metal_max)
		{
			near_index = i;
			min_distance = distance(GetState(INFO->elements[i])->pos, pos);
		}
		}
	return near_index;
}

//�ж϶Ըó�Ա�ĸ������Ƿ�����´����Ѿ��´���߼���ָ���򲻿ɣ�
int if_command(int i,CommandType type,ElementType target)
{
	if(GetState(INFO->elements[i])->type != BASE && (type == PRODUCE || type == FIX))return 0;      //������ά�޲���
	if((GetState(INFO->elements[i])->type >= FIGHTER || GetState(INFO->elements[i])->type == SUBMARINE) && type <= SUPPLYUNIT)return 0;    //��������
	if(type < FORWARD && type > command[i][0])  //���ƶ�ָ��
	{
		if((target == BASE || target == FORT || target == FIGHTER || target == SCOUT) && GetState(INFO->elements[i])->type == SUBMARINE)
			return 0;                       // Ǳͧ����أ��ݵ㣬�ɻ�����
		if((GetState(INFO->elements[i])->type == BASE || GetState(INFO->elements[i])->type == FORT ) && target == SUBMARINE)
			return 0;                               //���أ��ݵ��Ǳͧ����
		if(GetState(INFO->elements[i])->type == CARGO && type < ATTACKUNIT) return 0; //���佢û�й�����
		return 1;
	}
	if(type >= FORWARD && type > command[i][1])  //�ƶ�ָ��
	{   
		if(GetState(INFO->elements[i])->type == BASE || GetState(INFO->elements[i])->type == FORT)return 0;//���غ;ݵ㲻���ƶ�
		return 1;
	}
	return 0;
}


//���ڻغϿ�ʼʱ���Ϊ1�ĵ�λ���в���
void Cargo_Supply(int index)                           
{
	for(int j=0;j<INFO->element_num;j++)
	{
		if(distance(GetState(INFO->elements[index])->pos,GetState(INFO->elements[j])->pos) <= 1 
			&& index!=j && GetState(INFO->elements[j])->team == INFO->team_num)
		{
			if(GetState(INFO->elements[index])->type > OILFIELD)command[index][0] = SUPPLYUNIT;
			else if(GetState(INFO->elements[index])->type == FORT)command[index][0] = SUPPLYFORT;
			else if(GetState(INFO->elements[index])->type == BASE)command[index][0] = SUPPLYBASE;
			else continue;
			Supply(INFO->elements[index], INFO->elements[j],-1,-1,-1);
			break;
		}
		else if(GetState(INFO->elements[index])->type == BASE //���ؾ�������������λ��ͬ
			&& GetState(INFO->elements[j])->team == INFO->team_num
			&& DisToBase(GetState(INFO->elements[index])->pos, GetState(INFO->elements[j])->pos) <= 1)
		{
			command[index][0] = SUPPLYBASE;
			Supply(INFO->elements[index], INFO->elements[j],-1,-1,-1);
			break;
		}
	}	
}

//���ڳ��������Ȼû����������䴬��ȥ��Դ���ռ������߻ػ��ز��������ߴ���ǰ��
void MoveCargo()        
{
	for(int i=0;i<INFO->element_num;i++)
		if(GetState(INFO->elements[i])->type == CARGO && GetState(INFO->elements[i])->team == INFO->team_num)
		{
		State Element = *GetState(INFO->elements[i]);
		if(command[i][1] == -1 )
			{
				if(Element.fuel < 0.5 * kElementInfos[CARGO].fuel_max)          //ȼ�ϲ���
				{    ChangeDest(Element.index,GetState(INFO->elements[GetNear(Element.pos,OILFIELD)])->pos);
					 command[i][1] = CARGOSUPPLY;
				}
				else if(Element.metal < 0.5 * kElementInfos[CARGO].metal_max)     //��������
				{	ChangeDest(Element.index,GetState(INFO->elements[GetNear(Element.pos,MINE)])->pos);
					command[i][1] = CARGOSUPPLY;
				}
				else if(if_in(Element.index, cargo_index,2))         //�ǲ������صĴ�
				{	ChangeDest(Element.index,GetState(INFO->elements[GetBase(INFO->team_num)])->pos);   
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
	State	Element = *GetState(INFO->elements[index]);
	int health = 1000,enemy_index = -1;
	if(!if_command(index,ATTACKUNIT))return;
	for(int i=0;i<enemy_num;i++)
	{
		State *enemy = &enemy_element[i];
		if(enemy->type == BASE && enemy->team == 1-INFO->team_num
			&& DisToBase(Element.pos, enemy->pos) <= kElementInfos[Element.type].fire_ranges[SURFACE]
			&& if_command(index,CommandType(ATTACKBASE -(enemy->type-BASE)),ElementType(enemy->type)))
		{												//�з�����
			AttackUnit(Element.index,enemy->index);
			command[index][0] = CommandType(ATTACKBASE);
			if(if_command(index, FORWARD))
			{
				ChangeDest(Element.index,minus(Element.pos,enemy->pos,kElementInfos[Element.type].fire_ranges[SURFACE]));
				command[index][1] = FORWARD;
			}
			return;
		}
		if(distance(Element.pos, enemy->pos) <= kElementInfos[Element.type].fire_ranges[kElementInfos[enemy->type].level])
		{	
			if(enemy->type == FORT && enemy->team == 1-INFO->team_num
				&& if_command(index,CommandType(ATTACKBASE -(enemy->type-BASE)),ElementType(enemy->type)))         //�Է��ǵз��ݵ�
			{
				AttackUnit(Element.index,enemy->index);
				command[index][0] = CommandType(ATTACKFORT);
				if(if_command(index,FORWARD))
				{
					ChangeDest(Element.index,minus(Element.pos,enemy->pos,kElementInfos[Element.type].fire_ranges[SURFACE]));
					command[index][1] = FORWARD;
				}
				return;
			}
			if(enemy->type == FORT && enemy->team != 1-INFO->team_num 
				&& if_command(index,ATTACKFORT,FORT))                                     //�����ݵ�
			{
				AttackUnit(Element.index,enemy->index);
				command[index][0] = ATTACKFORT;
				if(if_command(index,FORWARD))
				{
					ChangeDest(Element.index,minus(Element.pos,enemy->pos,kElementInfos[Element.type].fire_ranges[SURFACE]));
					command[index][1] = FORWARD;
				}
				return;
			}
			if(health>enemy->health)   
			{
				health=enemy->health;
				enemy_index=i;
			}
		}
	}
	if(enemy_index != -1 && if_command(index, ATTACKUNIT, ElementType(GetState(INFO->elements[enemy_index])->type)))
	{
		AttackUnit(Element.index, INFO->elements[enemy_index]);
		command[index][0] = ATTACKUNIT;
		if(if_command(index,FORWARD))
		{
			ChangeDest(Element.index, minus(Element.pos,enemy_element[enemy_index].pos,
				kElementInfos[Element.type].fire_ranges[kElementInfos[enemy_element[enemy_index].type].level]));
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
		if(DisToBase(GetState(INFO->elements[i])->pos, GetState(INFO->elements[GetBase(INFO->team_num)])->pos)<=1 && 
			GetState(INFO->elements[i])->health < parameter[5]*kElementInfos[GetState(INFO->elements[i])->type].health_max
			&& health>GetState(INFO->elements[i])->health && GetState(INFO->elements[i])->team == INFO->team_num)
		{
			index = i;
			health = GetState(INFO->elements[i])->health;
		}
	}
	if(if_command(GetBase(INFO->team_num),FIX) && health != 1000)
	{	
		Fix(INFO->elements[GetBase(INFO->team_num)], index);
		command[GetBase(INFO->team_num)][0] = FIX;
		return;
	}
		BaseProduce();   //��������
}

///���ص�����
void BaseProduce()
{
	int index = GetBase(INFO->team_num);
	State Element =*GetState(INFO->elements[index]);
	if(!if_command(index,PRODUCE))return;
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
			case 0:Produce(SUBMARINE);command[index][0] = PRODUCE;produce_state[1] += 1;return;
			case 1: Produce(DESTROYER);command[index][0] = PRODUCE;produce_state[1] += 1;return;
			case 2:
			case 3: Produce(FIGHTER);command[index][0] = PRODUCE;produce_state[1] += 1;return;
			case 4: Produce(SCOUT);command[index][0] = PRODUCE;produce_state[0] += 1; return;
			default:;
		}

	 //����·������
	if(produce_state[0]>= 3 && produce_state[0]<5 && produce_state[1] <= 2)        
		switch(produce_state[1])
		{
			case 0:
			case 1: Produce(FIGHTER);command[index][0] = PRODUCE;produce_state[1] += 1;return;
			case 2: Produce(SCOUT);command[index][0] = PRODUCE;produce_state[1] += 1;return;
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
	if((produce_state[0] == 2 || produce_state[0] == 4)&& produce_state[1] == 5 && resource_defender[1] == -2 )
	{															
		int i = resource_defender[0] == -2 ? 0 : 1;
		Produce(FIGHTER);
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
		Produce(SUBMARINE);
		command[index][0] = PRODUCE;
		resource_defender[i] = -1;
		produce_state[0] += 1;
	}

}

////Ѱ������ĵо�ǰ������������䴬�ʹ�
void Forward(int index)
{
	State Element = *GetState(INFO->elements[index]);
	int Distance=1000; 
	Position target;
	if(!if_command(index,FORWARD))return;
	if(Element.type == BASE || Element.type == FORT)
		return;
	else if(Element.type == CARGO && ! if_in(Element.index, cargo_index, 2))
	{
		for(int i=0;i<INFO->element_num;i++)
			if(distance(Element.pos,GetState(INFO->elements[i])->pos) < Distance 
				&& index!=i && GetState(INFO->elements[index])->team == INFO->team_num)
			{
				Distance = distance(Element.pos,GetState(INFO->elements[i])->pos) < Distance ? distance(Element.pos,GetState(INFO->elements[i])->pos) : Distance;
				target = GetState(INFO->elements[i])->pos;
			}
	}
	else
	{
		for(int i=0;i<enemy_num;i++)
			if(distance(Element.pos,enemy_element[i].pos) < Distance)
			{
				Distance = distance(Element.pos,enemy_element[i].pos) < Distance ? distance(Element.pos,enemy_element[i].pos) : Distance;
				target = enemy_element[i].pos;
			}
	}
	ChangeDest(Element.index,target);
	command[index][1] = FORWARD;
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