//level 2 of the testmatch

#include "basic.h"
#include<iostream>

// Remove this line if you konw C++, and don't want a dirty namespace
using namespace teamstyle16;

const char * GetTeamName()
{
    return "teamstyle16";  // Name of your team
}

const GameInfo *INFO = Info();


int distance(Position pos1, Position pos2);//����֮��ľ���
int DisToBase(Position pos1, Position pos2)  //pos2��ʾ�������Ͻ�,��������صľ���
int GetNear(Position pos, ElementType type);//��þ��λ�������ĳ���͵�λ������
int GetBase(int team);//��ü�����Է����ص�����

void AIMain()
{
	int i, flag = 0;
	Produce(CARGO);
	for(i = 0; i < INFO->element_num; i++)
	{
		const State *Element = GetState(INFO->elements[i]);
		if(Element->team == INFO->team_num &&Element->type == CARGO)
		{
			if(DistToBase(Element->pos, GetBase(INFO->team_num)))
			if(Element->fuel <= 0.5 * kElementInfos[CARGO].fuel_max && Element->metal <= 0.5 * kElementInfos[CARGO].metal_max)
			{
				if(flag == 1)
				{
					ChangeDest(Element->index, GetState(INFO->elements[GetNear(Element->pos,MINE)])->pos);
					flag = 0;
				}	
				else
				{
					ChangeDest(Element->index, GetState(INFO->elements[GetNear(Element->pos,OILFIELD)])->pos);
					flag = 1;
				}
			}	
			else
				ChangeDest(Element->index, GetState(GetNear(Element->pos, BASE))->pos);
		}	
	}
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

//����֮��ľ���
int distance(Position pos1, Position pos2)
{
	return abs(pos1.x - pos2.x) + abs(pos1.y - pos2.y);
}

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