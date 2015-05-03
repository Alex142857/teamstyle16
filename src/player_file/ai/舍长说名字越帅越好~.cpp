#include <cstdlib>
#include "basic.h"
#include<iostream>
#include<cmath>
#include<vector>
using namespace teamstyle16;
using std::vector;
using std::cout;
using std::endl;


const char * GetTeamName()
{
    return "BIG BUG";
}


void enemy_init();                                           
Position minus(Position pos1,Position pos2,int fire_range);  
int GetBase(int team);                                       
int distance(Position pos1, Position pos2);                  
int if_alive(int operand);                                   
const GameInfo *INFO = Info();                               
int enemy_num;                                               //�з���λ��
State *enemy_element;                                        //�з�ָ��
int position_match_to_index[25][3];                          //����ط��������������
int GetNear(Position pos, ElementType type);                 //�������ĵ�λ���к�
int if_in(int i,int *a,int len);                             //�ж��Ƿ���������
vector<int>Enemy_Indexes;                                    //�з���λ���к�
void Attack_Fort(int index);                                 //�����ݵ�
void Attack_Base(int index);                                 //�������صĺ���
void my_Produce_1(int index);                                //���ؽ��������������
void my_Produce_2(int index);                                //����Զ�������������
void Cargo_Supply_Base(int index);                           //���䴬��������
void Cargo_move(int index);                                  //���䴬�ƶ�
void Base_attack(int index);                                 //���ع���
void surround_base(int index);                               //��Χ�з�����
int if_empty(Position p);
void init_base_array();
void attack_fighter(int index);
int count_type(ElementType t);
void Base_supply(int index);
void Base_fighter();
int My_fighter_array[9][3];
void init_fighter_array();
int if_full();
void fort_attact(int i);
int mine_metal_message[2][15][2];
void init_mine_metal_message();
int search_message(int index);
void fort_supply(int index);
void surround_fort(int index);
int position_match_to_index_fort[9][3];
void init_fort_array();
void carrier_move(int index);
int get_near_fort(Position pos, ElementType type);
void carrier_attact(int index);
Position if_ocean_and_not_full(int index);
int base_supply_array[12][3];
void init_base_supply_array();
void return_to_base(int index);
int carrier_array[32][3];
void init_carrier_array();
Position return_carrier_destination(int index);
int cargo_array[5][3];
void init_cargo_array();
int judge_position(int index);
void move_one_step(int index);

////////////////////////////////////////  ������  ////////////////////////////////////////

void AIMain()
{  
	TryUpdate();
	enemy_init();
	//init_base_array();
	//init_fighter_array();
	init_mine_metal_message();
	//init_fort_array();
	init_base_supply_array();
	//init_carrier_array();
	int tactics;
	if(distance(INFO->elements[GetBase(INFO->team_num)]->pos,INFO->elements[GetBase(1 - INFO->team_num)]->pos) < 80)
	{
		tactics = 1;
		init_base_array();
		init_fighter_array();
	}
	else
	{
		init_carrier_array();
		tactics = 2;
	}
	for(int i = 0;i < INFO->element_num; i++)
		if(INFO -> elements[i] -> team == INFO -> team_num)
		{
			const State *Element = INFO->elements[i];
			if(Element->type == BASE)
			{
				if(tactics == 1)
					my_Produce_1(i);
				else
					my_Produce_2(i);
				Base_attack(i);
				Base_supply(i);
			}
			else if(Element->type == FIGHTER && tactics == 1)
				Attack_Base(i);
			else if(Element->type == FIGHTER && tactics == 2)
			{
				Attack_Fort(i);
				return_to_base(i);
			}
			else if(Element->type == CARGO)
			{
				Cargo_move(i);
				Cargo_Supply_Base(i);   
			}
			else if(Element->type == FORT)
			{
				fort_attact(i);
				fort_supply(i);
			}
			else if(Element->type == CARRIER)
			{
				if(distance(Element->pos,INFO->elements[GetBase(1 - INFO->team_num)]->pos) != 8)
					carrier_move(i);
				carrier_attact(i);
			}
			else;
		}
	delete []enemy_element;
}

/////////////////////////////////////  ��������  ///////////////////////////////////////

//���䴬�ƶ�һС��
void move_one_step(int index)
{
	const State* cargo = INFO->elements[index];
	int one_step[9][3];
	//2
	one_step[1][1] = cargo->pos.x - 1;
	one_step[1][2] = cargo->pos.y - 1;
	//3
	one_step[2][1] = cargo->pos.x;
	one_step[2][2] = cargo->pos.y - 1;
	//4
	one_step[3][1] = cargo->pos.x - 1;
	one_step[3][2] = cargo->pos.y;
	//5
	one_step[4][1] = cargo->pos.x + 1;
	one_step[4][2] = cargo->pos.y + 1;
	//6
	one_step[5][1] = cargo->pos.x;
	one_step[5][2] = cargo->pos.y + 1;
	//7
	one_step[6][1] = cargo->pos.x + 1;
	one_step[6][2] = cargo->pos.y;
	//8
	one_step[7][1] = cargo->pos.x + 1;
	one_step[7][2] = cargo->pos.y - 1;
	//9
	one_step[8][1] = cargo->pos.x - 1;
	one_step[8][2] = cargo->pos.y + 1;
	for(int i = 1;i < 9; ++i)
		if(Map(one_step[i][1],one_step[i][2]) == OCEAN)
		{
			Position temp;
			temp.z = 1;
			temp.x = one_step[i][1];
			temp.y = one_step[i][2];
			ChangeDest(cargo->index,temp);
			break;
		}
}
//�ж����䴬�Ƿ��ƶ�
int judge_position(int index)
{
	int judge = 0;
	const State* cargo = INFO->elements[index];
	for(int i = 0;i < 5;++i)
		if(cargo->index == cargo_array[i][0])
		{
			if(cargo_array[i][1] == cargo->pos.x && cargo_array[i][2] == cargo->pos.y)
			{
				judge = 1;
				break;
			}
		}
	return judge;
}

//��ʼ�����䴬����
void init_cargo_array()
{
	int count = 0;
	if(INFO->round == 6)
	{
		for(int j = 0;j < INFO->element_num;++j)
		{
			const State* cargo = INFO->elements[j];
			if(cargo->type == CARGO && cargo->team == INFO->team_num)
			{
				cargo_array[count][0] = cargo->index;
				count++;
			}
		}
	}
	for(int i = 0;i < INFO->element_num; ++i)
	{
		const State* cargo = INFO->elements[i];
		for(int j = 0;j < 5; ++j)
			if(cargo->index == cargo_array[j][0])
			{
				cargo_array[j][1] = cargo->pos.x;
				cargo_array[j][2] = cargo->pos.y;
				break;
			}
	}
}

//���غ�ĸ��Ŀ�ĵ�
Position return_carrier_destination(int index)
{
	Position temp;
	for(int i = 0;i < 32; ++i)
	{
		if(index == carrier_array[i][0])
		{
			temp.z = 1;
			temp.x = carrier_array[i][1];
			temp.y = carrier_array[i][2];
			break;
		}
	}
	return temp;
}

//��ʼ����ĸ����
void init_carrier_array()
{
	if(INFO->round == 0)
	{
		for(int i = 0;i < 32; i++)
		{
			carrier_array[i][1] = 0;
			carrier_array[i][2] = 0;
		}
		const State* enemy_base = INFO->elements[GetBase(1 - INFO->team_num)];
		int x = enemy_base->pos.x - 8;
		int y = enemy_base->pos.y - 8;
		int count = 0;
		for(;x < (enemy_base->pos.x + 8); ++x)
		{
			y = enemy_base->pos.y - 8;
			for(;y < (enemy_base->pos.y + 8); ++y)
			{
				Position temp;
				temp.z = 1;
				temp.x = x;temp.y = y;
				if(distance(temp,enemy_base->pos) == 8 && Map(x,y) == OCEAN && x <= kMaxMapSize && y <= kMaxMapSize && x > 0 && y > 0)
				{
					carrier_array[count][1] = x;
					carrier_array[count][2] = y;
					count++;
				}
			}
		}
	}
	int count_2 = 0;
	for(int i = 0;i < INFO->element_num; ++i)
	{
		const State* temp = INFO->elements[i];
		if(temp->type == CARRIER && temp->team == INFO->team_num)
		{
			carrier_array[count_2][0] = temp->index;
			count_2++;
		}
	}
}
//�ж�λ���Ƿ�ռ��
int if_empty(Position p)
{
	int flag = 1;
	for(int i = 0;i < INFO->element_num; ++i)
	{
		const State* temp = INFO->elements[i];
		if(temp->type == CARRIER && temp->team == INFO->team_num)
		{
			if(temp->pos.x == p.x && temp->pos.y == p.y && temp->pos.z == 1)
			{
				flag = 0;
				break;
			}
		}
	}
	return flag;
}

//���ػ��صĺ���
void return_to_base(int index)
{
	const State* fighter = INFO->elements[index];
	if(fighter->ammo == 0 || fighter->fuel < distance(fighter->pos,INFO->elements[GetBase(INFO->team_num)]->pos))
	{
		for(int i = 0;i < 9; ++i)
			if(My_fighter_array[i][0] == 0)
			{
				Position temp;
				temp.z = 2;
				temp.x = My_fighter_array[i][1];
				temp.y = My_fighter_array[i][2];
				ChangeDest(fighter->index,temp);
			}
	}
}

//���䴬�ܵ�������λ��
Position if_ocean_and_not_full(int index)
{
	const State* cargo = INFO->elements[index];
	Position temp;
	temp.z = 1;
	int min_distance = 1000;
	for(int i = 0;i < 12; ++i)
		if(base_supply_array[i][2] == 0 && distance(cargo->pos,INFO->elements[GetBase(INFO->team_num)]->pos) < min_distance)
		{
			min_distance = distance(cargo->pos,INFO->elements[GetBase(INFO->team_num)]->pos);
			temp.x = base_supply_array[i][0];
			temp.y = base_supply_array[i][1];
		}
	return temp;
}

//��ʼ�����䴬�����Ļ�������
void init_base_supply_array()
{
	int base_position_x = INFO->elements[GetBase(INFO->team_num)]->pos.x;
	int base_position_y = INFO->elements[GetBase(INFO->team_num)]->pos.y;
	for(int i = 0;i < 12; ++i)
		base_supply_array[i][2] = 0;
	//1
	base_supply_array[0][0] = base_position_x + 1;
	base_supply_array[0][1] = base_position_y - 2;
	//2
	base_supply_array[1][0] = base_position_x - 2;
	base_supply_array[1][1] = base_position_y;
	//3
	base_supply_array[2][0] = base_position_x - 2;
	base_supply_array[2][1] = base_position_y + 1;
	//4
	base_supply_array[3][0] = base_position_x - 2;
	base_supply_array[3][1] = base_position_y - 1;
	//5
	base_supply_array[4][0] = base_position_x + 2;
	base_supply_array[4][1] = base_position_y;
	//6
	base_supply_array[5][0] = base_position_x + 2;
	base_supply_array[5][1] = base_position_y + 1;
	//7
	base_supply_array[6][0] = base_position_x + 2;
	base_supply_array[6][1] = base_position_y - 1;
	//8
	base_supply_array[7][0] = base_position_x;
	base_supply_array[7][1] = base_position_y + 2;
	//9
	base_supply_array[8][0] = base_position_x - 1;
	base_supply_array[8][1] = base_position_y + 2;
	//10
	base_supply_array[9][0] = base_position_x + 1;
	base_supply_array[9][1] = base_position_y + 2;
	//11
	base_supply_array[10][0] = base_position_x;
	base_supply_array[10][1] = base_position_y - 2;
	//12
	base_supply_array[11][0] = base_position_x - 1;
	base_supply_array[11][1] = base_position_y - 2;
	for(int i = 0;i < 12; ++i)
		for(int j = 0;j < INFO->element_num; ++j)
		{
			const State* temp = INFO->elements[j];
			if(temp->visible == true)
				if(temp->pos.x == base_supply_array[i][0] && temp->pos.y == base_supply_array[i][1] && temp->pos.z == 1)
					base_supply_array[i][2] = 1;
		}
	for(int i = 0;i < 12; ++i)
		if(Map(base_supply_array[i][0],base_supply_array[i][1]) == LAND)
			base_supply_array[i][2] = 1;
}

//��ĸ��������
void carrier_attact(int index)
{
	const State* carrier = INFO->elements[index];
	AttackPos(carrier->index,INFO->elements[GetBase(1 - INFO->team_num)]->pos);
}

//�������ķǼ����ݵ�
int get_near_fort(Position pos, ElementType type)
{
	int i,near_index = -1,min_distance = 1000;
	for(i = 0;i < INFO->element_num;i++)  //һ����λһ����λ��Ѱ��
	{
		const State *target = INFO->elements[i];
		if(target->type == type && distance(target->pos,pos) < min_distance && target->team == 2)
		{
			near_index = i;
			min_distance = distance(target->pos, pos);
		}
	}
	return near_index;
}

//����ĸ�����ƶ�
void carrier_move(int index)
{
	const State* carrier = INFO->elements[index];
	for(int i = 0;i < 32;i++)
		if(carrier_array[i][0] == carrier->index)
		{
			carrier_array[i][0] ;
			Position temp;
			temp.x = carrier_array[i][1];
			temp.y = carrier_array[i][2];
			temp.z = 1;
			ChangeDest(carrier->index,temp);
			break;
		}
}
//��ʼ���ݵ��Χ������
void init_fort_array()
{
	int fort_index = get_near_fort(INFO->elements[GetBase(INFO->team_num)]->pos,FORT);
	if(fort_index != -1)
	{
		int base_position_x = INFO->elements[fort_index]->pos.x;
		int base_position_y = INFO->elements[fort_index]->pos.y;
		for(int i = 0; i < 9; ++i)
			position_match_to_index_fort[i][2] = 0;
		//1��λ
		position_match_to_index_fort[0][0] = base_position_x;
		position_match_to_index_fort[0][1] = base_position_y;
		//2��λ
		position_match_to_index_fort[1][0] = base_position_x - 1;
		position_match_to_index_fort[1][1] = base_position_y - 1;
		//3��λ
		position_match_to_index_fort[2][0] = base_position_x - 1;
		position_match_to_index_fort[2][1] = base_position_y;
		//4��λ
		position_match_to_index_fort[3][0] = base_position_x - 1;
		position_match_to_index_fort[3][1] = base_position_y + 1;
		//5��λ
		position_match_to_index_fort[4][0] = base_position_x;
		position_match_to_index_fort[4][1] = base_position_y + 1;
		//6��λ
		position_match_to_index_fort[5][0] = base_position_x;
		position_match_to_index_fort[5][1] = base_position_y - 1;
		//7��λ
		position_match_to_index_fort[6][0] = base_position_x + 1;
		position_match_to_index_fort[6][1] = base_position_y + 1;
		//8��λ
		position_match_to_index_fort[7][0] = base_position_x + 1;
		position_match_to_index_fort[7][1] = base_position_y;
		//9��λ
		position_match_to_index_fort[8][0] = base_position_x + 1;
		position_match_to_index_fort[8][1] = base_position_y - 1;
	}
	int j = 0;
	for(int i = 0; i < INFO->element_num ;++i)
		if(INFO->elements[i]->team == INFO->team_num && INFO->elements[i]->type == FIGHTER)
		{
				position_match_to_index_fort[j][2] = INFO->elements[i]->index;
				j++;
		}
}

//��Χ�ݵ�ĺ���
void surround_fort(int index)
{
	for(int j = 0;j < 9; ++j)
		if(index == position_match_to_index_fort[j][2])
		{
			Position target;
			target.x = position_match_to_index_fort[j][0];
			target.y = position_match_to_index_fort[j][1];
			target.z = 2;
			ChangeDest(index,target);
			break;
		}
}

//�ڶ�����������
void my_Produce_2(int index)
{
	/*if(INFO->round == 0)
	{
		for(int i = 0;i < 5;++i)
			Produce(CARGO);
		Produce(FIGHTER);
		Produce(FIGHTER);
		Produce(FIGHTER);
	}
	else if(INFO->round == 6 || INFO->round == 7 || INFO->round == 8)
	{
		Produce(FIGHTER);
		Produce(FIGHTER);
	}
	else if(count_type(CARGO) < 5 && INFO->round > 10)
		Produce(CARGO);
	else if(INFO->round > 8)
	{
		for(int i = 0;i < 9;++i)
		Produce(CARRIER);
	}
	else;*/


	if(INFO->round == 0)
	{
		for(int i = 0;i < 5;++i)
			Produce(CARGO);
		Produce(CARRIER);
		Produce(CARRIER);
	}
	if(INFO->round >= 5)
		for(int i = 0;i < 6; ++i)
			Produce(CARRIER);
}

//������֪��λ��ŵ�ȼ�Ϻͽ�����Ϣ
int search_message(int index)
{
	for(int i = 0;i < 15; ++i)
	{
		if(index == mine_metal_message[0][i][0])
			return mine_metal_message[0][i][1];
		if(index == mine_metal_message[1][i][0])
			return mine_metal_message[1][i][1];
	}
}

//��ʼ���ѻ�õ��������Ϣ
void init_mine_metal_message()
{
	for(int i = 0; i < 15; i++)
	{
		mine_metal_message[0][i][0] = 0;
		mine_metal_message[0][i][1] = 800;
		mine_metal_message[1][i][0] = 0;
		mine_metal_message[1][i][1] = 500;
	}
	int count_oil = 0,count_metal = 0;
	for(int i = 0;i < INFO->element_num; ++i)
	{
		const State* temp = INFO->elements[i];
		if(temp->type == OILFIELD)
		{
			mine_metal_message[0][count_oil][0] = temp->index;
			if(temp->visible == true)
				mine_metal_message[0][count_oil][1] = temp->fuel;
			count_oil++;
		}
		if(temp->type == MINE)
		{
			mine_metal_message[1][count_metal][0] = temp->index;
			if(temp->visible == true)
				mine_metal_message[1][count_metal][1] = temp->metal;
			count_metal++;
		}
		if(temp->type == FORT && temp->team == INFO->team_num)
		{
			mine_metal_message[1][count_metal][0] = temp->index;
			mine_metal_message[1][count_metal][1] = temp->metal;
			mine_metal_message[0][count_metal][0] = temp->index;
			mine_metal_message[0][count_metal][1] = temp->fuel;
			count_metal++;
			count_oil++;
		}
	}
}

//�ݵ㲹������
void fort_supply(int index)
{
	const State	*Element = INFO->elements[index];
	for(int i = 0;i < INFO->element_num; i++)
	{
		const State* target = INFO->elements[i];
		if(target->team == INFO->team_num && target->type == CARGO)
			Supply(Element->index,target->index,300,0,50);
	}
}
//�ݵ㹥���Է���λ�ĺ���
void fort_attact(int index)
{
	const State	*fort = INFO->elements[index];
	const State	*target;
	for(int i = 0; i < INFO->element_num; ++i)
	{
		if(INFO->elements[i]->visible == true)
		{
			target = INFO->elements[i];
			int dis = 0;
			if(target->type == FIGHTER || target->type == SCOUT)dis = 5;
			if(target->type == DESTROYER || target->type == CARGO)dis = 7;
			if(distance(fort->pos,target->pos) <= dis && INFO->elements[i]->team == 1 - INFO->team_num)
			{
				AttackUnit(fort->index,target->index);
				break;
			}
		}
	}
}
//��ʼ�����طɻ�����
void init_fighter_array()
{
	const State* Base = INFO->elements[GetBase(INFO->team_num)];
	for(int i = 0;i < 9; i++)
		My_fighter_array[i][0] = 0;
	//1
	My_fighter_array[0][1] = Base->pos.x;
	My_fighter_array[0][2] = Base->pos.y;
	//2
	My_fighter_array[1][1] = Base->pos.x - 1;
	My_fighter_array[1][2] = Base->pos.y - 1;
	//3
	My_fighter_array[2][1] = Base->pos.x;
	My_fighter_array[2][2] = Base->pos.y - 1;
	//4
	My_fighter_array[3][1] = Base->pos.x - 1;
	My_fighter_array[3][2] = Base->pos.y;
	//5
	My_fighter_array[4][1] = Base->pos.x + 1;
	My_fighter_array[4][2] = Base->pos.y + 1;
	//6
	My_fighter_array[5][1] = Base->pos.x;
	My_fighter_array[5][2] = Base->pos.y + 1;
	//7
	My_fighter_array[6][1] = Base->pos.x + 1;
	My_fighter_array[6][2] = Base->pos.y;
	//8
	My_fighter_array[7][1] = Base->pos.x + 1;
	My_fighter_array[7][2] = Base->pos.y - 1;
	//9
	My_fighter_array[8][1] = Base->pos.x - 1;
	My_fighter_array[8][2] = Base->pos.y + 1;
	for(int i = 0;i < INFO->element_num; ++i)
	{
		if(INFO->elements[i]->type == FIGHTER && INFO->elements[i]->team == INFO->team_num)
		{
			const State* fighter = INFO->elements[i];
			for(int j = 0;j < 9; ++j)
				if(fighter->pos.x == My_fighter_array[j][1] && fighter->pos.y == My_fighter_array[j][2])
					My_fighter_array[j][0] = 1;
		}
	}
}
//�����ڻ��صķɻ��Ƿ���9��
int if_full()
{
	int flag = 1;
	for(int i = 0;i < 9; ++i)
		if(My_fighter_array[i][0] == 0)
		{
			flag = 0;break;
		}
	return flag;
}
//���ز�������
void Base_supply(int index)
{
	const State	*base = INFO->elements[index];
	for(int i = 0; i < INFO->element_num; ++i)
	{
		const State	*target = INFO->elements[i];
		if(target->type == CARGO && target->team == INFO->team_num)
			if(target->fuel == 0)
				Supply(base->index,target->index,50,0,0);
		if(target->type == FIGHTER && target->fuel < 70 && target->team == INFO->team_num && INFO->round < 102)
		{
			if(base->fuel > 50)
				Supply(base->index,target->index,50,21,0);
			else
				Supply(base->index,target->index,0,21,0);
		}
		if(INFO->round >= 102)
			Supply(base->index,target->index,0,21,0);
		if(target->type == CARRIER && target->fuel < 150 && target->team == INFO->team_num)
			Supply(base->index,target->index,200,0,0);
	}
}
//ͳ�����͸���
int count_type(ElementType t)
{
	int type = 0;
	for(int i = 0;i < INFO->element_num;i++)
		if(INFO->elements[i]->type == t && INFO->elements[i]->team == INFO->team_num)
			type++;
	return type;
}
//�����з��ɻ��ĺ���
void attack_fighter(int index)
{
	const State	*Base = INFO->elements[GetBase(1 - INFO->team_num)];
	const State	*my_fighter = INFO->elements[index];
	for(int i = 0;i < INFO->element_num; ++i)
	{
		const State	*target = INFO->elements[i];
		if(target->visible == true && target->type == FIGHTER && target->team == 1 - INFO->team_num)
			if(distance(my_fighter->pos,target->pos) <= 4 && distance(Base->pos,my_fighter->pos) > 4)
				AttackUnit(my_fighter->index,target->index);
	}
}

//�����ݵ�ĺ���
void Attack_Fort(int index)
{
	const State	*Element = INFO->elements[index];
	int health = 1000,enemy_index = -1;
	for(int i=0;i<enemy_num;i++)
	{
		State *enemy = &enemy_element[i];                                     
		if(enemy->type == FORT)                                     
		{
			surround_fort(Element->index);
			attack_fighter(index);
			if(distance(Element->pos,enemy->pos) <= 2)
			AttackUnit(Element->index,enemy->index);
		}
	}
}

//�����з����صĺ���
void Attack_Base(int index)
{
	const State	*Element = INFO->elements[index];
	int health = 1000,enemy_index = -1;
	for(int i = 0;i < enemy_num;i++)
	{
		State *enemy = &enemy_element[i];
		if(enemy->type == BASE && if_full() == 1)                                     
		{
			surround_base(Element->index);
		}
		if(enemy->type == BASE)
		{
			attack_fighter(index);
			AttackUnit(Element->index,enemy->index);
		}
	}
}

//���صķ�������
void Base_attack(int index)
{
	const State	*Base = INFO->elements[GetBase(INFO->team_num)];
	const State	*target;
	for(int i = 0; i < INFO->element_num; ++i)
	{
		if(INFO->elements[i]->visible == true)
		{
			target = INFO->elements[i];
			int dis = 0;
			if(target->type == FIGHTER || target->type == SCOUT)dis = 5;
			if(target->type == DESTROYER || target->type == CARGO ||target->type == CARRIER)dis = 7;
			if(distance(Base->pos,target->pos) <= dis && INFO->elements[i]->team == 1 - INFO->team_num)
			{
				AttackUnit(Base->index,target->index);
				break;
			}
		}
	}
}

//��������
void my_Produce_1(int index)
{
	if(INFO->round == 0)
	{
		for(int i = 0;i < 5;++i)
			Produce(CARGO);
		Produce(FIGHTER);
		Produce(FIGHTER);
		Produce(FIGHTER);
		Produce(FIGHTER);
	}
	else if(INFO->round > 4)
		for(int i = 0; i < 9;i++)
			Produce(FIGHTER);
}

//����Info()��ʼ����ʱ�ĵ���
void enemy_init()
{
	//�з���λ���������з���λ�������ݵ㣬�������󳡺�����
	enemy_num = 0;
	for(int i = 0;i < Info()->element_num;i++)
		if(Info()->elements[i]->team != Info()->team_num  
			&& Info()->elements[i]->type != MINE && Info()->elements[i]->type != OILFIELD)
		{
			enemy_num += 1;                //ͳ�Ƶз���λ��Ŀ
			Enemy_Indexes.push_back(i);    //��з�����������Ϣ
		}	
	enemy_element = new State[enemy_num];  //��������з���Ϣ������
	for(int i = 0; i < enemy_num; i++)
		{
			enemy_element[i] = *(Info()->elements[Enemy_Indexes[i]]);
		}
	Enemy_Indexes.clear();
}

//��ȡ�������к�
int GetBase(int team)
{
	int i;
	for(i=0; i<INFO->element_num; i++)
		if(INFO->elements[i]->type == BASE && INFO->elements[i]->team == team)
			return i; 
	return -1;  //��ȡʧ��
}

//����֮��ľ���
int distance(Position pos1, Position pos2)
{
	return fabs((float)(pos1.x - pos2.x)) + fabs((float)(pos1.y - pos2.y));
}

//�ж�ĳ��λ�Ƿ��Դ��
int if_alive(int operand)
{
	for(int i = 0;i < INFO -> element_num;i++)
		if(INFO -> elements[i]->index == operand)  
			return 1;
	return 0;
}

//���ڻ��ص����Ϊ1���в���
void Cargo_Supply_Base(int index)
{
	const State *element, *target;
	element = INFO->elements[index];
	for(int j = 0;j < INFO->element_num;j++)
	{
		target = INFO->elements[j];
		if(target->team == INFO->team_num && target->type == BASE && element->fuel >=100 && element->metal == 0)	
			Supply(INFO->elements[index]->index,target->index,(int)(element->fuel * 0.6),0,element->metal);
		if(target->team == INFO->team_num && target->type == BASE && element->fuel >=100 && element->metal != 0)	
			Supply(INFO->elements[index]->index,target->index,(int)(element->fuel * 0.75),0,element->metal);
	}	
}

//��þ��λ�������ĳ���͵�λ������
int GetNear(Position pos, ElementType type)
{
	int i,near_index = -1,min_distance = 1000;
	for(i = 0;i < INFO->element_num;i++)  //һ����λһ����λ��Ѱ��
	{
		const State *target = INFO->elements[i];
		if(target->type == type && distance(target->pos,pos) < min_distance)
		{
			if(type == OILFIELD && search_message(target->index) >= kProperty[CARGO].fuel_max * 0.8) //���ﻹ��ʯ��
			{
				near_index = i;
				min_distance = distance(target->pos, pos);
			}
			else if(type == MINE && search_message(target->index) >= kProperty[CARGO].metal_max * 0.8)//��֤�󳡻��н���
			{
				near_index = i;
				min_distance = distance(target->pos, pos);
			}
			else if(type == FORT && target->team == INFO->team_num && search_message(target->index) >= kProperty[CARGO].metal_max * 0.8)//�ݵ㻹����Դ
			{
				near_index = i;
				min_distance = distance(target->pos, pos);
			}
			else if(type == FORT && target->team == INFO->team_num && search_message(target->index) >= kProperty[CARGO].fuel_max * 0.8)//�ݵ㻹����Դ
			{
				near_index = i;
				min_distance = distance(target->pos, pos);
			}
			else ;
		}
	}
	return near_index;
}

//�ж�ĳ�����Ƿ���������
int if_in(int i,int *a,int len)
{
	for(int j=0;j<len;j++)
		if(i==a[j])
			return 1;
	return 0;
}

//���䴬���ƶ�
void Cargo_move(int index)
{
	const State *Element = INFO->elements[index];
	const State *base = INFO->elements[GetBase(INFO->team_num)];
	const State *enemy_base = INFO->elements[GetBase(1 - INFO->team_num)];
	if(Element->fuel < 200)
	{
		int oil_index = GetNear(Element->pos,OILFIELD);
		int fort_index = GetNear(Element->pos,FORT);
		ChangeDest(Element->index, INFO->elements[oil_index]->pos);
		if(fort_index == -1 && oil_index != -1)
			ChangeDest(Element->index, INFO->elements[oil_index]->pos);
		else if(oil_index == -1 && fort_index != -1)
			ChangeDest(Element->index, INFO->elements[fort_index]->pos);
		else if(fort_index != -1 && oil_index != -1)
		{
			const State *oil = INFO->elements[oil_index];
			const State *fort = INFO->elements[fort_index];
			if(distance(oil->pos,Element->pos) > distance(fort->pos,Element->pos))
				ChangeDest(Element->index, INFO->elements[fort_index]->pos);
			else
				ChangeDest(Element->index, INFO->elements[oil_index]->pos);
		}
		else
		{
			int x = (base->pos.x + enemy_base->pos.x)/2;
			int y = (base->pos.y + enemy_base->pos.y)/2;
			Position temp;
			temp.x = x;
			temp.y = y;
			temp.z = 1;
			ChangeDest(Element->index,temp);
		}
	}
	else if(Element->metal == 0 && base->fuel < 1000)
	{
		int metal_index = GetNear(Element->pos,MINE);
		int fort_index = GetNear(Element->pos,FORT);
		ChangeDest(Element->index, INFO->elements[metal_index]->pos);
		if(fort_index == -1 && metal_index != -1)
			ChangeDest(Element->index, INFO->elements[metal_index]->pos);
		else if(metal_index == -1 && fort_index != -1)
			ChangeDest(Element->index, INFO->elements[fort_index]->pos);
		else if(fort_index != -1 && metal_index != -1)
		{
			const State *oil = INFO->elements[metal_index];
			const State *fort = INFO->elements[fort_index];
			if(distance(oil->pos,Element->pos) > distance(fort->pos,Element->pos))
				ChangeDest(Element->index, INFO->elements[fort_index]->pos);
			else
				ChangeDest(Element->index, INFO->elements[metal_index]->pos);
		}
		else
		{
			int x = (base->pos.x + enemy_base->pos.x)/2;
			int y = (base->pos.y + enemy_base->pos.y)/2;
			Position temp;
			temp.x = x;
			temp.y = y;
			temp.z = 1;
			ChangeDest(Element->index,temp);
		}
	}
	else if(Element->metal > 0 && Element->fuel > 200)
		ChangeDest(Element->index,if_ocean_and_not_full(index));
	else;
}

//��Χ���ص������ʼ��
void init_base_array()
{
	int base_position_x = INFO->elements[GetBase(1-INFO->team_num)]->pos.x;
	int base_position_y = INFO->elements[GetBase(1-INFO->team_num)]->pos.y;
	if(INFO->round == 0)
	{
		for(int i = 0; i < 25; ++i)
			position_match_to_index[i][0] = 0;
		//1��λ
		position_match_to_index[0][0] = base_position_x;
		position_match_to_index[0][1] = base_position_y;
		//2��λ
		position_match_to_index[1][0] = base_position_x - 1;
		position_match_to_index[1][1] = base_position_y - 1;
		//3��λ
		position_match_to_index[2][0] = base_position_x - 1;
		position_match_to_index[2][1] = base_position_y;
		//4��λ
		position_match_to_index[3][0] = base_position_x - 1;
		position_match_to_index[3][1] = base_position_y + 1;
		//5��λ
		position_match_to_index[4][0] = base_position_x;
		position_match_to_index[4][1] = base_position_y + 1;
		//6��λ
		position_match_to_index[5][0] = base_position_x;
		position_match_to_index[5][1] = base_position_y - 1;
		//7��λ
		position_match_to_index[6][0] = base_position_x + 1;
		position_match_to_index[6][1] = base_position_y + 1;
		//8��λ
		position_match_to_index[7][0] = base_position_x + 1;
		position_match_to_index[7][1] = base_position_y;
		//9��λ
		position_match_to_index[8][0] = base_position_x + 1;
		position_match_to_index[8][1] = base_position_y - 1;
		//10��λ
		position_match_to_index[9][0] = base_position_x - 2;
		position_match_to_index[9][1] = base_position_y;
		//11��λ
		position_match_to_index[10][0] = base_position_x - 2;
		position_match_to_index[10][1] = base_position_y + 1;
		//12��λ
		position_match_to_index[11][0] = base_position_x - 2;
		position_match_to_index[11][1] = base_position_y + 2;
		//13��λ
		position_match_to_index[12][0] = base_position_x - 2;
		position_match_to_index[12][1] = base_position_y - 2;
		//14��λ
		position_match_to_index[13][0] = base_position_x - 2;
		position_match_to_index[13][1] = base_position_y - 1;
		//15��λ
		position_match_to_index[14][0] = base_position_x + 2;
		position_match_to_index[14][1] = base_position_y;
		//16��λ
		position_match_to_index[15][0] = base_position_x + 2;
		position_match_to_index[15][1] = base_position_y + 1;
		//17��λ
		position_match_to_index[16][0] = base_position_x + 2;
		position_match_to_index[16][1] = base_position_y + 2;
		//18��λ
		position_match_to_index[17][0] = base_position_x + 2;
		position_match_to_index[17][1] = base_position_y - 2;
		//19��λ
		position_match_to_index[18][0] = base_position_x + 2;
		position_match_to_index[18][1] = base_position_y - 1;
		//20��λ
		position_match_to_index[19][0] = base_position_x - 1;
		position_match_to_index[19][1] = base_position_y + 2;
		//21��λ
		position_match_to_index[20][0] = base_position_x - 1;
		position_match_to_index[20][1] = base_position_y - 2;
		//22��λ
		position_match_to_index[21][0] = base_position_x;
		position_match_to_index[21][1] = base_position_y + 2;
		//23��λ
		position_match_to_index[22][0] = base_position_x;
		position_match_to_index[22][1] = base_position_y - 2;
		//24��λ
		position_match_to_index[23][0] = base_position_x + 1;
		position_match_to_index[23][1] = base_position_y + 2;
		//25��λ
		position_match_to_index[24][0] = base_position_x + 1;
		position_match_to_index[24][1] = base_position_y - 2;
	}
	for(int i = 0; i < 25; ++i)
	{
		int fighter_index = position_match_to_index[i][2];
		if(!if_alive(fighter_index))
			position_match_to_index[i][2] = 0;
	}
	for(int i = 0; i < INFO->element_num ;++i)
	{
		if(INFO->elements[i]->team == INFO->team_num && INFO->elements[i]->type == FIGHTER)
		{
			bool flag = true;
			for(int j = 0; j < 25; ++j)
				if(position_match_to_index[j][2] == INFO->elements[i]->index)
				{
					flag = false;
					break;
				}
			if(flag == false)
				continue;
			for(int j = 0;j < 25; ++j)
				if(position_match_to_index[j][2] == 0)
				{
					position_match_to_index[j][2] = INFO->elements[i]->index;
					break;
				}
		}
	}
}

//��Χ���صĺ���
void surround_base(int index)
{
	for(int j = 0;j < 25; ++j)
		if(index == position_match_to_index[j][2])
		{
			Position target;
			target.x = position_match_to_index[j][0];
			target.y = position_match_to_index[j][1];
			target.z = 2;
			ChangeDest(index,target);
			break;
		}
}
